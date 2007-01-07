/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2006  Bram de Greve
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

/** @class liar::scenery::AabbTree
 *  @brief compound of some child scenery in an AABB tree.
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_AABB_TREE_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_AABB_TREE_H

#include "scenery_common.h"
#include "../kernel/scene_object.h"
#include <lass/spat/aabb_tree.h>
#include <lass/spat/aabp_tree.h>

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL AabbTree: public SceneObject
{
	PY_HEADER(SceneObject)
public:

	typedef std::vector<TSceneObjectPtr> TChildren;

	AabbTree();
	AabbTree(const TChildren& children); // for python
	template <typename InputIterator> AabbTree(InputIterator begin, InputIterator end):
	{
		add(begin, end);
	}
	
	void add(const TSceneObjectPtr& child);
	void add(const TChildren& children); // for python
	template <typename InputIterator> void add(InputIterator first, InputIterator last)
	{
		while (first != last)
		{
			this->add(*first++);
		}
	}

private:

	void doAccept(lass::util::VisitorBase& visitor);

	void doPreProcess(const TSceneObjectPtr& scene, const TimePeriod& period);
	void doIntersect(const Sample& sample, const BoundedRay& ray, 
		Intersection& result) const;
	const bool doIsIntersecting(const Sample& sample, const BoundedRay& ray) const;
	void doLocalContext(const Sample& sample, const BoundedRay& ray,
		const Intersection& intersection, IntersectionContext& result) const;
	const bool doContains(const Sample& sample, const TPoint3D& point) const;
	const TAabb3D doBoundingBox() const;
	const TScalar doArea() const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

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

		static const TAabb aabb(TObjectIterator object)
		{
			return (*object)->boundingBox();
		}

		static const bool contains(TObjectIterator object, const TPoint& point, 
			const TInfo* info)
		{
			LASS_ASSERT(info && info->sample);
			return (*object)->contains(*info->sample, point);
		}

		static const bool intersect(TObjectIterator object, const TRay& ray, 
			TReference t, TParam minT, const TInfo* info)
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

		static const bool intersects(TObjectIterator object, const TRay& ray, 
			TParam minT, TParam maxT, const TInfo* info)
		{
			LASS_ASSERT(info && info->sample);
			LASS_ASSERT(ray.nearLimit() == minT && ray.farLimit() == maxT);
			return (*object)->isIntersecting(*info->sample, ray);
		}

		static TAabb emptyAabb()
		{
			return TAabb();
		}

		static const bool contains(const TAabb& aabb, const TPoint& point) 
		{ 
			return aabb.contains(point); 
		}

		static const bool intersect(const TAabb& aabb, const TRay& ray, 
			TReference t, const TParam minT)
		{
			TScalar temp;
			prim::Result hit = prim::intersect(aabb, ray.unboundedRay(), temp, minT);
			if (hit == prim::rOne)
			{
				t = temp;
				return true;
			}
			return false;
		}

		static const TAabb join(const TAabb& a, const TAabb& b) { return a + b; }
		static const TPoint min(const TAabb& aabb) { return aabb.min(); }
		static const TPoint max(const TAabb& aabb) { return aabb.max(); }
		static const TPoint support(const TRay& ray) { return ray.support();	}
		static const TVector direction(const TRay& ray) {	return ray.direction(); }
		static const TValue coordinate(const TPoint& point, size_t axis) { return point[axis]; }
		static const TValue component(const TVector& vector, size_t axis) { return vector[axis]; }
		static const TVector reciprocal(const TVector& vector) { return vector.reciprocal();	}
	};

	typedef spat::AabpTree<TChildren::value_type, ObjectTraits> TTree;

	TChildren children_;
	TTree tree_;
};



}

}

#endif

// EOF
