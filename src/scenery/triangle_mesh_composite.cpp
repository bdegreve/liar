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
#include "triangle_mesh_composite.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_DOC(TriangleMeshComposite, "a composite of triangle meshes")
PY_CLASS_CONSTRUCTOR_0(TriangleMeshComposite)
PY_CLASS_CONSTRUCTOR_1(TriangleMeshComposite, const TriangleMeshComposite::TChildren&)
PY_CLASS_METHOD_QUALIFIED_1(TriangleMeshComposite, add, void, const TriangleMeshComposite::TTriangleMeshPtr&)
PY_CLASS_METHOD_QUALIFIED_1(TriangleMeshComposite, add, void, const TriangleMeshComposite::TChildren&)
PY_CLASS_METHOD(TriangleMeshComposite, children)

// --- public --------------------------------------------------------------------------------------

TriangleMeshComposite::TriangleMeshComposite()
{
}



TriangleMeshComposite::TriangleMeshComposite(const TChildren& children):
	children_(children)
{
}



void TriangleMeshComposite::add(const TTriangleMeshPtr& child)
{
	children_.push_back(child);
}



void TriangleMeshComposite::add(const TChildren& children)
{
	children_.insert(children_.end(), children.begin(), children.end());
}



const TriangleMeshComposite::TChildren& TriangleMeshComposite::children() const
{
	return children_;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void TriangleMeshComposite::doAccept(util::VisitorBase& visitor)
{
	preAccept(visitor, *this);
	const TChildren::const_iterator end = children_.end();
	for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
	{
		(*i)->accept(visitor);
	}
	postAccept(visitor, *this);
}



void TriangleMeshComposite::doPreProcess(const TSceneObjectPtr& scene, const TimePeriod& period)
{
	typedef TIndexTriangles::value_type TIndexTriangle;

	TVertices verts;
	TNormals normals;
	TUvs uvs;
	TIndexTriangles triangles;
	TBackLinks backLinks;

	const TChildren::const_iterator end = children_.end();
	for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
	{
		TriangleMesh& child = **i;
		child.preProcess(scene, period);

		const TVertices& childVerts = child.vertices();
		const TNormals& childNormals = child.normals();
		const TUvs& childUvs = child.uvs();
		const TIndexTriangles& childTriangles = child.triangles();
		
		const size_t vertOffset = verts.size();
		const size_t normalOffset = normals.size();
		const size_t uvOffset = uvs.size();
		const size_t triangleOffset = triangles.size();

		verts.insert(verts.end(), childVerts.begin(), childVerts.end());
		normals.insert(normals.end(), childNormals.begin(), childNormals.end());
		uvs.insert(uvs.end(), childUvs.begin(), childUvs.end());
		for (TIndexTriangles::const_iterator j = childTriangles.begin(); j != childTriangles.end(); ++j)
		{
			TIndexTriangle triangle = *j; // copy it.
			for (size_t k = 0; k < 3; ++k)
			{
				if (triangle.vertices[k] != TIndexTriangle::null())
				{
					LASS_ASSERT(triangle.vertices[k] < childVerts.size());
					triangle.vertices[k] += vertOffset;
					LASS_ASSERT(triangle.vertices[k] < verts.size());
				}
				if (triangle.normals[k] != TIndexTriangle::null())
				{
					LASS_ASSERT(triangle.normals[k] < childNormals.size());
					triangle.normals[k] += normalOffset;
					LASS_ASSERT(triangle.normals[k] < normals.size());
				}
				if (triangle.uvs[k] != TIndexTriangle::null())
				{
					LASS_ASSERT(triangle.uvs[k] < childUvs.size());
					triangle.uvs[k] += uvOffset;
					LASS_ASSERT(triangle.uvs[k] < uvs.size());
				}
			}
			triangles.push_back(triangle);
			backLinks.push_back( std::make_pair(&child, triangleOffset) );
		}
	}

	TMesh mesh(verts, normals, uvs, triangles);

	mesh_.swap(mesh);
	backLinks_.swap(backLinks);
}



void TriangleMeshComposite::doIntersect(const Sample&, const BoundedRay& ray, Intersection& result) const
{
	TScalar t;
	TMesh::TTriangleIterator triangle;
	const prim::Result hit = mesh_.intersect(ray.unboundedRay(), triangle, t, ray.nearLimit());
	if (hit == prim::rOne && ray.inRange(t))
	{
		const size_t k = static_cast<size_t>(std::distance(mesh_.triangles().begin(), triangle));
		TriangleMesh* const child = backLinks_[k].first;
		const size_t k2 = k - backLinks_[k].second;
		result = Intersection(child, t, seNoEvent, k2);
		result.push(this);
	}
	else
	{
		result = Intersection::empty();
	}
}



bool TriangleMeshComposite::doIsIntersecting(const Sample&, const BoundedRay& ray) const
{
	return mesh_.intersects(ray.unboundedRay(), ray.nearLimit(), ray.farLimit());
}



void TriangleMeshComposite::doLocalContext(
		const Sample& sample, const BoundedRay& ray, const Intersection& intersection,
		IntersectionContext& result) const
{
	IntersectionDescendor descend(intersection);
	intersection.object()->localContext(sample, ray, intersection, result);
}



bool TriangleMeshComposite::doContains(const Sample&, const TPoint3D&) const
{
	return false;
}



const TAabb3D TriangleMeshComposite::doBoundingBox() const
{
	TAabb3D result = mesh_.aabb();
	if (!result.isEmpty())
	{
		return result;
	}

	// backup plan: if mesh doesn't have aabb yet, compute from children.
	const TChildren::const_iterator end = children_.end();
	for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
	{
		const SceneObject* child = i->get();
		LASS_ASSERT(child);
		result += child->boundingBox();
	}
	return result;
}



TScalar TriangleMeshComposite::doArea() const
{
	return mesh_.area();
}



TScalar TriangleMeshComposite::doArea(const TVector3D&) const
{
	LASS_THROW("not implemented yet");
	return TNumTraits::qNaN;
}



const TPyObjectPtr TriangleMeshComposite::doGetState() const
{
	return python::makeTuple(children_);
}



void TriangleMeshComposite::doSetState(const TPyObjectPtr& state)
{
	LASS_ENFORCE(python::decodeTuple(state, children_));
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
