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

/** @class liar::scenery::LightPoint
 *  @brief model of a point light
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_LIGHT_DIRECTIONAL_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_LIGHT_DIRECTIONAL_H

#include "scenery_common.h"
#include "../kernel/scene_light.h"
#include <lass/util/pyshadow_object.h>

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL LightDirectional: public SceneLight
{
    PY_HEADER(SceneLight)
public:

	LightDirectional();
	LightDirectional(const TVector3D& iDirection, const Spectrum& iRadiance);

	const TVector3D& direction() const;
	const Spectrum& radiance() const;

	void setDirection(const TVector3D& iDirection);
	void setRadiance(const Spectrum& iRadiance);

private:

    LASS_UTIL_ACCEPT_VISITOR;

	void doIntersect(const Sample& iSample, const BoundedRay& iRay, 
		Intersection& oResult) const;
	const bool doIsIntersecting(const Sample& iSample, const BoundedRay& iRay) const;
	void doLocalContext(const Sample& iSample, const BoundedRay& iRay,
		const Intersection& iIntersection, IntersectionContext& oResult) const;
	const bool doContains(const Sample& iSample, const TPoint3D& iPoint) const;
	const TAabb3D doBoundingBox() const;
	const TScalar doArea() const;

	const Spectrum doSampleEmission(const Sample& iSample, const TVector2D& iLightSample, 
		const TPoint3D& iTarget, const TVector3D& iTargetNormal, BoundedRay& oShadowRay, 
		TScalar& oPdf) const;
	const unsigned doNumberOfEmissionSamples() const;

	const TPyObjectPtr doGetLightState() const;
	void doSetLightState(const TPyObjectPtr& iState);

    TVector3D direction_;
	Spectrum radiance_;
};

}

}

#endif

// EOF
