/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2024  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::scenery::TriangleMesh
 *  @brief a simple triangle mesh
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_TRIANGLE_MESH_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_TRIANGLE_MESH_H

#include "scenery_common.h"
#include "../kernel/scene_object.h"
#include "../kernel/texture.h"

#include <lass/prim/triangle_mesh_3d.h>
#include <lass/spat/qbvh_tree.h>
#include <filesystem>

namespace liar
{
namespace scenery
{

class TriangleMesh;
using TTriangleMeshPtr = python::PyObjectPtr<TriangleMesh>::Type;

class LIAR_SCENERY_DLL TriangleMesh: public SceneObject
{
	PY_HEADER(SceneObject)
public:

	typedef prim::TriangleMesh3D<TScalar, spat::QbvhTree, spat::SAHSplitHeuristics> TMesh;
	typedef std::vector<TMesh::TPoint> TVertices;
	typedef TMesh::TNormals TNormals;
	typedef std::vector<TMesh::TUv> TUvs;
	typedef std::vector<TMesh::TIndexTriangle> TIndexTriangles;

	TriangleMesh(TVertices vertices, TNormals normals, TUvs uvs, const TIndexTriangles& triangles);

	void smoothNormals();
	void flatFaces();
	void loopSubdivision(unsigned level);
	void autoSew();
	void autoCrease(unsigned level, TScalar maxAngleInRadians);

	const TVertices& vertices() const;
	const TNormals& normals() const;
	const TUvs& uvs() const;
	const TIndexTriangles triangles() const;

	const TTexturePtr& alphaMask() const;
	Texture::TValue alphaThreshold() const;
	void setAlphaMask(const TTexturePtr& alphaMask);
	void setAlphaThreshold(Texture::TValue alphaThreshold);

#if LIAR_HAVE_HAPPLY
	static TTriangleMeshPtr loadPly(std::filesystem::path path);
#endif

private:

	LASS_UTIL_VISITOR_DO_ACCEPT

	void doIntersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const override;
	bool doIsIntersecting(const Sample& sample, const BoundedRay& ray) const override;
	void doLocalContext(const Sample& sample, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const override;
	bool doContains(const Sample& sample, const TPoint3D& point) const override;
	const TAabb3D doBoundingBox() const override;
	const TSphere3D doBoundingSphere() const override;
	TScalar doArea() const override;
	TScalar doArea(const TVector3D& normal) const override;

	bool doHasSurfaceSampling() const override;
	const TPoint3D doSampleSurface(const TPoint2D& sample, TVector3D& normal, TScalar& pdf) const override;

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

	bool triangleFilter(TMesh::TTriangleIterator triangle, TScalar t, const Sample& sample, const BoundedRay& ray) const;

	TMesh mesh_;
	std::vector<TScalar> cdf_;
	TTexturePtr alphaMask_;
	TScalar area_;
	Texture::TValue alphaThreshold_;
};



}

}

#endif

// EOF
