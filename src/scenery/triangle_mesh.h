/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2005  Bram de Greve
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  http://liar.sourceforge.net
 */

/** @class liar::scenery::TriangleMesh
 *  @brief a simple triangle mesh
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_TRIANGLE_MESH_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_TRIANGLE_MESH_H

#include "scenery_common.h"
#include "../kernel/scene_object.h"

#include <lass/prim/triangle_mesh_3d.h>
#include <lass/spat/aabb_tree.h>
#include <lass/spat/aabp_tree.h>

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL TriangleMesh: public SceneObject
{
    PY_HEADER(SceneObject)
public:

	typedef prim::TriangleMesh3D<TScalar, spat::AabbTree> TMesh;
	typedef std::vector<TMesh::TPoint> TVertices;
	typedef std::vector<TMesh::TVector> TNormals;
	typedef std::vector<TMesh::TUv> TUvs;
	typedef std::vector<TMesh::TIndexTriangle> TIndexTriangles;

	TriangleMesh(const TVertices& iVertices, const TNormals& iNormals,
		const TUvs& iUvs, const TIndexTriangles& iTriangles);

private:

    LASS_UTIL_ACCEPT_VISITOR
    
	void doIntersect(const Sample& iSample, const BoundedRay& iRay, Intersection& oResult) const;
	const bool doIsIntersecting(const Sample& iSample, const BoundedRay& iRay) const;
	void doLocalContext(const Sample& iSample, const BoundedRay& iRay,
		const Intersection& iIntersection, IntersectionContext& oResult) const;
	const bool doContains(const Sample& iSample, const TPoint3D& iPoint) const;
	const TAabb3D doBoundingBox() const;

    TMesh mesh_;
};



}

}

#endif

// EOF
