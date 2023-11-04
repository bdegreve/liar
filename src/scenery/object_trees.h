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

/** @class liar::scenery::AabbTree
 *  @brief compound of some child scenery in an AABB tree.
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_OBJECT_TREE_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_OBJECT_TREE_H

#include "scenery_common.h"
#include "list.h"
#include "../kernel/scene_object.h"
#include <lass/spat/aabb_tree.h>
#include <lass/spat/aabp_tree.h>
#include <lass/spat/quad_tree.h>

namespace liar
{
namespace scenery
{

template
<
	template <typename, typename> class TreeTypedef
>
class ObjectTree: public SceneObject
{
public:

	typedef std::vector<TSceneObjectPtr> TChildren;

	void add(const TSceneObjectPtr& child)
	{
		if (child->area() == TNumTraits::infinity)
		{
			bigChildren_.add(child);
		}
		else
		{
			smallChildren_.push_back(child);
		}
		allChildren_.push_back(child);
	}
	void add(const TChildren& children)
	{
		for (TChildren::const_iterator child = children.begin(); child != children.end(); ++child)
		{
			this->add(*child);
		}
	}
	const TChildren& children() const
	{
		return allChildren_;
	}

protected:

	ObjectTree() {}
	ObjectTree(const TChildren& children)
	{
		this->add(children);
	}

private:

	void doAccept(lass::util::VisitorBase& visitor)
	{
		preAccept(visitor, *this);
		const TChildren::const_iterator end = allChildren_.end();
		for (TChildren::const_iterator i = allChildren_.begin(); i != end; ++i)
		{
			(*i)->accept(visitor);
		}
		postAccept(visitor, *this);
	}

	void doPreProcess(const TSceneObjectPtr& scene, const TimePeriod& period)
	{
		const TChildren::const_iterator end = allChildren_.end();
		for (TChildren::const_iterator i = allChildren_.begin(); i != end; ++i)
		{
			(*i)->preProcess(scene, period);
		}
		tree_.reset(smallChildren_.begin(), smallChildren_.end());
	}

	void doIntersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const
	{
		Intersection treeResult;
		Info info;
		info.sample = &sample;
		info.intersectionResult = &treeResult;
		TScalar t;
		if (tree_.intersect(ray, t, ray.nearLimit(), &info) != smallChildren_.end() && t < ray.farLimit())
		{
			LASS_ASSERT(treeResult);
			treeResult.push(this);
		}
		else
		{
			LASS_ASSERT(!treeResult);
		}

		Intersection bigResult;
		bigChildren_.intersect(sample, ray, bigResult);
		if (bigResult && (!treeResult || bigResult.t() < treeResult.t()))
		{
			bigResult.push(this);
			treeResult.swap(bigResult);
		}
		result.swap(treeResult);
	}

	bool doIsIntersecting(const Sample& sample, const BoundedRay& ray) const
	{
		Info info;
		info.sample = &sample;
		info.intersectionResult = 0;
		return bigChildren_.isIntersecting(sample, ray) || tree_.intersects(ray, ray.nearLimit(), ray.farLimit(), &info);
	}

	void doLocalContext(
			const Sample& sample, const BoundedRay& ray,
			const Intersection& intersection, IntersectionContext& result) const
	{
		IntersectionDescendor descend(intersection);
		intersection.object()->localContext(sample, ray, intersection, result);
	}

	bool doContains(const Sample& sample, const TPoint3D& point) const
	{
		Info info;
		info.sample = &sample;
		info.intersectionResult = 0;
		return tree_.contains(point, &info) || bigChildren_.contains(sample, point);
	}

	const TAabb3D doBoundingBox() const
	{
		return tree_.aabb() + bigChildren_.boundingBox();
	}

	TScalar doArea() const
	{
		TScalar result = 0;
		for (TChildren::const_iterator i = allChildren_.begin(); i != allChildren_.end(); ++i)
		{
			result += (*i)->area();
		}
		return result;
	}

	TScalar doArea(const TVector3D& normal) const
	{
		TScalar result = 0;
		for (TChildren::const_iterator i = allChildren_.begin(); i != allChildren_.end(); ++i)
		{
			result += (*i)->area(normal);
		}
		return result;
	}

	const TPyObjectPtr doGetState() const
	{
		return python::makeTuple(allChildren_);
	}

	void doSetState(const TPyObjectPtr& state)
	{
		LASS_ENFORCE(python::decodeTuple(state, allChildren_));
	}

	struct Info
	{
		const Sample* sample;
		Intersection* intersectionResult;
	};

	struct ObjectTraits
	{
		typedef TChildren::value_type TObject;
		typedef TAabb3D TAabb;
		typedef BoundedRay TRay;

		typedef TChildren::const_iterator TObjectIterator;
		typedef TChildren::const_reference TObjectReference;

		typedef TPoint3D TPoint;
		typedef TVector3D TVector;
		typedef TPoint::TValue TValue;
		typedef TPoint::TParam TParam;
		typedef TPoint::TReference TReference;
		typedef TPoint::TConstReference TConstReference;

		typedef Info TInfo;

		enum { dimension = TPoint::dimension };

		static const TAabb objectAabb(TObjectIterator object)
		{
			return (*object)->boundingBox();
		}

		static bool objectContains(TObjectIterator object, const TPoint& point, const TInfo* info)
		{
			LASS_ASSERT(info && info->sample);
			return (*object)->contains(*info->sample, point);
		}

		static bool objectIntersect(TObjectIterator object, const TRay& ray, TReference t, TParam LASS_UNUSED(minT), const TInfo* info)
		{
			LASS_ASSERT(info && info->sample && info->intersectionResult);
			LASS_ASSERT(ray.nearLimit() == minT);
			Intersection temp;
			(*object)->intersect(*info->sample, ray, temp);
			if (temp)
			{
				LASS_ASSERT(temp.t() > minT);
				t = temp.t();
				if (info->intersectionResult->isEmpty() ||
					temp.t() < info->intersectionResult->t())
				{
					info->intersectionResult->swap(temp);
				}
				return true;
			}
			return false;
		}

		static bool objectIntersects(TObjectIterator object, const TRay& ray, TParam LASS_UNUSED(minT), TParam LASS_UNUSED(maxT), const TInfo* info)
		{
			LASS_ASSERT(info && info->sample);
			LASS_ASSERT(ray.nearLimit() == minT && ray.farLimit() == maxT);
			return (*object)->isIntersecting(*info->sample, ray);
		}

		static bool objectIntersects(TObjectIterator it, const TAabb& aabb, const TInfo*)
		{
			return objectAabb(it).intersects(aabb);
		}

		static TAabb aabbEmpty()
		{
			return TAabb();
		}

		static const TAabb aabbMake(const TPoint& min, const TPoint& max)
		{
			return TAabb(min, max);
		}

		static bool aabbContains(const TAabb& aabb, const TPoint& point)
		{
			return aabb.contains(point);
		}

		static bool aabbContains(const TAabb& aabb, const TAabb& other)
		{
			return aabb.contains(other);
		}

		static bool aabbIntersect(const TAabb& aabb, const TRay& ray, TReference t, const TParam tMin)
		{
			TScalar temp;
			prim::Result hit = prim::intersect(aabb, ray.unboundedRay(), temp, tMin);
			if (hit == prim::rOne)
			{
				t = temp;
				return true;
			}
			return false;
		}

		static bool aabbIntersect(const TAabb& aabb, const TRay& ray, const TVector3D& invDir, TReference t, const TParam tMin)
		{
			TScalar temp;
			prim::Result hit = prim::intersect(aabb, ray.unboundedRay(), invDir, temp, tMin);
			if (hit == prim::rOne)
			{
				t = temp;
				return true;
			}
			return false;
		}

		static const TAabb aabbJoin(const TAabb& a, const TAabb& b) { return a + b; }
		static const TPoint aabbMin(const TAabb& aabb) { return aabb.min(); }
		static const TPoint aabbMax(const TAabb& aabb) { return aabb.max(); }
		static const TPoint raySupport(const TRay& ray) { return ray.support();	}
		static const TVector rayDirection(const TRay& ray) {	return ray.direction(); }
		static const TPoint rayPoint(const TRay& ray, TParam t) { return ray.point(t);	}
		static TValue coord(const TPoint& point, size_t axis) { return point[axis]; }
		static TValue coord(const TVector& vector, size_t axis) { return vector[axis]; }
		static void coord(TPoint& point, size_t axis, TValue x) { point[axis] = x; }
		static void coord(TVector& vector, size_t axis, TValue x) { vector[axis] = x; }
		static const TVector vectorReciprocal(const TVector& vector) { return vector.reciprocal();	}
	};

	typedef typename TreeTypedef<TChildren::value_type, ObjectTraits>::Type TTree;
	//typedef spat::AabbTree<TChildren::value_type, ObjectTraits> TTree;

	TChildren allChildren_;
	TChildren smallChildren_;
	List bigChildren_;
	TTree tree_;
};

#pragma LASS_TODO("There's no proper way yet to export templated classes to python, think of a way [Bramz]")

#define LIAR_SCENERY_DEFINE_OBJECT_TREE(i_name, t_ObjectTreeTemplate)\
	template <typename ObjectType, typename ObjectTraits>\
	struct LASS_CONCATENATE( ObjectTreeTypedef_, i_name )\
	{\
		typedef t_ObjectTreeTemplate<ObjectType, ObjectTraits> Type;\
	};\
	class LIAR_SCENERY_DLL i_name:\
		public ObjectTree< LASS_CONCATENATE( ObjectTreeTypedef_, i_name ) >\
	{\
		PY_HEADER(SceneObject)\
	public:\
		typedef ObjectTree< LASS_CONCATENATE( ObjectTreeTypedef_, i_name ) > TObjectTree;\
		i_name(): TObjectTree() {}\
		i_name(const TChildren& children): TObjectTree(children) {}\
		template <typename InputIterator> i_name(InputIterator begin, InputIterator end): TObjectTree(begin, end) {}\
	};\
	/**/

LIAR_SCENERY_DEFINE_OBJECT_TREE(AabbTree, ::lass::spat::AabbTree);
LIAR_SCENERY_DEFINE_OBJECT_TREE(AabpTree, ::lass::spat::AabpTree);
LIAR_SCENERY_DEFINE_OBJECT_TREE(OctTree, ::lass::spat::QuadTree);

}

}

#endif

// EOF
