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
#include "motion_translation.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_DOC(MotionTranslation, "time-dependent translation")
PY_CLASS_CONSTRUCTOR_3(MotionTranslation, const TSceneObjectPtr&, const TVector3D&, const TVector3D&)
PY_CLASS_MEMBER_RW(MotionTranslation, child, setChild)

// --- public --------------------------------------------------------------------------------------

MotionTranslation::MotionTranslation(const TSceneObjectPtr& child, 
									 const TVector3D& iLocalToWorldStart,
									 const TVector3D& iSpeedInWorld):
    child_(LASS_ENFORCE_POINTER(child)),
    localToWorldStart_(iLocalToWorldStart),
	speedInWorld_(iSpeedInWorld)
{
}



const TSceneObjectPtr& MotionTranslation::child() const
{
	return child_;
}



void MotionTranslation::setChild(const TSceneObjectPtr& child)
{
	child_ = LASS_ENFORCE_POINTER(child);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------


void MotionTranslation::doAccept(util::VisitorBase& visitor)
{
    preAccept(visitor, *this);
    child_->accept(visitor);
	postAccept(visitor, *this);
}



void MotionTranslation::doPreProcess(const TSceneObjectPtr& scene, const TimePeriod& period)
{
	child_->preProcess(scene, period);

	const TAabb3D localBox = child_->boundingBox();

	const TVector3D offsetBegin = localToWorldOffset(period.begin());
	const TVector3D offsetEnd = localToWorldOffset(period.end());
	aabb_ = TAabb3D(localBox.min() + offsetBegin, localBox.max() + offsetBegin);
	aabb_ += TAabb3D(localBox.min() + offsetEnd, localBox.max() + offsetEnd);
}



void MotionTranslation::doIntersect(const Sample& sample, const BoundedRay& ray, 
							  Intersection& result) const
{
	const BoundedRay localRay = translate(ray, -localToWorldOffset(sample.time()));
	child_->intersect(sample, localRay, result);
    if (result)
    {
        result.push(this);
    }
}



const bool MotionTranslation::doIsIntersecting(const Sample& sample, 
											   const BoundedRay& ray) const
{
	const BoundedRay localRay = translate(ray, -localToWorldOffset(sample.time()));
	return child_->isIntersecting(sample, localRay);
}



void MotionTranslation::doLocalContext(const Sample& sample, const BoundedRay& ray,
									   const Intersection& intersection, 
									   IntersectionContext& result) const
{
	IntersectionDescendor descendor(intersection);
	LASS_ASSERT(intersection.object() == child_.get());

	const TVector3D offset = localToWorldOffset(sample.time());
	const TRay3D localRay(ray.support() - offset, ray.direction());
	child_->localContext(sample, localRay, intersection, result);
	result.translateBy(offset);
}



void MotionTranslation::doLocalSpace(TTime time, TTransformation3D& localToWorld) const 
{
	localToWorld = concatenate(
		TTransformation3D::translation(localToWorldOffset(time)), localToWorld);
}



const bool MotionTranslation::doContains(const Sample& sample, const TPoint3D& point) const
{
	const TPoint3D localPoint = point - localToWorldOffset(sample.time());
	return child_->contains(sample, localPoint);
}



const TAabb3D MotionTranslation::doBoundingBox() const
{
	return aabb_;
}



const TScalar MotionTranslation::doArea() const
{
	return child_->area();
}



const TPyObjectPtr MotionTranslation::doGetState() const
{
	return python::makeTuple(child_, localToWorldStart_, speedInWorld_);
}



void MotionTranslation::doSetState(const TPyObjectPtr& state)
{
	LASS_ENFORCE(python::decodeTuple(state, child_, localToWorldStart_, speedInWorld_));
}



inline const TVector3D MotionTranslation::localToWorldOffset(TTime time) const
{
	return localToWorldStart_ + static_cast<TScalar>(time) * speedInWorld_;
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
