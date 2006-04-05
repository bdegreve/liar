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

/** @class liar::scenery::MotionTranslation
 *  @brief a translation that varies in time.
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_MOTION_TRANSLATION_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_MOTION_TRANSLATION_H

#include "scenery_common.h"
#include "../kernel/scene_object.h"

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL MotionTranslation: public SceneObject
{
    PY_HEADER(SceneObject)
public:

	MotionTranslation(const TSceneObjectPtr& iChild, 
		const TVector3D& iLocalToWorldStart,
		const TVector3D& iSpeedInWorld);

	const TSceneObjectPtr& child() const;
	void setChild(const TSceneObjectPtr& iChild);

private:

    void doAccept(lass::util::VisitorBase& ioVisitor);

	void doPreProcess(const TimePeriod& iPeriod);
	void doIntersect(const Sample& iSample, const BoundedRay& iRay, 
		Intersection& oResult) const;
	const bool doIsIntersecting(const Sample& iSample, const BoundedRay& iRay) const;
	void doLocalContext(const Sample& iSample, const BoundedRay& iRay,
		const Intersection& iIntersection, IntersectionContext& oResult) const;
	void doLocalSpace(TTime iTime, TTransformation3D& ioLocalToWorld) const;
	const bool doContains(const Sample& iSample, const TPoint3D& iPoint) const;
	const TAabb3D doBoundingBox() const;
	const TScalar doArea() const;
	const bool doHasMotion() const { return true; }

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& iState);

	const TVector3D localToWorld(TTime iTime) const;

    TSceneObjectPtr child_;
	TAabb3D aabb_;
    TVector3D localToWorldStart_;
	TVector3D speedInWorld_;
};



}

}

#endif

// EOF
