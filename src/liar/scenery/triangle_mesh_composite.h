/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2025  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::scenery::TriangleMeshTree
 *  @brief composes several triangle meshes into one acceleration tree.
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_TRIANGLE_MESH_COMPOSITE_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_TRIANGLE_MESH_COMPOSITE_H

#include "scenery_common.h"
#include "triangle_mesh.h"

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL TriangleMeshComposite: public SceneObject
{
	PY_HEADER(SceneObject)
public:

	typedef kernel::PyObjectRef<TriangleMesh> TTriangleMeshRef;
	typedef std::vector<TTriangleMeshRef> TChildren;

	TriangleMeshComposite();
	TriangleMeshComposite(const TChildren& children);
	template <typename InputIterator> TriangleMeshComposite(InputIterator begin, InputIterator end):
		children_(begin, end)
	{
	}
	void add(const TTriangleMeshRef& child);
	void add(const TChildren& children);
	const TChildren& children() const;

private:

	typedef TriangleMesh::TMesh TMesh;
	typedef TriangleMesh::TVertices TVertices;
	typedef TriangleMesh::TNormals TNormals;
	typedef TriangleMesh::TUvs TUvs;
	typedef TriangleMesh::TIndexTriangles TIndexTriangles;
	typedef std::vector< std::pair<TriangleMesh*, size_t> > TBackLinks;

	void doAccept(lass::util::VisitorBase& visitor) override;
	void doPreProcess(const TimePeriod& period) override;

	void doIntersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const override;
	bool doIsIntersecting(const Sample& sample, const BoundedRay& ray) const override;
	void doLocalContext(const Sample& sample, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const override;
	bool doContains(const Sample& sample, const TPoint3D& point) const override;
	const TAabb3D doBoundingBox() const override;
	const TSphere3D doBoundingSphere() const override;
	TScalar doArea() const override;
	TScalar doArea(const TVector3D& normal) const override;

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

	bool triangleFilter(TMesh::TTriangleIterator triangle, TScalar t, const Sample& sample, const BoundedRay& ray) const;

	TChildren children_;
	TMesh mesh_;
	TBackLinks backLinks_;
	bool hasAlphaMasks_ = false;
};



}

}

#endif

// EOF
