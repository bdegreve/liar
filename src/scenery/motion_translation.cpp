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

#include "scenery_common.h"
#include "motion_translation.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(MotionTranslation)
PY_CLASS_CONSTRUCTOR_3(MotionTranslation, const kernel::TSceneObjectPtr&, const TVector3D&, const TVector3D&)
PY_CLASS_MEMBER_RW(MotionTranslation, "child", child, setChild)

// --- public --------------------------------------------------------------------------------------

MotionTranslation::MotionTranslation(const kernel::TSceneObjectPtr& iChild, 
									 const TVector3D& iLocalToWorldStart,
									 const TVector3D& iSpeedInWorld):
    SceneObject(&Type),
    child_(iChild),
    localToWorldStart_(iLocalToWorldStart),
	speedInWorld_(iSpeedInWorld)
{
}



const kernel::TSceneObjectPtr& MotionTranslation::child() const
{
	return child_;
}



void MotionTranslation::setChild(const kernel::TSceneObjectPtr& iChild)
{
	child_ = iChild;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void MotionTranslation::doAccept(util::VisitorBase& ioVisitor)
{
    doVisit(*this, ioVisitor);
    child_->accept(ioVisitor);
	doVisitOnExit(*this, ioVisitor);
}



void MotionTranslation::doIntersect(const kernel::Sample& iSample, const kernel::BoundedRay& iRay, 
							  kernel::Intersection& oResult) const
{
	const kernel::BoundedRay localRay = translate(iRay, -localToWorld(iSample.time()));
	child_->intersect(iSample, localRay, oResult);
    if (oResult)
    {
        oResult.push(this);
    }
}



const bool MotionTranslation::doIsIntersecting(const kernel::Sample& iSample, 
											   const kernel::BoundedRay& iRay) const
{
	const kernel::BoundedRay localRay = translate(iRay, -localToWorld(iSample.time()));
	return child_->isIntersecting(iSample, localRay);
}



void MotionTranslation::doLocalContext(const kernel::Sample& iSample, const BoundedRay& iRay,
									   const kernel::Intersection& iIntersection, 
									   kernel::IntersectionContext& oResult) const
{
	kernel::IntersectionDescendor descendor(iIntersection);
	LASS_ASSERT(iIntersection.object() == child_.get());

	const TVector3D offset = localToWorld(iSample.time());
	const TRay3D localRay(iRay.support() - offset, iRay.direction());
	child_->localContext(iSample, localRay, iIntersection, oResult);
	oResult.translate(offset);
}



void MotionTranslation::doLocalSpace(TTime iTime, TTransformation3D& ioLocalToWorld) const 
{
	ioLocalToWorld = concatenate(
		TTransformation3D::translation(localToWorld(iTime)), ioLocalToWorld);
}



const bool MotionTranslation::doContains(const kernel::Sample& iSample, const TPoint3D& iPoint) const
{
	const TPoint3D localPoint = iPoint - localToWorld(iSample.time());
	return child_->contains(iSample, localPoint);
}



const TAabb3D MotionTranslation::doBoundingBox(const kernel::TimePeriod& iPeriod) const
{
	const TAabb3D localBox = child_->boundingBox(iPeriod);
	
	const TVector3D offsetBegin = localToWorld(iPeriod.begin());
	TAabb3D result(localBox.min() + offsetBegin, localBox.max() + offsetBegin);

	const TVector3D offsetEnd = localToWorld(iPeriod.end());
	result += TAabb3D(localBox.min() + offsetEnd, localBox.max() + offsetEnd);
	return result;
}



inline const TVector3D MotionTranslation::localToWorld(TTime iTime) const
{
	return localToWorldStart_ + static_cast<TScalar>(iTime) * speedInWorld_;
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
