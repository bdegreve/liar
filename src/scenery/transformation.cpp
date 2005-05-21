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
PY_CLASS_CONSTRUCTOR_2(Transformation, const kernel::TSceneObjectPtr&, const TTransformation3D&)
PY_CLASS_MEMBER_RW(Transformation, "child", child, setChild)
PY_CLASS_MEMBER_RW(Transformation, "localToWorld", localToWorld, setLocalToWorld)
PY_CLASS_MEMBER_R(Transformation, "worldToLocal", worldToLocal)


// --- public --------------------------------------------------------------------------------------

Transformation::Transformation(const kernel::TSceneObjectPtr& iChild, 
							   const TTransformation3D& iLocalToWorld):
    SceneObject(&Type),
    child_(iChild),
    localToWorld_(iLocalToWorld)
{
}



const kernel::TSceneObjectPtr& Transformation::child() const
{
	return child_;
}



void Transformation::setChild(const kernel::TSceneObjectPtr& iChild)
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

void Transformation::doAccept(util::VisitorBase& ioVisitor)
{
    doVisit(*this, ioVisitor);
    child_->accept(ioVisitor);
	doVisitOnExit(*this, ioVisitor);
}



void Transformation::doIntersect(const TRay3D& iRay, 
								 kernel::Intersection& oResult) const
{
	const TRay3D localRay = transform(iRay, localToWorld_.inverse());
	child_->intersect(localRay, oResult);
}



const bool Transformation::doIsIntersecting(const TRay3D& iRay, TScalar iMaxT,
											const SceneObject* iExcludeA, 
											const SceneObject* iExcludeB) const
{
	const TRay3D localRay = transform(iRay, localToWorld_.inverse());
	return child_->isIntersecting(localRay, iMaxT);
}



void Transformation::doLocalContext(const TRay3D& iRay, 
									const kernel::Intersection& iIntersection, 
									kernel::IntersectionContext& oResult) const
{
	const TRay3D localRay = transform(iRay, localToWorld_.inverse());
	child_->localContext(localRay, iIntersection, oResult);
	oResult.transform(localToWorld_);
}



void Transformation::doLocalSpace(TTransformation3D& ioLocalToWorld) const 
{
	ioLocalToWorld = concatenate(localToWorld_, ioLocalToWorld);
}



const TAabb3D Transformation::doBoundingBox() const
{
	return transform(child_->boundingBox(), localToWorld_);
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
