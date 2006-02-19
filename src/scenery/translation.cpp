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
#include "translation.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(Translation)
PY_CLASS_CONSTRUCTOR_2(Translation, const TSceneObjectPtr&, const TVector3D&)
PY_CLASS_MEMBER_RW(Translation, "child", child, setChild)
PY_CLASS_MEMBER_RW(Translation, "localToWorld", localToWorld, setLocalToWorld)
PY_CLASS_MEMBER_R(Translation, "worldToLocal", worldToLocal)


// --- public --------------------------------------------------------------------------------------

Translation::Translation(const TSceneObjectPtr& iChild, 
							   const TVector3D& iLocalToWorld):
    SceneObject(&Type),
    child_(iChild),
    localToWorld_(iLocalToWorld)
{
}



const TSceneObjectPtr& Translation::child() const
{
	return child_;
}



void Translation::setChild(const TSceneObjectPtr& iChild)
{
	child_ = iChild;
}



const TVector3D& Translation::localToWorld() const
{
	return localToWorld_;
}



void Translation::setLocalToWorld(const TVector3D& iLocalToWorld)
{
	localToWorld_ = iLocalToWorld;
}



const TVector3D Translation::worldToLocal() const
{
	return -localToWorld_;
}



// --- protected -----------------------------------------------------------------------------------

// --- private -------------------------------------------------------------------------------------

void Translation::doPreProcess(const TimePeriod& iPeriod)
{
	child_->preProcess(iPeriod);
}



void Translation::doAccept(util::VisitorBase& ioVisitor)
{
    doVisit(*this, ioVisitor);
    child_->accept(ioVisitor);
	doVisitOnExit(*this, ioVisitor);
}



void Translation::doIntersect(const Sample& iSample, const BoundedRay& iRay, 
							  Intersection& oResult) const
{
	const BoundedRay localRay = translate(iRay, -localToWorld_);
	child_->intersect(iSample, localRay, oResult);
    if (oResult)
    {
        oResult.push(this);
    }
}



const bool Translation::doIsIntersecting(const Sample& iSample, 
										 const BoundedRay& iRay) const
{
	const BoundedRay localRay = translate(iRay, -localToWorld_);
	return child_->isIntersecting(iSample, localRay);
}



void Translation::doLocalContext(const Sample& iSample, const BoundedRay& iRay,
								 const Intersection& iIntersection, 
								 IntersectionContext& oResult) const
{
	IntersectionDescendor descendor(iIntersection);
	LASS_ASSERT(iIntersection.object() == child_.get());

	const TRay3D localRay(iRay.support() - localToWorld_, iRay.direction());
	child_->localContext(iSample, localRay, iIntersection, oResult);
	oResult.translate(localToWorld_);
}



void Translation::doLocalSpace(TTime iTime, TTransformation3D& ioLocalToWorld) const 
{
	ioLocalToWorld = concatenate(TTransformation3D::translation(localToWorld_), ioLocalToWorld);
}



const bool Translation::doContains(const Sample& iSample, const TPoint3D& iPoint) const
{
	const TPoint3D localPoint = iPoint - localToWorld_;
	return child_->contains(iSample, localPoint);
}



const TAabb3D Translation::doBoundingBox() const
{
	const TAabb3D localBox = child_->boundingBox();
	return TAabb3D(localBox.min() + localToWorld_, localBox.max() + localToWorld_);
}



const TScalar Translation::doArea() const
{
	return child_->area();
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
