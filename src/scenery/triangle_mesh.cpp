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

#include "scenery_common.h"
#include "triangle_mesh.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_DOC(TriangleMesh, "a simple triangle mesh")
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
PY_CLASS_METHOD(TriangleMesh, flatFaces)
PY_CLASS_METHOD(TriangleMesh, loopSubdivision) 
PY_CLASS_METHOD(TriangleMesh, autoSew) 
PY_CLASS_METHOD(TriangleMesh, autoCrease) 

PY_CLASS_METHOD(TriangleMesh, vertices)
PY_CLASS_METHOD(TriangleMesh, normals)
PY_CLASS_METHOD(TriangleMesh, uvs)
PY_CLASS_METHOD(TriangleMesh, triangles)

// --- public --------------------------------------------------------------------------------------

TriangleMesh::TriangleMesh(const TVertices& vertices, const TNormals& normals, const TUvs& uvs, const TIndexTriangles& triangles):
	mesh_(vertices, normals, uvs, triangles)
{
}



void TriangleMesh::smoothNormals()
{
	mesh_.smoothNormals();
}



void TriangleMesh::flatFaces()
{
	mesh_.flatFaces();
}



void TriangleMesh::loopSubdivision(unsigned level)
{
	mesh_.loopSubdivision(level);
}



void TriangleMesh::autoSew()
{
	mesh_.autoSew();
}



void TriangleMesh::autoCrease(unsigned level, TScalar maxAngleInRadians)
{
	mesh_.autoCrease(level, maxAngleInRadians);
}



const TriangleMesh::TVertices& TriangleMesh::vertices() const
{
	return mesh_.vertices();
}



const TriangleMesh::TNormals& TriangleMesh::normals() const
{
	return mesh_.normals();
}



const TriangleMesh::TUvs& TriangleMesh::uvs() const
{
	return mesh_.uvs();
}



const TriangleMesh::TIndexTriangles TriangleMesh::triangles() const
{
	TIndexTriangles triangles;
	mesh_.indexTriangles(std::back_inserter(triangles));
	return triangles;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void TriangleMesh::doIntersect(const Sample&, const BoundedRay& ray, Intersection& result) const
{
	TScalar t;
	TMesh::TTriangleIterator triangle;
	const prim::Result hit = mesh_.intersect(ray.unboundedRay(), triangle, t, ray.nearLimit());
	if (hit == prim::rOne && ray.inRange(t))
	{
		result = Intersection(this, t, seNoEvent, triangle - mesh_.triangles().begin());
	}
	else
	{
		result = Intersection::empty();
	}
}



bool TriangleMesh::doIsIntersecting(const Sample&, const BoundedRay& ray) const
{
	return mesh_.intersects(ray.unboundedRay(), ray.nearLimit(), ray.farLimit());
}



void TriangleMesh::doLocalContext(const Sample&, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const
{
	result.setBounds(this->boundingBox());

	const TScalar t = intersection.t();
	const TPoint3D point = ray.point(t);
	result.setT(t);
	result.setPoint(point);

	LASS_ASSERT(intersection.specialField() < mesh_.triangles().size());
	const TMesh::TTriangle& triangle = mesh_.triangles()[intersection.specialField()];

	TMesh::TIntersectionContext context;
	TScalar t2;	
	const prim::Result LASS_UNUSED(r) = triangle.intersect(ray.unboundedRay(), t2, ray.nearLimit(), &context);
#pragma LASS_FIXME("why can t != t2?")
	LASS_ASSERT(r == prim::rOne /*&& t == t2*/);

	result.setGeometricNormal(context.geometricNormal);
	result.setNormal(context.normal);
	result.setDPoint_dU(context.dPoint_dU);
	result.setDPoint_dV(context.dPoint_dV);
	result.setDNormal_dU(context.dNormal_dU);
	result.setDNormal_dV(context.dNormal_dV);
	result.setUv(context.uv);
}



bool TriangleMesh::doContains(const Sample&, const TPoint3D&) const
{
	return false;
}



const TAabb3D TriangleMesh::doBoundingBox() const
{
	return mesh_.aabb();
}



TScalar TriangleMesh::doArea() const
{
	return mesh_.area();
}



TScalar TriangleMesh::doArea(const TVector3D&) const
{
	LASS_THROW("not implemented yet");
	return TNumTraits::qNaN;
}



const TPyObjectPtr TriangleMesh::doGetState() const
{
	TIndexTriangles triangles;
	mesh_.indexTriangles(std::back_inserter(triangles));
	return python::makeTuple(mesh_.vertices(), mesh_.normals(), mesh_.uvs(), triangles);
}



void TriangleMesh::doSetState(const TPyObjectPtr& state)
{
	TMesh::TVertices vertices;
	TMesh::TNormals normals;
	TMesh::TUvs uvs;
	TIndexTriangles triangles;
	LASS_ENFORCE(python::decodeTuple(state, vertices, normals, uvs, triangles));
	mesh_ = TMesh(vertices, normals, uvs, triangles);
}



// --- free ----------------------------------------------------------------------------------------



// --- python --------------------------------------------------------------------------------------

}

}

// EOF
