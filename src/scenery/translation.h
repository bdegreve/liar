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

/** @class liar::scenery::Translation
 *  @brief linear transformation with translation part only.
 *
 *  This transformation object only performs a translation.  While this can also
 *	be achieved by using the general Transformation object, it is in general much
 *	faster.
 *
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_TRANSLATION_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_TRANSLATION_H

#include "scenery_common.h"
#include "../kernel/scene_object.h"

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL Translation: public kernel::SceneObject
{
    PY_HEADER(kernel::SceneObject)
public:

	Translation(const kernel::TSceneObjectPtr& iChild, const TVector3D& iLocalToWorld);

	const kernel::TSceneObjectPtr& child() const;
	void setChild(const kernel::TSceneObjectPtr& iChild);
	
	const TVector3D& localToWorld() const;
	void setLocalToWorld(const TVector3D& iLocalToWorld);
	
	const TVector3D worldToLocal() const;

private:

    void doAccept(lass::util::VisitorBase& ioVisitor);

	void doIntersect(const kernel::Sample& iSample, const TRay3D& iRay, 
		kernel::Intersection& oResult) const;
	const bool doIsIntersecting(const kernel::Sample& iSample, const TRay3D& iRay, 
		TScalar iMaxT) const;
	void doLocalContext(const kernel::Sample& iSample, const TRay3D& iRay, 
		const kernel::Intersection& iIntersection, kernel::IntersectionContext& oResult) const;
	void doLocalSpace(TTime iTime, TTransformation3D& ioLocalToWorld) const;
	const TAabb3D doBoundingBox(const kernel::TimePeriod& iPeriod) const;

    kernel::TSceneObjectPtr child_;
    TVector3D localToWorld_;
};



}

}

#endif

// EOF
