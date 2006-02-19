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
#include "transformation.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(Transformation)
PY_CLASS_CONSTRUCTOR_2(Transformation, const TSceneObjectPtr&, const TTransformation3D&)
PY_CLASS_MEMBER_RW(Transformation, "child", child, setChild)
PY_CLASS_MEMBER_RW(Transformation, "localToWorld", localToWorld, setLocalToWorld)
PY_CLASS_MEMBER_R(Transformation, "worldToLocal", worldToLocal)


// --- public --------------------------------------------------------------------------------------

Transformation::Transformation(const TSceneObjectPtr& iChild, 
							   const TTransformation3D& iLocalToWorld):
    SceneObject(&Type),
    child_(iChild),
    localToWorld_(iLocalToWorld)
{
}



const TSceneObjectPtr& Transformation::child() const
{
	return child_;
}



void Transformation::setChild(const TSceneObjectPtr& iChild)
{
	child_ = iChild;
}



const TTransformation3D& Transformation::localToWorld() const
{
	return localToWorld_;
}



void Transformation::setLocalToWorld(const TTransformation3D& iLocalToWorld)
{
	localToWorld_ = iLocalToWorld;
}



const TTransformation3D Transformation::worldToLocal() const
{
	return localToWorld_.inverse();
}



// --- protected -----------------------------------------------------------------------------------

// --- private -------------------------------------------------------------------------------------

void Transformation::doPreProcess(const TimePeriod& iPeriod)
{
	child_->preProcess(iPeriod);
}



void Transformation::doAccept(util::VisitorBase& ioVisitor)
{
    doVisit(*this, ioVisitor);
    child_->accept(ioVisitor);
	doVisitOnExit(*this, ioVisitor);
}



void Transformation::doIntersect(const Sample& iSample, const BoundedRay& iRay, 
								 Intersection& oResult) const
{
	TScalar tScaler = TNumTraits::one;
	const BoundedRay localRay = transform(iRay, localToWorld_.inverse(), tScaler);
	child_->intersect(iSample, localRay, oResult);
    if (oResult)
    {
        oResult.push(this, oResult.t() / tScaler);
    }
}



const bool Transformation::doIsIntersecting(const Sample& iSample, 
											const BoundedRay& iRay) const
{
	const BoundedRay localRay = transform(iRay, localToWorld_.inverse());
	return child_->isIntersecting(iSample, localRay);
}



void Transformation::doLocalContext(const Sample& iSample, const BoundedRay& iRay,
									const Intersection& iIntersection, 
									IntersectionContext& oResult) const
{
	IntersectionDescendor descendor(iIntersection);
	LASS_ASSERT(iIntersection.object() == child_.get());

	const BoundedRay localRay = ::liar::kernel::transform(iRay, localToWorld_.inverse());
	child_->localContext(iSample, localRay, iIntersection, oResult);

	oResult.transform(localToWorld_);
	oResult.setT(iIntersection.t());
}



void Transformation::doLocalSpace(TTime iTime, TTransformation3D& ioLocalToWorld) const
{
	ioLocalToWorld = concatenate(localToWorld_, ioLocalToWorld);
}



const bool Transformation::doContains(const Sample& iSample, const TPoint3D& iPoint) const
{
	const TPoint3D localPoint = transform(iPoint, localToWorld_.inverse());
	return child_->contains(iSample, localPoint);
}



const TAabb3D Transformation::doBoundingBox() const
{
	return transform(child_->boundingBox(), localToWorld_);
}



const TScalar Transformation::doArea() const
{
	return child_->area();
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
