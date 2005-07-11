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
PY_CLASS_CONSTRUCTOR_2(Translation, const kernel::TSceneObjectPtr&, const TVector3D&)
PY_CLASS_MEMBER_RW(Translation, "child", child, setChild)
PY_CLASS_MEMBER_RW(Translation, "localToWorld", localToWorld, setLocalToWorld)
PY_CLASS_MEMBER_R(Translation, "worldToLocal", worldToLocal)


// --- public --------------------------------------------------------------------------------------

Translation::Translation(const kernel::TSceneObjectPtr& iChild, 
							   const TVector3D& iLocalToWorld):
    SceneObject(&Type),
    child_(iChild),
    localToWorld_(iLocalToWorld)
{
}



const kernel::TSceneObjectPtr& Translation::child() const
{
	return child_;
}



void Translation::setChild(const kernel::TSceneObjectPtr& iChild)
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

void Translation::doAccept(util::VisitorBase& ioVisitor)
{
    doVisit(*this, ioVisitor);
    child_->accept(ioVisitor);
	doVisitOnExit(*this, ioVisitor);
}



void Translation::doIntersect(const kernel::Sample& iSample, const TRay3D& iRay, 
							  kernel::Intersection& oResult) const
{
	const TRay3D localRay(iRay.support() - localToWorld_, iRay.direction());
	child_->intersect(iSample, localRay, oResult);
    if (oResult)
    {
        oResult.push(this);
    }
}



const bool Translation::doIsIntersecting(const kernel::Sample& iSample, const TRay3D& iRay, 
										 TScalar iMaxT) const
{
	const TRay3D localRay(iRay.support() - localToWorld_, iRay.direction());
	return child_->isIntersecting(iSample, localRay, iMaxT);
}



void Translation::doLocalContext(const kernel::Sample& iSample, const TRay3D& iRay, 
								 const kernel::Intersection& iIntersection, 
								 kernel::IntersectionContext& oResult) const
{
    kernel::IntersectionDescendor descend(iIntersection);
	const TRay3D localRay(iRay.support() - localToWorld_, iRay.direction());
	child_->localContext(iSample, localRay, iIntersection, oResult);
	oResult.translate(localToWorld_);
}



void Translation::doLocalSpace(TTime iTime, TTransformation3D& ioLocalToWorld) const 
{
	ioLocalToWorld = concatenate(TTransformation3D::translation(localToWorld_), ioLocalToWorld);
}



const TAabb3D Translation::doBoundingBox(const kernel::TimePeriod& iPeriod) const
{
	const TAabb3D localBox = child_->boundingBox(iPeriod);
	return TAabb3D(localBox.min() + localToWorld_, localBox.max() + localToWorld_);
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
