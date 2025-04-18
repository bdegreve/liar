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
#include "clip_map.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_DOC(ClipMap, "Applies clip map to child object")
PY_CLASS_CONSTRUCTOR_2(ClipMap, const TSceneObjectRef&, const TTextureRef&)
PY_CLASS_CONSTRUCTOR_3(ClipMap, const TSceneObjectRef&, const TTextureRef&, ClipMap::TValue)
PY_CLASS_MEMBER_RW(ClipMap, child, setChild)
PY_CLASS_MEMBER_RW(ClipMap, clipMap, setClipMap)
PY_CLASS_MEMBER_RW(ClipMap, threshold, setThreshold)

// --- public --------------------------------------------------------------------------------------

ClipMap::ClipMap(const TSceneObjectRef& child, const TTextureRef& clipMap):
	child_(child),
	clipMap_(clipMap),
	threshold_(0.5f)
{
}



ClipMap::ClipMap(const TSceneObjectRef& child, const TTextureRef& clipMap, TValue threshold):
	 child_(child),
	clipMap_(clipMap),
	threshold_(threshold)
{
}



const TSceneObjectRef& ClipMap::child() const
{
	return child_;
}



void ClipMap::setChild(const TSceneObjectRef& child)
{
	child_ = child;
}



const TTextureRef& ClipMap::clipMap() const
{
	return clipMap_;
}



void ClipMap::setClipMap(const TTextureRef& clipMap)
{
	clipMap_ = clipMap;
}



ClipMap::TValue ClipMap::threshold() const
{
	return threshold_;
}



void ClipMap::setThreshold(TValue threshold)
{
	threshold_ = threshold;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------


void ClipMap::doAccept(util::VisitorBase& visitor)
{
	preAccept(visitor, *this);
	child_->accept(visitor);
	postAccept(visitor, *this);
}



void ClipMap::doPreProcess(const TimePeriod& period)
{
	child_->preProcess(period);
}



void ClipMap::doIntersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const
{
	BoundedRay boundedRay = ray;
	Intersection intersection;
	while (true)
	{
		child_->intersect(sample, boundedRay, intersection);
		if (!intersection)
		{
			result.swap(intersection);
			return;
		}

		IntersectionContext context(*child_, sample, boundedRay, intersection, 0);
		if (clipMap_->scalarLookUp(sample, context) >= threshold_)
		{
			intersection.push(this);
			result.swap(intersection);
			return;
		}

		boundedRay = bound(boundedRay, intersection.t(), boundedRay.farLimit());
	}
}



bool ClipMap::doIsIntersecting(const Sample& sample, const BoundedRay& ray) const
{
	Intersection temp;
	this->intersect(sample, ray, temp);
	return static_cast<bool>(temp);
}



void ClipMap::doLocalContext(const Sample& sample, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const
{
	IntersectionDescendor descendor(intersection);
	LASS_ASSERT(intersection.object() == child_.get());
	child_->localContext(sample, ray, intersection, result);
}



bool ClipMap::doContains(const Sample& sample, const TPoint3D& point) const
{
	return child_->contains(sample, point);
}



const TAabb3D ClipMap::doBoundingBox() const
{
	return child_->boundingBox();
}



TScalar ClipMap::doArea() const
{
	return child_->area();
}



TScalar ClipMap::doArea(const TVector3D& normal) const
{
	return child_->area(normal);
}



const TPyObjectPtr ClipMap::doGetState() const
{
	return python::makeTuple(child_, clipMap_, threshold_);
}



void ClipMap::doSetState(const TPyObjectPtr& state)
{
	LASS_ENFORCE(python::decodeTuple(state, child_, clipMap_, threshold_));
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
