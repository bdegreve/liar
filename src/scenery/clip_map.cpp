/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
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
PY_CLASS_CONSTRUCTOR_2(ClipMap, const TSceneObjectPtr&, const TTexturePtr&)
PY_CLASS_CONSTRUCTOR_3(ClipMap, const TSceneObjectPtr&, const TTexturePtr&, TScalar)
PY_CLASS_MEMBER_RW(ClipMap, child, setChild)
PY_CLASS_MEMBER_RW(ClipMap, clipMap, setClipMap)
PY_CLASS_MEMBER_RW(ClipMap, threshold, setThreshold)

// --- public --------------------------------------------------------------------------------------

ClipMap::ClipMap(const TSceneObjectPtr& child, const TTexturePtr& clipMap):
    child_(LASS_ENFORCE_POINTER(child)),
    clipMap_(LASS_ENFORCE_POINTER(clipMap)),
	threshold_(0.5f)
{
}



ClipMap::ClipMap(const TSceneObjectPtr& child, const TTexturePtr& clipMap, TScalar threshold):
    child_(LASS_ENFORCE_POINTER(child)),
    clipMap_(clipMap),
	threshold_(threshold)
{
}



const TSceneObjectPtr& ClipMap::child() const
{
	return child_;
}



void ClipMap::setChild(const TSceneObjectPtr& child)
{
	child_ = LASS_ENFORCE_POINTER(child);
}



const TTexturePtr& ClipMap::clipMap() const
{
	return clipMap_;
}



void ClipMap::setClipMap(const TTexturePtr& clipMap)
{
	clipMap_ = LASS_ENFORCE_POINTER(clipMap);
}



const TScalar ClipMap::threshold() const
{
	return threshold_;
}



void ClipMap::setThreshold(TScalar threshold)
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



void ClipMap::doPreProcess(const TSceneObjectPtr& scene, const TimePeriod& period)
{
	child_->preProcess(scene, period);
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

		IntersectionContext context;
		child_->localContext(sample, boundedRay, intersection, context);
		if (average(clipMap_->lookUp(sample, context)) >= threshold_)
		{
			intersection.push(this);
			result.swap(intersection);
			return;
		}

		boundedRay = bound(boundedRay, intersection.t(), boundedRay.farLimit());
	}
}



const bool ClipMap::doIsIntersecting(const Sample& sample, const BoundedRay& ray) const
{
	Intersection temp;
	this->intersect(sample, ray, temp);
	return temp;
}



void ClipMap::doLocalContext(
		const Sample& sample, const BoundedRay& ray, const Intersection& intersection, 
		IntersectionContext& result) const
{
	IntersectionDescendor descendor(intersection);
	LASS_ASSERT(intersection.object() == child_.get());
	child_->localContext(sample, ray, intersection, result);
}



const bool ClipMap::doContains(const Sample& sample, const TPoint3D& point) const
{
	return child_->contains(sample, point);
}



const TAabb3D ClipMap::doBoundingBox() const
{
	return child_->boundingBox();;
}



const TScalar ClipMap::doArea() const
{
	return child_->area();
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
