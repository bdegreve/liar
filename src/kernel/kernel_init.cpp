/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  http://liar.bramz.net/
 */

#include "kernel_common.h"

// keep in alphabetical order please! [Bramz]
//
#include "attenuation.h"
#include "camera.h"
#include "image_codec.h"
#include "medium.h"
#include "observer.h"
#include "projection.h"
#include "projection_perspective.h"
#include "projection_spherical.h"
#include "sampler.h"
#include "ray_tracer.h"
#include "recovery.h"
#include "render_engine.h"
#include "render_target.h"
#include "rgb_space.h"
#include "scene_light.h"
#include "scene_object.h"
#include "shader.h"
#include "spectrum.h"
#include "spline.h"
#include "xyz.h"
#include "texture.h"
#include "transformation.h"

#include <lass/io/proxy_man.h>
#include <lass/io/logger.h>
#include <lass/stde/extended_string.h>
#include <lass/util/process.h>
#include <lass/python/utilities.h>

void license()
{
	std::string text = lass::stde::replace_all(
		liar::license, std::string("\n"), std::string("\\n"));
	lass::python::execute("print \"" + text + "\"\n");
}

liar::TScalar tolerance()
{
	return liar::tolerance;
}

void setTolerance(liar::TScalar tolerance)
{
	liar::tolerance = tolerance;
}

PY_DECLARE_MODULE_DOC(kernel, "LiAR isn't a raytracer")

// keep in alphabetical order please! [Bramz]
//
PY_MODULE_CLASS(kernel, liar::kernel::Attenuation)
PY_MODULE_CLASS(kernel, liar::kernel::Camera)
PY_MODULE_CLASS(kernel, liar::kernel::ImageCodec)
PY_MODULE_CLASS(kernel, liar::kernel::ImageCodecLass)
PY_MODULE_CLASS(kernel, liar::kernel::Medium)
PY_MODULE_CLASS(kernel, liar::kernel::Observer)
PY_MODULE_CLASS(kernel, liar::kernel::Projection)
	PY_MODULE_CLASS(kernel, liar::kernel::ProjectionPerspective)
	PY_MODULE_CLASS(kernel, liar::kernel::ProjectionSpherical)
PY_MODULE_CLASS(kernel, liar::kernel::RgbSpace)
PY_MODULE_CLASS(kernel, liar::kernel::Recovery)
PY_MODULE_CLASS(kernel, liar::kernel::RayTracer)
PY_MODULE_CLASS(kernel, liar::kernel::RenderEngine)
PY_MODULE_CLASS(kernel, liar::kernel::RenderTarget)
PY_MODULE_CLASS(kernel, liar::kernel::Sampler)
PY_MODULE_CLASS(kernel, liar::kernel::ScalarSpline)
	PY_MODULE_CLASS(kernel, liar::kernel::LinearScalarSpline)
	PY_MODULE_CLASS(kernel, liar::kernel::CubicScalarSpline)
PY_MODULE_CLASS(kernel, liar::kernel::SceneObject)
	PY_MODULE_CLASS(kernel, liar::kernel::SceneLight)
PY_MODULE_CLASS(kernel, liar::kernel::Shader)
PY_MODULE_CLASS(kernel, liar::kernel::Spectrum)
PY_MODULE_CLASS(kernel, liar::kernel::Texture)
PY_MODULE_CLASS(kernel, liar::kernel::impl::ShadowTransformation2D)
PY_MODULE_CLASS(kernel, liar::kernel::PyTransformation3D)

PY_MODULE_FUNCTION(kernel, license)
PY_MODULE_FUNCTION(kernel, tolerance)
PY_MODULE_FUNCTION(kernel, setTolerance)

using lass::util::setProcessPriority;
PY_MODULE_FUNCTION_QUALIFIED_DOC_1(kernel, setProcessPriority, void, const std::string&,
	"setProcessPriority('low' | 'belownormal' | 'normal' | 'abovenormal' | 'high')\n")


using liar::kernel::rgb;
using liar::kernel::TSpectrumPtr;
using lass::prim::ColorRGBA;
using liar::kernel::TRgbSpacePtr;
PY_MODULE_FUNCTION_QUALIFIED_DOC_1(kernel, rgb, TSpectrumPtr, const ColorRGBA&,
	"rgb({<R>, <G>, <B>} | <(R, G, B)>} [, <RgbSpace>] [, <SpectrumFormat>])\n"
	"Create an XYZ from RGB color value\n");
PY_MODULE_FUNCTION_QUALIFIED_2(kernel, rgb, TSpectrumPtr, const ColorRGBA&, const TRgbSpacePtr&)
PY_MODULE_FUNCTION_QUALIFIED_3(kernel, rgb, TSpectrumPtr, ColorRGBA::TValue, ColorRGBA::TValue, ColorRGBA::TValue)
PY_MODULE_FUNCTION_QUALIFIED_4(kernel, rgb, TSpectrumPtr, ColorRGBA::TValue, ColorRGBA::TValue, ColorRGBA::TValue, const TRgbSpacePtr&)


using liar::kernel::imageCodecs;
using liar::kernel::transcodeImage;
PY_MODULE_FUNCTION(kernel, imageCodecs)
PY_MODULE_FUNCTION_DOC(kernel, transcodeImage,
	"transcodeImage(sourcePath, destPath, sourceSpace, destSpace) -> None\n"
	"transcode image from one file format to another.\n"
	"- sourcePath: path to source image.\n"
	"- destPath: path to destination image.\n"
	"- sourceSpace: RgbSpace of source image. If None, use image's stored space, format's default space or liar's default space, in that order.\n"
	"- destSpace: RgbSpace of destination image. If None, use same as source image.\n");

void kernelPreInject()
{
	lass::util::setProcessPriority(lass::util::ppBelowNormal);
	lass::io::proxyMan()->cout()->add(&lass::python::sysStdout);
	lass::io::proxyMan()->cerr()->add(&lass::python::sysStderr);
#ifdef _DEBUG
	lass::io::proxyMan()->clog()->add(&lass::python::sysStderr);
#endif
	lass::io::proxyMan()->cout()->remove(&std::cout);
	lass::io::proxyMan()->cerr()->remove(&std::cerr);
	lass::io::proxyMan()->clog()->remove(&std::clog);
}

void kernelPostInject(PyObject*)
{
	PY_INJECT_OBJECT_IN_MODULE_EX(liar::kernel::CIEXYZ, kernel, "CIEXYZ")
	PY_INJECT_OBJECT_IN_MODULE_EX(liar::kernel::sRGB, kernel, "sRGB")

	LASS_COUT << liar::name << " v" << liar::version << " ("
		<< LASS_LIB_PLATFORM "_" LASS_LIB_COMPILER LASS_LIB_DEBUG << ")\n"
		<< "authors: " << liar::authors << "\n"
		<< "website: " << liar::website << "\n"
		<< liar::name << " comes with ABSOLUTELY NO WARRANTY.\n"
		<< "This is free software, and you are welcome to redistribute it \n"
		<< "under certain conditions.  Call license() for details.\n";

	LASS_COUT << "liar.kernel imported (v" LIAR_VERSION_FULL " - " __DATE__ ", " __TIME__ ")\n";
}

LASS_EXECUTE_BEFORE_MAIN(
	kernel.setPreInject(kernelPreInject);
	kernel.setPostInject(kernelPostInject);
	)

PY_MODULE_ENTRYPOINT(kernel)

// EOF
