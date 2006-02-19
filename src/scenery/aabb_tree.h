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

/** @class liar::scenery::AabbTree
 *  @brief compound of some child scenery in an AABB tree.
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_AABB_TREE_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_AABB_TREE_H

#include "scenery_common.h"
#include "../kernel/scene_object.h"
#include <lass/spat/aabb_tree.h>

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
    AabbTree(const TChildren& iChildren); // for python

    template <typename InputIterator> AabbTree(InputIterator iBegin, InputIterator iEnd):
        SceneComposite(&Type)
    {
        add(iBegin, iEnd);
    }

    void add(const TSceneObjectPtr& iChild);
	void add(const TChildren& iChildren); // for python

	template <typename InputIterator> void add(InputIterator iBegin, InputIterator iEnd)
	{
        while (iBegin != iEnd)
		{
			this->add(*iBegin++);
		}
	}

private:

    void doAccept(lass::util::VisitorBase& ioVisitor);

	void doPreProcess(const TimePeriod& iPeriod);
	void doIntersect(const Sample& iSample, const BoundedRay& iRay, 
		Intersection& oResult) const;
	const bool doIsIntersecting(const Sample& iSample, const BoundedRay& iRay) const;
	void doLocalContext(const Sample& iSample, const BoundedRay& iRay,
		const Intersection& iIntersection, IntersectionContext& oResult) const;
	const bool doContains(const Sample& iSample, const TPoint3D& iPoint) const;
	const TAabb3D doBoundingBox() const;
	const TScalar doArea() const;

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

		static const TAabb aabb(TObjectIterator iObject)
		{
			return (*iObject)->boundingBox();
		}

		static const bool contains(TObjectIterator iObject, const TPoint& iPoint, 
			const TInfo* iInfo)
		{
			LASS_ASSERT(iInfo && iInfo->sample);
			return (*iObject)->contains(*iInfo->sample, iPoint);
		}

		static const bool intersect(TObjectIterator iObject, const TRay& iRay, 
			TReference oT, TParam iMinT, const TInfo* iInfo)
		{
			LASS_ASSERT(iInfo && iInfo->sample && iInfo->intersectionResult);
			Intersection temp;
			(*iObject)->intersect(*iInfo->sample, iRay, temp);
			if (temp && temp.t() > iMinT)
			{
				oT = temp.t();
				if (iInfo->intersectionResult->isEmpty() || 
					temp.t() < iInfo->intersectionResult->t())
				{
					iInfo->intersectionResult->swap(temp);
				}
				return true;
			}
			return false;
		}

		static const bool intersects(TObjectIterator iObject, const TRay& iRay, 
			TParam iMinT, TParam iMaxT, const TInfo* iInfo)
		{
			LASS_ASSERT(iInfo && iInfo->sample);
			return (*iObject)->isIntersecting(*iInfo->sample, iRay);
		}

		static const bool contains(const TAabb& iAabb, const TPoint& iPoint) 
		{ 
			return iAabb.contains(iPoint); 
		}

		static const bool intersect(const TAabb& iAabb, const TRay& iRay, 
			TReference oT, const TParam iMinT)
		{
			TScalar t;
			prim::Result hit = 
				prim::intersect(iAabb, iRay.unboundedRay(), t, iMinT);
			if (hit == prim::rOne)
			{
				oT = t;
				return true;
			}
			return false;
		}

		static const TAabb join(const TAabb& iA, const TAabb& iB) { return iA + iB; }
		static const TPoint min(const TAabb& iAabb) { return iAabb.min(); }
		static const TPoint max(const TAabb& iAabb) { return iAabb.max(); }
		static const TPoint support(const TRay& iRay) { return iRay.support();	}
		static const TVector direction(const TRay& iRay) {	return iRay.direction(); }
		static const TValue coordinate(const TPoint& iPoint, size_t iAxis) { return iPoint[iAxis]; }
		static const TValue component(const TVector& iVector, size_t iAxis) { return iVector[iAxis]; }
		static const TVector reciprocal(const TVector& iVector) { return iVector.reciprocal();	}
	};

	typedef spat::AabbTree<TChildren::value_type, ObjectTraits> TTree;

    TChildren children_;
	TTree tree_;
};



}

}

#endif

// EOF
