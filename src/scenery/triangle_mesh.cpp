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

#include "scenery_common.h"
#include "triangle_mesh.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(TriangleMesh)
PY_CLASS_CONSTRUCTOR_4(TriangleMesh, 
					   const TriangleMesh::TVertices&, 
					   const TriangleMesh::TNormals&, 
					   const TriangleMesh::TUvs&, 
 					   const TriangleMesh::TIndexTriangles&)
PY_CLASS_METHOD_DOC(TriangleMesh, smoothNormals, 
	"smoothNormals(maxAngleInRadians)\n"
	"generate smooth vertex normals.  maxAngleInRadians is the maximum angle (in radians) that is "
	"allowed between the smooth vertex normal and face normal of the triangle.  If the angle "
	"between both normals is less than this maximum, the vertex normal is used to create the "
	"illusion of a smooth edge.  If not, the face normal is preserved to keep hard edges.")

// --- public --------------------------------------------------------------------------------------

TriangleMesh::TriangleMesh(const TVertices& iVertices, const TNormals& iNormals,
		const TUvs& iUvs, const TIndexTriangles& iTriangles):
	SceneObject(&Type),
    mesh_(iVertices, iNormals, iUvs, iTriangles)
{
}



void TriangleMesh::smoothNormals(TScalar iMaxAngleInRadians)
{
	mesh_.smoothNormals(iMaxAngleInRadians);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void TriangleMesh::doIntersect(const Sample& iSample, const BoundedRay& iRay, 
						 Intersection& oResult) const
{
    TScalar t;
	TMesh::TTriangleIterator triangle;
	const prim::Result hit = mesh_.intersect(iRay.unboundedRay(), triangle, t, iRay.nearLimit());
	if (hit == prim::rOne && iRay.inRange(t))
	{
		oResult = Intersection(this, t, seNoEvent, triangle - mesh_.triangles().begin());
	}
	else
	{
		oResult = Intersection::empty();
	}
}



const bool TriangleMesh::doIsIntersecting(const Sample& iSample, 
									const BoundedRay& iRay) const
{
	return mesh_.intersects(iRay.unboundedRay(), iRay.nearLimit(), iRay.farLimit());
}



void TriangleMesh::doLocalContext(const Sample& iSample, const BoundedRay& iRay,                            
								  const Intersection& iIntersection, 
								  IntersectionContext& oResult) const
{
	const TScalar t = iIntersection.t();
	const TPoint3D point = iRay.point(t);
    oResult.setT(t);
    oResult.setPoint(point);

	LASS_ASSERT(iIntersection.specialField() < mesh_.triangles().size());
	const TMesh::TTriangle& triangle = mesh_.triangles()[iIntersection.specialField()];

	TMesh::TIntersectionContext context;
	TScalar t2;	
	const prim::Result result = triangle.intersect(iRay.unboundedRay(), t2, iRay.nearLimit(), &context);
	LASS_ASSERT(result == prim::rOne && t == t2);

	oResult.setGeometricNormal(context.geometricNormal);
	oResult.setNormal(context.normal);
	oResult.setDPoint_dU(context.dPoint_dU);
	oResult.setDPoint_dV(context.dPoint_dV);
	oResult.setDNormal_dU(context.dNormal_dU);
	oResult.setDNormal_dV(context.dNormal_dV);
	oResult.setUv(context.uv);
}



const bool TriangleMesh::doContains(const Sample& iSample, const TPoint3D& iPoint) const
{
	return false;
}



const TAabb3D TriangleMesh::doBoundingBox() const
{
    return mesh_.aabb();
}



const TScalar TriangleMesh::doArea() const
{
    return mesh_.area();
}



const TPyObjectPtr TriangleMesh::doGetState() const
{
	TIndexTriangles triangles;
	mesh_.indexTriangles(std::back_inserter(triangles));
	return python::makeTuple(mesh_.vertices(), mesh_.normals(), mesh_.uvs(), triangles);
}



void TriangleMesh::doSetState(const TPyObjectPtr& iState)
{
	TMesh::TVertices vertices;
	TMesh::TNormals normals;
	TMesh::TUvs uvs;
	TIndexTriangles triangles;
	LASS_ENFORCE(python::decodeTuple(iState, vertices, normals, uvs, triangles));
	mesh_ = TMesh(vertices, normals, uvs, triangles);
}



// --- free ----------------------------------------------------------------------------------------



// --- python --------------------------------------------------------------------------------------

}

}

// EOF
