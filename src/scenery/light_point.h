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
#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_LIGHT_POINT_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_LIGHT_POINT_H

#include "scenery_common.h"
#include "../kernel/scene_light.h"
#include <lass/util/pyshadow_object.h>

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL LightPoint: public kernel::SceneLight
{
    PY_HEADER(kernel::SceneLight)
public:

    struct Attenuation
    {
        TScalar constant;
        TScalar linear;
        TScalar quadratic;

		Attenuation();
        Attenuation(TScalar iConstant, TScalar iLinear, TScalar iQuadratic);
    };

	LightPoint();
	LightPoint(const TPoint3D& iPosition, const kernel::Spectrum& iPower);

	const TPoint3D& position() const;
	const kernel::Spectrum& power() const;
	const Attenuation& attenuation() const;

	void setPosition(const TPoint3D& iPosition);
	void setPower(const kernel::Spectrum& iPower);
	void setAttenuation(const Attenuation& iAttenuation);

private:

    LASS_UTIL_ACCEPT_VISITOR;

	void doIntersect(const kernel::Sample& iSample, const kernel::BoundedRay& iRay, 
		kernel::Intersection& oResult) const;
	const bool doIsIntersecting(const kernel::Sample& iSample, const kernel::BoundedRay& iRay) const;
	void doLocalContext(const kernel::Sample& iSample, const TRay3D& iRay, 
		const kernel::Intersection& iIntersection, kernel::IntersectionContext& oResult) const;
	const bool doContains(const kernel::Sample& iSample, const TPoint3D& iPoint) const;
	const TAabb3D doBoundingBox(const kernel::TimePeriod& iPeriod) const;

	const kernel::Spectrum doSampleRadiance(const TVector2D& iSample, const TPoint3D& iDestionation,
		kernel::BoundedRay& oShadowRay) const;
	const unsigned doNumberOfRadianceSamples() const;

    TPoint3D position_;
	kernel::Spectrum power_;
    Attenuation attenuation_;
};

PY_SHADOW_CLASS(LIAR_SCENERY_DLL, PyLightPointAttenuation, LightPoint::Attenuation);

}

}

PY_SHADOW_CASTERS(liar::scenery::PyLightPointAttenuation);

#endif

// EOF
