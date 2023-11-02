# LiAR isn't a raytracer
# Copyright (C) 2010-2023  Bram de Greve (bramz@users.sourceforge.net)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# http://liar.bramz.net/


import dataclasses
import enum
import keyword
import logging
import math
import os
import re
import sys
import warnings
from dataclasses import dataclass
from typing import Any, Dict, List

import liar
import liar.tools.spd
from liar.tools import ply


class Verbosity(enum.Enum):
    QUIET = logging.CRITICAL
    NORMAL = logging.INFO
    VERBOSE = logging.DEBUG


def parse(path, *args, **kwargs):
    scene = PbrtScene(*args, **kwargs)
    scene.parse(path)
    return scene


class PbrtScene(object):
    def __init__(
        self,
        verbosity=Verbosity.NORMAL,
        render_immediately=True,
        display=True,
        threads=None,
    ):
        self.__logger = logging.getLogger("liar.tools.pbrt")
        self.__logger.setLevel(verbosity.value)
        self.engine = liar.RenderEngine()
        self.engine.numberOfThreads = (
            threads or liar.RenderEngine.AUTO_NUMBER_OF_THREADS
        )
        self.Identity()
        self.__state = State.OPTIONS_BLOCK
        self.__named_coordinate_systems = {}
        self.__render_immediately = render_immediately
        self.__display = display
        self.__named_materials = {}
        self.Camera()
        self.PixelFilter()
        self.Film()
        self.SurfaceIntegrator()
        self.Sampler()

    def Identity(self):
        self.__cur_transform = liar.Transformation3D.identity()

    def Translate(self, x, y, z):
        self.__cur_transform = liar.Transformation3D.translation((x, y, z)).concatenate(
            self.__cur_transform
        )

    def Scale(self, x, y, z):
        self.__cur_transform = liar.Transformation3D.scaler((x, y, z)).concatenate(
            self.__cur_transform
        )

    def Rotate(self, angle, x, y, z):
        self.__cur_transform = liar.Transformation3D.rotation(
            (x, y, z), math.radians(angle)
        ).concatenate(self.__cur_transform)

    def LookAt(self, ex, ey, ez, lx, ly, lz, ux, uy, uz):
        # defining a LHS using RHS operations?  wtf? pretend like nothing happened!
        from liar.tools.geometry import cross_product, negate, normalize, subtract

        eye = ex, ey, ez
        lookat = lx, ly, lz
        up = ux, uy, uz
        dir = normalize(subtract(lookat, eye))
        right = normalize(cross_product(dir, up))
        up = cross_product(right, dir)
        left = negate(right)
        self.__cur_transform = (
            liar.Transformation3D(eye, left, up, dir)
            .inverse()
            .concatenate(self.__cur_transform)
        )

    def CoordinateSystem(self, name):
        self.__named_coordinate_systems[name] = self.__cur_transform

    def CoordSysTransform(self, name):
        self.__cur_transform = self.__named_coordinate_systems[name]

    def Transform(self, *m):
        m = _unwrap(m)
        assert len(m) == 16
        self.__cur_transform = liar.Transformation3D(_transpose(m))

    def ConcatTransform(self, *m):
        m = _unwrap(m)
        assert len(m) == 16
        self.__cur_transform = liar.Transformation3D(_transpose(m)).concatenate(
            self.__cur_transform
        )

    def ReverseOrientation(self):
        # quick hack
        self.Scale(-1, -1, -1)

    def WorldBegin(self):
        self.verify_options()
        self.Identity()
        self.CoordinateSystem("world")
        self.__state = State.WORLD_BLOCK
        self.__area_light = None
        self.__volume = None
        self.__pushed_transforms = []
        self.__pushed_states = []
        self.__textures = {}
        self.__auto_textures = {}
        self.__objects = []
        self.__instances = {}
        self.__cur_instance = None
        self.__fourier_materials = {}
        self.Material()

    def WorldEnd(self):
        engine, camera = self.engine, self.engine.camera
        (width, height) = self.resolution
        try:
            engine.sampler.resolution = (width, height)
        except AttributeError:
            pass
        camera.aspectRatio = self.__frameaspectratio or (float(width) / height)
        if camera.aspectRatio > 1:
            camera.fovAngle = 2 * math.atan(
                math.tan(0.5 * math.radians(self.__fov)) * camera.aspectRatio
            )
        else:
            camera.fovAngle = math.radians(self.__fov)
        engine.target = self.__film
        if self.__display:
            try:
                Display = liar.output.Display
            except AttributeError:
                self.__logger.warning(
                    "no display output possible, unsupported by LiAR installation"
                )
            else:
                w, h = width, height
                if w > 1600:
                    w = 1600
                    h = w / camera.aspectRatio
                if h > 900:
                    h = 900
                    w = h * camera.aspectRatio
                display = Display("PBRT powered by LiAR!", (int(w), int(h)))
                display.exposureStops = getattr(engine.target, "exposureStops", 0)
                engine.target = liar.output.Splitter([engine.target, display])
        if self.__pixelFilter:
            self.__pixelFilter.target, engine.target = engine.target, self.__pixelFilter

        meshes = [
            shape
            for shape in self.__objects
            if isinstance(shape, liar.scenery.TriangleMesh)
        ]
        if len(meshes) > 1:
            composite = liar.scenery.TriangleMeshComposite(meshes)
            self.__objects = [
                shape
                for shape in self.__objects
                if not isinstance(shape, liar.scenery.TriangleMesh)
            ]
            self.__objects.append(composite)
            print(f"Combined {len(meshes)} triangle meshes in WorldEnd")

        engine.scene = liar.scenery.AabbTree(self.__objects)
        engine.scene.interior = self.__volume
        if self.__render_immediately:
            self.render()

    def TransformBegin(self):
        self.verify_world()
        self.__pushed_transforms.append(self.__cur_transform)

    def TransformEnd(self):
        self.verify_world()
        self.__cur_transform = self.__pushed_transforms.pop()

    def AttributeBegin(self):
        self.TransformBegin()
        self.__pushed_states.append(
            (self.__textures.copy(), self.__material, self.__area_light)
        )

    def AttributeEnd(self):
        self.TransformEnd()
        self.__textures, self.__material, self.__area_light = self.__pushed_states.pop()

    def ObjectBegin(self, name):
        self.AttributeBegin()
        assert (
            self.__cur_instance is None
        ), "ObjectBegin called inside of instance definition"
        self.__cur_instance = self.__instances[name] = []

    def ObjectEnd(self):
        self.AttributeEnd()
        assert self.__cur_instance is not None

        meshes = [
            shape
            for shape in self.__cur_instance
            if isinstance(shape, liar.scenery.TriangleMesh)
        ]
        if len(meshes) > 1:
            composite = liar.scenery.TriangleMeshComposite(meshes)
            self.__cur_instance[:] = [
                shape
                for shape in self.__cur_instance
                if not isinstance(shape, liar.scenery.TriangleMesh)
            ]
            self.__cur_instance.append(composite)

        self.__cur_instance = None

    def ObjectInstance(self, name):
        self.verify_world()
        for shape in self.__instances[name]:
            self.__add_shape(shape)

    def __add_shape(self, shape):
        self.verify_world()
        shape = self.__with_cur_transform(shape)
        if self.__cur_instance is not None:
            self.__cur_instance.append(shape)
        else:
            self.__objects.append(shape)

    def __with_cur_transform(self, shape):
        # todo: merge __cur_transform with shape's transform.
        if self.__cur_transform.isIdentity():
            return shape
        if self.__cur_transform.isTranslation():
            matrix = self.__cur_transform.matrix
            return liar.scenery.Translation(shape, matrix[3:12:4])
        return liar.scenery.Transformation(shape, self.__cur_transform)

    def Camera(
        self,
        name="perspective",
        frameaspectratio=None,
        screenwindow=None,
        fov=90,
        **kwargs,
    ):
        self.verify_options()
        self.__frameaspectratio, self.__screenwindow, self.__fov = (
            frameaspectratio,
            screenwindow,
            fov,
        )
        camera_space = self.__cur_transform.inverse()
        self.__named_coordinate_systems["camera"] = camera_space
        mat = camera_space.matrix
        right, up, direction, position = [mat[k:12:4] for k in range(4)]
        self.engine.camera = getattr(self, "_camera_" + name)(
            position, direction, up, right, **kwargs
        )

    def _camera_perspective(
        self,
        position,
        direction,
        up,
        right,
        hither=10e-3,
        yon=10e30,
        shutteropen=0,
        shutterclose=1,
        lensradius=0,
        focaldistance=10e30,
        _falloff=0,
    ):
        from liar.tools.geometry import negate

        camera = liar.cameras.PerspectiveCamera()
        camera.position, camera.sky, camera.direction = position, up, direction
        camera.right, camera.down = right, negate(up)  # -up for down
        camera.nearLimit, camera.farLimit = hither, yon
        camera.shutterOpenDelta, camera.shutterCloseDelta = shutteropen, shutterclose
        camera.lensRadius, camera.focusDistance = lensradius, focaldistance
        camera.falloffPower = _falloff
        return camera

    def Sampler(self, name="halton", **kwargs):
        self.verify_options()
        self.engine.sampler = getattr(self, "_sampler_" + name)(**kwargs)

    def _sampler_stratified(
        self, jitter=True, xsamples=2, ysamples=2, pixelsamples=None
    ):
        if pixelsamples:
            sampler = liar.samplers.LatinHypercube()
            sampler.samplesPerPixel = pixelsamples
        else:
            sampler = liar.samplers.Stratifier()
            sampler.samplesPerPixel = xsamples * ysamples
        sampler.jittered = jitter
        return sampler

    def _sampler_halton(self):
        return liar.samplers.Halton()

    def Film(
        self,
        name="image",
        xresolution=640,
        yresolution=480,
        cropwindow=(0, 1, 0, 1),
        **kwargs,
    ):
        self.verify_options()
        self.resolution = (xresolution, yresolution)
        xstart, xend, ystart, yend = cropwindow
        self.cropwindow = (xstart, ystart), (xend, yend)
        self.__film = getattr(self, "_film_" + name)(**kwargs)

    def _film_image(
        self, filename="pbrt.exr", writefrequency=-1, premultiplyalpha=True, scale=1
    ):
        film = liar.output.Image(filename, self.resolution)
        film.exposureStops = math.log2(scale)
        return film

    def PixelFilter(self, name="box", **kwargs):
        self.verify_options()
        self.__pixelFilter = getattr(self, "_pixelfilter_" + name)(**kwargs)

    def _pixelfilter_box(self):
        return None

    def _pixelfilter_mitchell(self, xwidth=2, ywidth=2, B=1.0 / 3, C=None):
        return liar.output.FilterMitchell(None, B)

    def _pixelfilter_triangle(self, xwidth=2, ywidth=2):
        width = math.sqrt(xwidth * ywidth)
        return liar.output.FilterTriangle(None, width)

    def Shape(self, name, **kwargs):
        self.verify_world()
        shape = getattr(self, "_shape_" + name)(**kwargs)
        shape.shader = self.__material
        if self.__area_light:
            light_name, light_kwargs = self.__area_light
            shape = getattr(self, "_lightsource_" + light_name)(
                shape=shape, **light_kwargs
            )
            shape.surface.shader = liar.shaders.Unshaded(
                liar.textures.Constant(shape.radiance)
            )
        self.__add_shape(shape)

    def _shape_disk(self, height=0, radius=1):
        return liar.scenery.Disk((0, 0, height), (0, 0, 1), radius)

    def _shape_sphere(self, radius=1):
        return liar.scenery.Sphere((0, 0, 0), radius)

    def _shape_trianglemesh(self, indices, P, N=None, uv=None, st=None):
        def split_as_tuples(xs, n):
            assert len(xs) % n == 0
            return [tuple(xs[k : k + n]) for k in range(0, len(xs), n)]

        uv = uv or st
        if N and uv:
            as_vertex_index = lambda i: (i, i, i)
        elif N:
            as_vertex_index = lambda i: (i, i)
        elif uv:
            as_vertex_index = lambda i: (i, None, i)
        else:
            as_vertex_index = lambda i: (i,)

        uv = split_as_tuples(uv or [], 2)
        triangles = split_as_tuples([as_vertex_index(i) for i in indices], 3)
        return liar.scenery.TriangleMesh(P, N or [], uv or [], triangles)

    def _shape_loopsubdiv(self, nlevels=3, *args, **kwargs):
        mesh = self._shape_trianglemesh(*args, **kwargs)
        mesh.loopSubdivision(nlevels)
        mesh.smoothNormals()
        return mesh

    def _shape_plymesh(self, filename, alpha=None, shadowalpha=None):
        mesh = ply.load(filename)
        if alpha:
            mesh = liar.scenery.ClipMap(mesh, self._get_texture(alpha), 0.001)
        if shadowalpha:
            assert shadowalpha is alpha
        return mesh

    def Accelerator(self, *args, **kwargs):
        self.verify_options()
        # we ignore this

    def Material(self, name="matte", **kwargs):
        self.verify_world()
        self.__material = self._Material(name, **kwargs)

    def MakeNamedMaterial(self, name, **kwargs):
        self.verify_world()  # ?
        type_ = kwargs.pop("type")
        self.__named_materials[name] = self._Material(type_, **kwargs)

    def NamedMaterial(self, name):
        self.verify_world()  # ?
        self.__material = self.__named_materials[name]

    def _Material(self, name="matte", bumpmap=None, **kwargs):
        self.verify_world()
        try:
            maker = getattr(self, "_material_" + name)
        except AttributeError:
            self.__logger.warning("unknown material %r", name)
            kwargs = {
                key: value for key, value in kwargs.items() if key in ("Kd", "sigma")
            }
            material = self._material_matte(**kwargs)
        else:
            material = maker(**kwargs)
        if bumpmap:
            material = liar.shaders.BumpMapping(material, self._get_texture(bumpmap))
        return material

    def _material_bluepaint(self):
        diffuse = self._get_texture((0.3094, 0.39667, 0.70837))
        mat = liar.shaders.Lafortune(diffuse)
        xy = self._get_texture((0.870567, 0.857255, 0.670982))
        z = self._get_texture((0.803624, 0.774290, 0.586674))
        e = self._get_texture((21.820103, 18.597755, 7.472717))
        mat.addLobe(xy, xy, z, e)
        xy = self._get_texture((-0.451218, -0.406681, -0.477976))
        z = self._get_texture((0.023123, 0.017625, 0.227295))
        e = self._get_texture((2.774499, 2.581499, 3.677653))
        mat.addLobe(xy, xy, z, e)
        xy = self._get_texture((-1.031545, -1.029426, -1.026588))
        z = self._get_texture((0.706734, 0.696530, 0.687715))
        e = self._get_texture((66.899060, 63.767912, 57.489181))
        mat.addLobe(xy, xy, z, e)
        return mat

    def _material_clay(self):
        diffuse = self._get_texture((0.383626, 0.260749, 0.274207))
        mat = liar.shaders.Lafortune(diffuse)
        xy = self._get_texture((-1.089701, -1.102701, -1.107603))
        z = self._get_texture((-1.354682, -2.714801, -1.569866))
        e = self._get_texture((17.968505, 11.024489, 21.270282))
        mat.addLobe(xy, xy, z, e)
        xy = self._get_texture((-0.733381, -0.793320, -0.848206))
        z = self._get_texture((0.676108, 0.679314, 0.726031))
        e = self._get_texture((8.219745, 9.055139, 11.261951))
        mat.addLobe(xy, xy, z, e)
        xy = self._get_texture((-1.010548, -1.012378, -1.011263))
        z = self._get_texture((0.910783, 0.885239, 0.892451))
        e = self._get_texture((152.912795, 141.937171, 201.046802))
        mat.addLobe(xy, xy, z, e)
        return mat

    def _material_felt(self):
        diffuse = self._get_texture((0.025865, 0.025865, 0.025865))
        mat = liar.shaders.Lafortune(diffuse)
        xy = self._get_texture((-0.304075, -0.304075, -0.304075))
        z = self._get_texture((-0.065992, -0.065992, -0.065992))
        e = self._get_texture((3.047892, 3.047892, 3.047892))
        mat.addLobe(xy, xy, z, e)
        xy = self._get_texture((-0.749561, -0.749561, -0.749561))
        z = self._get_texture((-1.167929, -1.167929, -1.167929))
        e = self._get_texture((6.931827, 6.931827, 6.931827))
        mat.addLobe(xy, xy, z, e)
        xy = self._get_texture((1.004921, 1.004921, 1.004921))
        z = self._get_texture((-0.205529, -0.205529, -0.205529))
        e = self._get_texture((94.117332, 94.117332, 94.117332))
        mat.addLobe(xy, xy, z, e)
        return mat

    def _material_fourier(self, *, bsdffile: str):
        try:
            return self.__fourier_materials[bsdffile]
        except KeyError:
            mat = liar.shaders.Jakob(bsdffile)
            self.__fourier_materials[bsdffile] = mat
            return mat

    def _material_glass(
        self,
        Kr=1,
        Kt=1,
        eta=1.5,
        index=None,
        uroughness=0,
        vroughness=0,
        remaproughness=True,
    ):
        if index is not None:
            eta = index
        if uroughness or vroughness:
            uroughness = _remap_roughness(uroughness, remaproughness=remaproughness)
            vroughness = _remap_roughness(vroughness, remaproughness=remaproughness)
            glass = liar.shaders.Walter(self._get_texture(eta))
            glass.roughnessU = self._get_texture(uroughness)
            glass.roughnessV = self._get_texture(vroughness)
        else:
            glass = liar.shaders.Dielectric(self._get_texture(eta))
        glass.reflectance = self._get_texture(Kr)
        glass.transmittance = self._get_texture(Kt)
        return glass

    def _material_matte(self, Kd=1, sigma=0):
        return liar.shaders.Lambert(self._get_texture(Kd))

    def _material_metal(
        self,
        eta,
        k,
        roughness=0.01,
        uroughness=None,
        vroughness=None,
        remaproughness=True,
    ):
        if uroughness is None:
            uroughness = roughness
        if vroughness is None:
            vroughness = roughness
        uroughness = _remap_roughness(uroughness, remaproughness=remaproughness)
        vroughness = _remap_roughness(vroughness, remaproughness=remaproughness)
        material = liar.shaders.CookTorrance(
            self._get_texture(eta), self._get_texture(k)
        )
        material.roughnessU = self._get_texture(uroughness)
        material.roughnessV = self._get_texture(vroughness)
        return material

    def _material_mirror(self, Kr=1):
        return liar.shaders.Mirror(self._get_texture(Kr))

    def _material_mix(self, *, amount=0.5, namedmaterial1: str, namedmaterial2: str):
        """
        According to PBRT v3 specs: A value of one corresponds to just "namedmaterial1",
        a value of zero corresponds to just "namedmaterial2"
        """
        return liar.shaders.LinearInterpolator(
            [
                (0, self.__named_materials[namedmaterial2]),
                (1, self.__named_materials[namedmaterial1]),
            ],
            self._get_texture(amount),
        )

    def _material_plastic(self, Kd=1, Ks=1, roughness=0.1, remaproughness=True):
        return self._material_substrate(
            Kd=Kd,
            Ks=Ks,
            uroughness=roughness,
            vroughness=roughness,
            remaproughness=remaproughness,
        )

    def _material_primer(self):
        diffuse = self._get_texture((0.118230, 0.121218, 0.133209))
        mat = liar.shaders.Lafortune(diffuse)
        xy = self._get_texture((-0.399286, -1.033473, -1.058104))
        z = self._get_texture((0.167504, 0.009545, -0.068002))
        e = self._get_texture((2.466633, 7.637253, 8.117645))
        mat.addLobe(xy, xy, z, e)
        xy = self._get_texture((-1.041861, -1.100108, -1.087779))
        z = self._get_texture((0.014375, -0.198147, -0.053605))
        e = self._get_texture((7.993722, 29.446268, 41.988990))
        mat.addLobe(xy, xy, z, e)
        xy = self._get_texture((-1.098605, -0.379883, -0.449038))
        z = self._get_texture((-0.145110, 0.159127, 0.173224))
        e = self._get_texture((31.899719, 2.372852, 2.636161))
        mat.addLobe(xy, xy, z, e)
        return mat

    def _material_skin(self):
        diffuse = self._get_texture((0.428425, 0.301341, 0.331054))
        mat = liar.shaders.Lafortune(diffuse)
        xy = self._get_texture((-1.131747, -1.016939, -0.966018))
        z = self._get_texture((-1.209182, -1.462488, -1.222419))
        e = self._get_texture((6.421658, 3.699932, 3.524889))
        mat.addLobe(xy, xy, z, e)
        xy = self._get_texture((-0.546570, -0.643533, -0.638934))
        z = self._get_texture((0.380123, 0.410559, 0.437367))
        e = self._get_texture((3.685044, 4.266495, 4.539742))
        mat.addLobe(xy, xy, z, e)
        xy = self._get_texture((-0.998888, -1.020153, -1.027479))
        z = self._get_texture((0.857998, 0.703913, 0.573625))
        e = self._get_texture((64.208486, 63.919687, 43.809866))
        mat.addLobe(xy, xy, z, e)
        return mat

    def _material_substrate(
        self, Kd=0.5, Ks=0.5, uroughness=0.1, vroughness=0.1, remaproughness=True
    ):
        uroughness = _remap_roughness(uroughness, remaproughness=remaproughness)
        vroughness = _remap_roughness(vroughness, remaproughness=remaproughness)
        shader = liar.shaders.AshikhminShirley(
            self._get_texture(Kd), self._get_texture(Ks)
        )
        shader.roughnessU = self._get_texture(uroughness)
        shader.roughnessV = self._get_texture(vroughness)
        return shader

    def _material_translucent(
        self,
        Kd=0.25,
        Ks=0.25,
        reflect=0.5,
        transmit=0.5,
        roughness=0.1,
        remaproughness=True,
    ):
        layers = []
        if Kd:
            if _luminance(reflect) > 0.001:
                layers.append(
                    liar.shaders.Lambert(
                        liar.textures.Product(
                            [self._get_texture(Kd), self._get_texture(reflect),]
                        )
                    )
                )
            if _luminance(transmit) > 0.001:
                layers.append(
                    liar.shaders.Flip(
                        liar.shaders.Lambert(
                            liar.textures.Product(
                                [self._get_texture(Kd), self._get_texture(transmit),]
                            )
                        )
                    )
                )
        if Ks:
            roughness = _remap_roughness(roughness, remaproughness=remaproughness)
            layer = liar.shaders.Walter(self._get_texture(1.5))
            layer.reflectance = liar.textures.Product(
                [self._get_texture(Ks), self._get_texture(reflect),]
            )
            layer.transmittance = liar.textures.Product(
                [self._get_texture(Ks), self._get_texture(transmit),]
            )
            layer.roughnessU = layer.roughnessV = self._get_texture(roughness)
            layers.append(layer)

        if len(layers) == 1:
            return layers[0]
        else:
            return liar.shaders.Sum(layers)

    def _material_uber(
        self,
        *,
        Kd=0.25,
        Ks=0.25,
        Kr=0,
        Kt=0,
        roughness=0.1,
        uroughness=None,
        vroughness=None,
        opacity=1,
        eta=None,
        index=None,
        remaproughness: bool = True,
    ):
        if uroughness is None:
            uroughness = roughness
        if vroughness is None:
            vroughness = roughness
        if eta is None:
            eta = index or 1.5
        else:
            assert index is None, "uber: eta and index cannot both be set."

        layers = []
        if Ks:
            layers.append(
                self._material_substrate(
                    Kd=Kd,
                    Ks=Ks,
                    uroughness=uroughness,
                    vroughness=vroughness,
                    remaproughness=remaproughness,
                )
            )
        elif Kd:
            layers.append(self._material_matte(Kd=Kd))
        if Kr or Kt:
            layers.append(self._material_glass(Kr=Kr, Kt=Kt, eta=eta))
        if len(layers) == 1:
            mat = layers[0]
        else:
            mat = liar.shaders.Sum(layers)
        if _luminance(opacity) < 0.999:
            transparent = liar.shaders.Flip(liar.shaders.Mirror(self._get_texture(1)))
            mat = liar.shaders.LinearInterpolator(
                [(0, transparent), (1, mat)], self._get_texture(opacity)
            )
        return mat

    def Texture(
        self,
        name,
        type,
        class_,
        mapping="uv",
        uscale=1,
        vscale=1,
        udelta=0,
        vdelta=0,
        v1=(1, 0, 0),
        v2=(0, 1, 0),
        **kwargs,
    ):
        self.verify_world()
        tex = getattr(self, "_texture_" + class_)(**kwargs)
        if mapping != "uv":
            self.__logger.warning(
                "at this point, we don't support mappings other than uv"
            )
        if (uscale, vscale, udelta, vdelta) != (1, 1, 0, 0):
            transform = liar.Transformation2D(
                [uscale, 0, udelta, 0, vscale, vdelta, 0, 0, 1]
            )
            if isinstance(tex, liar.textures.TransformationUv):
                tex.transformation = transform.concatenate(tex.transformation)
            else:
                tex = liar.textures.TransformationUv(tex, transform)
        self.__textures[name] = tex

    def _texture_checkerboard(self, tex1=1, tex2=0, dimension=2, aamode="closedform"):
        tex1, tex2 = self._get_texture(tex1), self._get_texture(tex2)
        assert dimension in (2, 3)
        if dimension == 3:
            tex = liar.textures.CheckerVolume(tex1, tex2)
        else:
            tex = liar.textures.CheckerBoard(tex1, tex2)
            assert aamode in ("none", "closedform", "supersample"), aamode
            tex.antiAliasing = ("none", "bilinear")[aamode != "none"]
            tex = liar.textures.TransformationUv(tex, liar.Transformation2D.scaler(0.5))
        return tex

    def _texture_constant(self, value):
        return self._get_texture(value)

    def _texture_fbm(self, octaves=8, roughness=0.5):
        return liar.textures.FBm(octaves, roughness)

    def _texture_imagemap(
        self, filename, wrap="repeat", maxanisotropy=8, trilinear=False, gamma=1
    ):
        return liar.textures.Image(filename, _RGB_SPACE.withGamma(gamma))

    def _texture_mix(self, tex1=0, tex2=1, amount=0.5):
        tex1, tex2, amount = (
            self._get_texture(tex1),
            self._get_texture(tex2),
            self._get_texture(amount),
        )
        return liar.textures.LinearInterpolator([(0, tex1), (1, tex2)], amount)

    def _texture_scale(self, tex1=1, tex2=1):
        tex1, tex2 = self._get_texture(tex1), self._get_texture(tex2)
        return liar.textures.Product([tex1, tex2])

    def _texture_windy(self):
        from liar.textures import Abs, FBm, Product, TransformationLocal

        waveHeight = FBm(6)
        windStrength = Abs(
            TransformationLocal(FBm(3), liar.Transformation3D.scaler(0.1))
        )
        return Product(waveHeight, windStrength)

    def _get_texture(self, arg):
        # arg is already a texture
        if isinstance(arg, liar.Texture):
            return arg
        # arg as a texture name
        try:
            return self.__textures[arg]
        except (KeyError, TypeError):
            pass
        # arg as a cached texture
        try:
            return self.__auto_textures[arg]
        except (KeyError, TypeError):
            pass
        tex = None
        # arg as a path to an image file
        try:
            if os.path.isfile(arg):
                tex = liar.textures.Image(arg, _RGB_SPACE)
        except TypeError:
            pass
        if not tex:
            # an RGB triple
            try:
                r, g, b = arg
            except TypeError:
                pass
            else:
                try:
                    tex = liar.textures.Constant(_color(r, g, b))
                except TypeError:
                    pass
        if not tex:
            # a gray value
            try:
                tex = liar.textures.Constant(arg)
            except TypeError:
                pass
        if not tex:
            raise ValueError(
                "%(arg)r is nor a registered texture name, nor a imagemap path, nor an RGB triple, nor a single float"
                % vars()
            )
        try:
            self.__auto_textures[arg] = tex
        except TypeError:
            pass
        return tex

    def Volume(self, name, p0=(0, 0, 0), p1=(1, 1, 1), **kwargs):
        self.verify_world()
        shader = getattr(self, "_volume_" + name)(**kwargs)
        try:
            shader.origin = p0
        except AttributeError:
            pass
        self.__volume = liar.mediums.Bounded(shader, (p0, p1))
        if not self.__cur_transform.isIdentity():
            self.__volume = liar.mediums.Transformation(
                self.__volume, self.__cur_transform
            )

    def _volume_homogeneous(self, sigma_a=0, sigma_s=0, g=0, Le=0):
        # let's start off with a simple one ...
        sigma_e = _avg(sigma_a) + _avg(sigma_s)
        fog = liar.mediums.Fog(sigma_e, g)
        fog.emission = _color(Le)
        fog.color = _color(_mul(sigma_s, 0))  # _mul(sigma_s, 1 / sigma_e)
        return fog

    def _volume_exponential(
        self, sigma_a=0, sigma_s=0, g=0, Le=0, a=1, b=1, updir=(0, 1, 0)
    ):
        # let's start off with a simple one ...
        sigma_e = _avg(sigma_a) + _avg(sigma_s)
        fog = liar.mediums.ExponentialFog(a * sigma_e, g)
        fog.emission = _mul(Le, a)
        fog.color = _mul(sigma_s, 0)  # _mul(sigma_s, 1 / sigma_e)
        fog.decay = b
        fog.up = updir
        return fog

    def LightSource(self, name, **kwargs):
        self.verify_world()
        light = getattr(self, "_lightsource_" + name)(**kwargs)
        self.__objects.append(self.__with_cur_transform(light))

    def AreaLightSource(self, name, **kwargs):
        self.verify_world()
        self.__area_light = name, kwargs

    def _lightsource_diffuse(self, shape, L=(1, 1, 1), nsamples=1):
        self.verify_world()
        light = liar.scenery.LightArea(shape)
        light.radiance = _color(L)
        light.numberOfEmissionSamples = nsamples
        return light

    def _lightsource_area(self, shape, L=(1, 1, 1), nsamples=1):
        warnings.warn(
            'AreaLightSource "area" is deprecated, use "diffuse" instead',
            DeprecationWarning,
        )
        return self._lightsource_diffuse(shape, L, nsamples)

    def _lightsource_distant(self, from_=(0, 0, 0), to=(0, 0, 1), L=(1, 1, 1)):
        from liar.tools import geometry

        direction = geometry.subtract(to, from_)
        return liar.scenery.LightDirectional(direction, _color(L))

    def _lightsource_infinite(self, L=(1, 1, 1), nsamples=1, mapname=None, scale=None):
        tex, res = self._get_light_texture(L, mapname)
        if scale:
            tex = liar.textures.Product(tex, liar.textures.Constant(_color(scale)))
        light = liar.scenery.LightSky(tex)
        if res:
            light.samplingResolution = res
        light.numberOfEmissionSamples = nsamples
        light.shader = liar.shaders.Unshaded(light.radiance)
        return light

    def _lightsource_point(self, from_=(0, 0, 0), I=(1, 1, 1)):
        return liar.scenery.LightPoint(from_, _color(I))

    def _lightsource_projection(self, I=(1, 1, 1), fov=45, mapname=None):
        intensity, resolution = self._get_light_texture(I, mapname)
        light = liar.scenery.LightProjection()
        light.intensity = intensity
        light.projection = liar.ProjectionPerspective()
        light.projection.sky, light.projection.direction = (0, 1, 0), (0, 0, 1)
        light.projection.right, light.projection.up = (1, 0, 0), (0, -1, 0)
        if resolution:
            light.samplingResolution = resolution
            light.projection.aspectRatio = resolution[0] / resolution[1]
        else:
            light.projection.aspectRatio = 1
        light.projection.fovAngle = math.radians(fov)
        if light.projection.aspectRatio > 1:
            # PBRT's fov is on shortest edge, but liar's fov is always on width
            # fix by scaling focalLength
            light.projection.focalLength /= light.projection.aspectRatio
        return light

    def _lightsource_spot(
        self, I=(1, 1, 1), from_=(0, 0, 0), to=(0, 0, 1), coneangle=30, conedeltaangle=5
    ):
        light = liar.scenery.LightSpot()
        light.position = from_
        light.lookAt(to)
        light.intensity = _color(I)
        light.outerAngle = math.radians(coneangle)
        light.innerAngle = math.radians(conedeltaangle)
        return light

    def _get_light_texture(self, factor, mapname):
        if not mapname:
            return self._get_texture(multiplier)
        tex = self._get_texture(mapname)
        try:
            res = tex.resolution
        except AttributeError:
            res = None
        try:
            tex.mipMapping = "none"
        except AttributeError:
            pass
        if not (factor == (1, 1, 1) or factor == 1):
            tex = liar.textures.Product(self._get_texture(factor), tex)
        return tex, res

    def Integrator(self, name="path", **kwargs):
        self.verify_options()
        self.engine.tracer = getattr(self, "_integrator_" + name)(**kwargs)

    def _integrator_path(self, maxdepth=5):
        tracer = liar.tracers.AdjointPhotonTracer()
        tracer.maxRayGeneration = maxdepth
        return tracer

    def SurfaceIntegrator(self, name="directlighting", **kwargs):
        self.verify_options()
        self.engine.tracer = getattr(self, "_surfaceintegrator_" + name)(**kwargs)

    def _surfaceintegrator_directlighting(self, maxdepth=5, strategy="all"):
        tracer = liar.tracers.DirectLighting()
        tracer.maxRayGeneration = maxdepth
        return tracer

    def _surfaceintegrator_photonmap(
        self,
        causticphotons=20000,
        indirectphotons=100000,
        directphotons=100000,
        nused=50,
        maxdepth=5,
        maxdist=0.1,
        finalgather=True,
        finalgathersamples=32,
        directwithphotons=False,
    ):
        tracer = liar.tracers.PhotonMapper()
        tracer.globalMapSize = indirectphotons
        tracer.causticsQuality = (100.0 * causticphotons) / indirectphotons
        self.__logger.info("causticsQuality=%r" % tracer.causticsQuality)
        if finalgather or directwithphotons:
            tracer.globalMapSize += directphotons
        for k in ("global", "caustic", "volume"):
            tracer.setEstimationSize(k, nused)
            # tracer.setEstimationRadius(k, maxdist)
        tracer.numFinalGatherRays = finalgather and finalgathersamples
        tracer.isRayTracingDirect = not directwithphotons
        return tracer

    def VolumeIntegrator(self, *args, **kwargs):
        self.verify_options()
        # we don't do special settings yet

    def Renderer(self, *args, **kwargs):
        self.verify_options()
        # we don't do special settings yet

    def verify_options(self):
        assert self.__state == State.OPTIONS_BLOCK

    def verify_world(self):
        assert self.__state == State.WORLD_BLOCK

    def parse(self, path: str):
        if path == "-":
            return self.parse_stream("stdin", sys.stdin)
        with open(path) as fp:
            return self.parse_stream(path, fp)

    def parse_stream(self, path: str, stream):
        statement = None
        arg_type, arg_name = None, None
        tokens = _scanner(path, stream)
        for token in tokens:
            if token.type_ == TokenType.IDENTIFIER:
                if statement:
                    self._execute_statement(statement)
                    statement = None
                if token.value == "Include":
                    token = next(tokens)
                    assert token.type_ == TokenType.STRING, (
                        "syntax error in file %(path)r, line %(token.line_number)d: Include must be followed by a string"
                        % vars()
                    )
                    self.__logger.debug("Include %r" % token.value)
                    self.parse(token.value)
                else:
                    statement = Statement(token.line_number, token.value)
            elif token.type_ == TokenType.PARAMETER:
                # assert not token.value in kwargs
                arg_type, arg_name = token.value
                if keyword.iskeyword(arg_name):
                    arg_name += "_"
            else:
                if token.type_ == TokenType.START_LIST:
                    value = []
                    for token in tokens:
                        if token.type_ == TokenType.END_LIST:
                            break
                        assert token.type_ in (TokenType.NUMBER, TokenType.STRING), (
                            "syntax error in file %(path)r, line %(token.line_number)d: parameter lists should only contain numbers and strings"
                            % vars()
                        )
                        value.append(token.value)
                else:
                    value = [token.value]
                if arg_type:
                    value = getattr(self, "_arg_" + arg_type)(*value)
                    arg_type = None
                value = _unwrap(tuple(value))
                if arg_name:
                    statement.kwargs[arg_name] = value
                    arg_name = None
                else:
                    statement.args.append(value)
        if statement:
            self._execute_statement(statement)

    def _execute_statement(self, statement: "Statement") -> None:
        type_ = statement.keyword
        if keyword.iskeyword(type_):
            type_ += "_"
        self.__print_statement(statement)
        getattr(self, type_)(*statement.args, **statement.kwargs)

    def _arg_integer(self, *values):
        return map(int, values)

    def _arg_float(self, *values):
        return map(float, values)

    def _arg_point(self, *values):
        return _split_as_tuples(values, 3)

    def _arg_vector(self, *values):
        return _split_as_tuples(values, 3)

    def _arg_normal(self, *values):
        return _split_as_tuples(values, 3)

    def _arg_bool(self, *values):
        return map(bool, values)

    def _arg_string(self, *values):
        return map(str, values)

    def _arg_texture(self, *values):
        return map(self._get_texture, values)

    def _arg_xyz(self, *values):
        return map(liar.Spectrum.XYZ, _split_as_tuples(values, 3))

    def _arg_rgb(self, *values):
        return [liar.rgb(rgb, _RGB_SPACE) for rgb in _split_as_tuples(values, 3)]

    _arg_color = _arg_rgb

    def _arg_blackbody(self, temperature, scale=1):
        blackbody = liar.spectra.BlackBody(temperature)
        blackbody.scale = scale
        return [blackbody]

    def _arg_spectrum(self, *values):
        if len(values) == 1:
            (filename,) = values
            assert isinstance(filename, str), filename
            with open(filename) as fp:
                return liar.tools.spd.load(fp)
        wavelengths, values = zip(*_split_as_tuples(values, 2))
        return [liar.spectra.Sampled(wavelengths, values)]

    def __print_statement(self, statement: "Statement") -> None:
        def truncated_repr(x, n=80):
            s = repr(x)
            if len(s) <= n:
                return s
            trunc = " ..."
            if s[-1] in "'\")]}>":
                trunc += s[-1]
            return s[: n - len(trunc)] + trunc

        args = [truncated_repr(value) for value in statement.args]
        args += [
            f"{key}={truncated_repr(value)}" for key, value in statement.kwargs.items()
        ]
        fmt = f"%s: %s ({', '.join(('%s',) * len(args))})"
        self.__logger.debug(fmt, statement.line_number, statement.keyword, *args)

    def render(self):
        self.engine.render(self.cropwindow)


class State(enum.Enum):
    OPTIONS_BLOCK = 1
    WORLD_BLOCK = 2


class TokenType(enum.Enum):
    IDENTIFIER = 1
    NUMBER = 2
    STRING = 3
    PARAMETER = 4
    START_LIST = 5
    END_LIST = 6


@dataclass
class Token:
    type_: TokenType
    value: Any
    line_number: int


@dataclass
class Statement:
    line_number: int
    keyword: str
    args: List[Any] = dataclasses.field(default_factory=list)
    kwargs: Dict[str, Any] = dataclasses.field(default_factory=dict)


def _scanner(path, stream):
    scanner = re.Scanner(
        [
            (r"[a-zA-Z_]\w*", lambda _, token: (TokenType.IDENTIFIER, token)),
            (
                r"[\-+]?(\d+\.\d*|\.\d+)([eE][\-+]?[0-9]+)?",
                lambda _, token: (TokenType.NUMBER, float(token)),
            ),
            (r"[\-+]?\d+", lambda _, token: (TokenType.NUMBER, int(token))),
            (r"\[", lambda _, token: (TokenType.START_LIST, token)),
            (r"\]", lambda _, token: (TokenType.END_LIST, token)),
            (
                r'"(integer|float|point|vector|normal|bool|string|texture|color|rgb|xyz|blackbody|spectrum)\s+[a-zA-Z_][a-zA-Z0-9_]*"',
                lambda _, token: (TokenType.PARAMETER, token[1:-1].split()),
            ),
            (r'"true"', lambda _, token: (TokenType.NUMBER, True)),
            (r'"false"', lambda _, token: (TokenType.NUMBER, False)),
            (r'".*?"', lambda _, token: (TokenType.STRING, token[1:-1])),
            (r"#.*$", None),  # comments
            (r"\s+", None),  # whitespace
        ]
    )
    for line_number, line in enumerate(stream, start=1):
        tokens, rest = scanner.scan(line.rstrip("\n"))
        assert not rest, (
            "syntax error in file %(path)r, line %(line_number)d: %(rest)r" % vars()
        )
        for token_type, token in tokens:
            yield Token(token_type, token, line_number)


def _unwrap(arg):
    """
    _unwrap( (x,) ) -> x
    _unwrap( x ) -> x

    if arg is a sequence of one item, return that item, otherwise return arg.
    """
    try:
        (x,) = arg
    except (TypeError, ValueError):
        x = arg
    return x


def _split_as_tuples(xs, n):
    assert len(xs) % n == 0
    return [tuple(xs[k : k + n]) for k in range(0, len(xs), n)]


def _transpose(m):
    """
    transpose a 4x4 matrix
    """
    # fmt: off
    assert len(m) == 16
    return [
        m[0],  m[4],  m[8],  m[12],
        m[1],  m[5],  m[9],  m[13],
        m[2],  m[6], m[10],  m[14],
        m[3],  m[7], m[11],  m[15],
    ]
    # fmt: on


def _avg(arg):
    try:
        return float(arg)
    except TypeError:
        return float(sum(arg)) / len(arg)


def _mul(arg, factor):
    try:
        return [x * factor for x in arg]
    except TypeError:
        return factor * arg


def _color(*args):
    if len(args) == 1:
        arg = args[0]
        if isinstance(arg, liar.Spectrum):
            return arg
        try:
            return liar.Spectrum.Flat(float(arg))
        except TypeError:
            r, g, b = arg
    else:
        r, g, b = args
    return liar.rgb(r, g, b, _RGB_SPACE)


def _remap_roughness(roughness, *, remaproughness: bool):
    # PBRT feeds roughness value directly into "alpha" parameter of the microfacet
    # models. Liar uses the more common Disney and UE4 "roughness" parameter, which is
    # the square root of alpha. So we need to remap the roughness value.
    # If remaproughness is True, PBRT also applies a mapping to the roughness value,
    # which is also replicated here.
    if isinstance(roughness, liar.Texture):
        return liar.textures.RemapPbrtRoughness(roughness, remaproughness)

    r = roughness
    roughness = liar.textures.RemapPbrtRoughness.remap(r, remaproughness)
    #print(f"_remap_roughness({r}, {remaproughness}) -> {roughness}")
    return roughness


def _luminance(x) -> float:
    try:
        return x.luminance
    except AttributeError:
        return float(x)


_RGB_SPACE = liar.sRGB.linearSpace()


if __name__ == "__main__":
    # use the module as a commandline script
    logging.basicConfig(level=logging.DEBUG, format="%(message)s")
    from optparse import OptionParser

    parser = OptionParser()
    parser.add_option(
        "-q",
        "--quiet",
        action="store_const",
        const=Verbosity.QUIET,
        dest="verbosity",
        default=Verbosity.NORMAL,
    )
    parser.add_option(
        "-v",
        "--verbose",
        action="store_const",
        const=Verbosity.VERBOSE,
        dest="verbosity",
    )
    parser.add_option(
        "-d",
        "--display",
        action="store_true",
        dest="display",
        default=False,
        help="show progress in preview display [default=%default]",
    )
    parser.add_option(
        "-t",
        "--threads",
        action="store",
        type="int",
        help="number of threads for rendering",
    )
    options, args = parser.parse_args()
    for path in args:
        parse(path, render_immediately=True, **options.__dict__)
