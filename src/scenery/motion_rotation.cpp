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
#include "motion_rotation.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_DOC(MotionRotation, "time-dependent rotation")
PY_CLASS_CONSTRUCTOR_4(MotionRotation, const TSceneObjectPtr&, const TVector3D&, TScalar, TScalar)
PY_CLASS_MEMBER_RW(MotionRotation, child, setChild)
PY_CLASS_MEMBER_RW(MotionRotation, axis, setAxis)
PY_CLASS_MEMBER_RW_DOC(MotionRotation, start, setStart, "rotation angle at time = 0 seconds, in radians")
PY_CLASS_MEMBER_RW_DOC(MotionRotation, speed, setSpeed, "rotation speed, in radians per second")

// --- public --------------------------------------------------------------------------------------

MotionRotation::MotionRotation(const TSceneObjectPtr& child, const TVector3D& axis, TScalar startAngleRadians, TScalar speedAngleRadians):
	child_(LASS_ENFORCE_POINTER(child)),
	axis_(axis),
	start_(startAngleRadians),
	speed_(speedAngleRadians)
{
}



const TSceneObjectPtr& MotionRotation::child() const
{
	return child_;
}



void MotionRotation::setChild(const TSceneObjectPtr& child)
{
	child_ = LASS_ENFORCE_POINTER(child);
}



const TVector3D& MotionRotation::axis() const
{
	return axis_;
}



void MotionRotation::setAxis(const TVector3D& axis)
{
	axis_ = axis;
}



TScalar MotionRotation::start() const
{
	return start_;
}



void MotionRotation::setStart(TScalar start)
{
	start_ = start;
}



TScalar MotionRotation::speed() const
{
	return speed_;
}



void MotionRotation::setSpeed(TScalar speed)
{
	speed_ = speed;
}


// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------


void MotionRotation::doAccept(util::VisitorBase& visitor)
{
	preAccept(visitor, *this);
	child_->accept(visitor);
	postAccept(visitor, *this);
}



void MotionRotation::doPreProcess(const TSceneObjectPtr& scene, const TimePeriod& period)
{
	child_->preProcess(scene, period);

	const TAabb3D localBox = child_->boundingBox();
	if (localBox.isEmpty())
	{
		aabb_ = localBox;
		return;
	}
	aabb_ = prim::transform(localBox, worldToLocal(period.begin()).inverse());
	if (period.end() == period.begin())
	{
		return;
	}
	aabb_ += prim::transform(localBox, worldToLocal(period.end()).inverse());

	const TScalar theta1 = angle(period.begin());
	const TScalar theta2 = angle(period.end());
	const TScalar quadrant = TNumTraits::pi / 2;
	const int k1 = static_cast<int>(num::ceil(theta1 / quadrant));
	const int k2 = static_cast<int>(num::floor(theta2 / quadrant));
	for (int k = k1; k <= k2; ++k)
	{
		TTransformation3D localToWorld = TTransformation3D::rotation(axis_, k * quadrant);
		aabb_ += prim::transform(localBox, localToWorld);
	}
}



void MotionRotation::doIntersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const
{
	const BoundedRay localRay = transform(ray, worldToLocal(sample.time()));
	child_->intersect(sample, localRay, result);
	if (result)
	{
		result.push(this);
	}
}



bool MotionRotation::doIsIntersecting(const Sample& sample, const BoundedRay& ray) const
{
	const BoundedRay localRay = transform(ray, worldToLocal(sample.time()));
	return child_->isIntersecting(sample, localRay);
}



void MotionRotation::doLocalContext(const Sample& sample, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const
{
	IntersectionDescendor descendor(intersection);
	LASS_ASSERT(intersection.object() == child_.get());

	const BoundedRay localRay = transform(ray, worldToLocal(sample.time()));
	child_->localContext(sample, localRay, intersection, result);
	result.transformBy(worldToLocal(sample.time()).inverse());
}



void MotionRotation::doLocalSpace(TTime time, TTransformation3D& localToWorld) const
{
	localToWorld = concatenate(worldToLocal(time).inverse(), localToWorld);
}



bool MotionRotation::doContains(const Sample& sample, const TPoint3D& point) const
{
	const TPoint3D localPoint = prim::transform(point, worldToLocal(sample.time()));
	return child_->contains(sample, localPoint);
}



const TAabb3D MotionRotation::doBoundingBox() const
{
	return aabb_;
}



TScalar MotionRotation::doArea() const
{
	return child_->area();
}



TScalar MotionRotation::doArea(const TVector3D& normal) const
{
	return child_->area(normal);
}



const TPyObjectPtr MotionRotation::doGetState() const
{
	return python::makeTuple(child_, axis_, start_, speed_);
}



void MotionRotation::doSetState(const TPyObjectPtr& state)
{
	LASS_ENFORCE(python::decodeTuple(state, child_, axis_, start_, speed_));
}



inline TScalar MotionRotation::angle(TTime time) const
{
	return start_ + static_cast<TScalar>(time) * speed_;
}



inline const TTransformation3D& MotionRotation::worldToLocal(TTime time) const
{
	TransformationCache& cache = *cache_;
	if (cache.time != time)
	{
		cache.localToWorld = TTransformation3D::rotation(axis_, -angle(time));
		cache.time = time;
	}
	return cache.localToWorld;
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
