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

#include "scenery_common.h"
#include "triangle_mesh_composite.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_DOC(TriangleMeshComposite, "a composite of triangle meshes")
PY_CLASS_CONSTRUCTOR_0(TriangleMeshComposite)
PY_CLASS_CONSTRUCTOR_1(TriangleMeshComposite, const TriangleMeshComposite::TChildren&)
PY_CLASS_METHOD_QUALIFIED_1(TriangleMeshComposite, add, void, const TriangleMeshComposite::TTriangleMeshRef&)
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



void TriangleMeshComposite::add(const TTriangleMeshRef& child)
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



void TriangleMeshComposite::doPreProcess(const TimePeriod& period)
{
	if (!mesh_.triangles().empty())
	{
		return;
	}

	typedef TIndexTriangles::value_type TIndexTriangle;

	TVertices verts;
	TNormals normals;
	TUvs uvs;
	TIndexTriangles triangles;
	TBackLinks backLinks;

	size_t numVerts = 0;
	size_t numNormals = 0;
	size_t numUvs = 0;
	size_t numTriangles = 0;
	hasAlphaMasks_ = false;
	for (const auto& child : children_)
	{
		numVerts += child->vertices().size();
		numNormals += child->normals().size();
		numUvs += child->uvs().size();
		numTriangles += child->triangles().size();
		hasAlphaMasks_ |= static_cast<bool>(child->alphaMask());
	}
	verts.reserve(numVerts);
	normals.reserve(numNormals);
	uvs.reserve(numUvs);
	triangles.reserve(numTriangles);
	backLinks.reserve(numTriangles);

	const TChildren::const_iterator end = children_.end();
	for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
	{
		TriangleMesh& child = **i;
		child.preProcess(period);

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

	LASS_COUT << "Combined " << children_.size() << " triangle meshes into one with " << triangles.size() << " triangles." << std::endl;

	TMesh mesh(std::move(verts), std::move(normals), std::move(uvs), triangles);

	mesh_.swap(mesh);
	backLinks_.swap(backLinks);
}



void TriangleMeshComposite::doIntersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const
{
	// also modify TriangleMesh::doIntersects, if you change this

	auto filter = hasAlphaMasks_
		? [this, &sample, &ray](TMesh::TTriangleIterator triangle, TScalar t) { return this->triangleFilter(triangle, t, sample, ray); }
		: TMesh::TFilter();

	Intersection intersection;
	TScalar t;
	TMesh::TTriangleIterator triangle;
	const prim::Result hit = mesh_.intersectFilter(ray.unboundedRay(), triangle, t, ray.nearLimit(), filter);
	if (hit == prim::rOne && ray.inRange(t))
	{
		const TScalar d = dot(ray.direction(), triangle->geometricNormal());
		const SolidEvent se = (d < TNumTraits::zero)
			? seEntering
			: (d > TNumTraits::zero)
			? seLeaving
			: seNoEvent;
		const size_t k = static_cast<size_t>(std::distance(mesh_.triangles().begin(), triangle));
		TriangleMesh* const child = backLinks_[k].first;
		const size_t k2 = k - backLinks_[k].second;
		intersection = Intersection(child, t, se, k2);
		intersection.push(this);

		result.swap(intersection);
	}
	else
	{
		result = Intersection::empty();
	}
}



bool TriangleMeshComposite::doIsIntersecting(const Sample& sample, const BoundedRay& ray) const
{
	// also modify TriangleMesh::doIntersects, if you change this

	auto filter = hasAlphaMasks_
		? [this, &sample, &ray](TMesh::TTriangleIterator triangle, TScalar t) { return this->triangleFilter(triangle, t, sample, ray); }
		: TMesh::TFilter();

	return mesh_.intersectsFilter(ray.unboundedRay(), ray.nearLimit(), ray.farLimit(), filter);
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



const TSphere3D TriangleMeshComposite::doBoundingSphere() const
{
	if (!mesh_.aabb().isEmpty())
	{
		return prim::boundingSphere(mesh_);
	}

	// backup plan: compute from children.
	const TChildren::const_iterator end = children_.end();
	TMesh::TVertices vertices;
	for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
	{
		const auto &verts = (*i)->vertices();
		vertices.insert(vertices.end(), verts.begin(), verts.end());
	}
	return prim::boundingSphere(vertices);
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



bool TriangleMeshComposite::triangleFilter(TMesh::TTriangleIterator triangle, TScalar t, const Sample& sample, const BoundedRay& ray) const
{
	const size_t k = static_cast<size_t>(std::distance(mesh_.triangles().begin(), triangle));
	TriangleMesh* const child = backLinks_[k].first;
	if (!child->alphaMask())
	{
		return true;
	}

	const TScalar d = dot(ray.direction(), triangle->geometricNormal());
	const SolidEvent se = (d < TNumTraits::zero)
		? seEntering
		: (d > TNumTraits::zero)
		? seLeaving
		: seNoEvent;

	const size_t k2 = k - backLinks_[k].second;
	Intersection intersection(child, t, se, k2);
	intersection.push(this);

	const IntersectionContext context(*this, sample, ray, intersection, 0);

	return child->alphaMask()->scalarLookUp(sample, context) >= child->alphaThreshold();
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
