/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2025  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::scenery::LightPoint
 *  @brief model of a point light
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_LIGHT_SPOT_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_LIGHT_SPOT_H

#include "scenery_common.h"
#include "../kernel/attenuation.h"
#include "../kernel/scene_light.h"
#include "../kernel/spectrum.h"

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL LightSpot: public SceneLight
{
	PY_HEADER(SceneLight)
public:

	LightSpot();

	const TPoint3D& position() const;
	const TVector3D& direction() const;
	const TSpectrumRef& intensity() const;
	const TAttenuationRef& attenuation() const;
	TScalar outerAngle() const;
	TScalar innerAngle() const;

	void setPosition(const TPoint3D& iPosition);
	void setDirection(const TVector3D& direction);
	void setIntensity(const TSpectrumRef& iIntensity);
	void setAttenuation(const TAttenuationRef& iAttenuation);
	void setOuterAngle(TScalar iRadians);
	void setInnerAngle(TScalar iRadians);

	void lookAt(const TPoint3D& target);

private:

	LASS_UTIL_VISITOR_DO_ACCEPT;

	void doIntersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const override;
	bool doIsIntersecting(const Sample& sample, const BoundedRay& ray) const override;
	void doLocalContext(const Sample& sample, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const override;
	bool doContains(const Sample& sample, const TPoint3D& point) const override;
	const TAabb3D doBoundingBox() const override;
	TScalar doArea() const override;
	TScalar doArea(const TVector3D& normal) const override;

	const Spectral doEmission(const Sample& sample, const TRay3D& ray, BoundedRay& shadowRay, TScalar& pdf) const override;
	const Spectral doSampleEmission(
			const Sample& sample, const TPoint2D& lightSample, const TPoint3D& target,
			BoundedRay& shadowRay, TScalar& pdf) const override;
	const Spectral doSampleEmission(
			const Sample& cameraSample, const TPoint2D& lightSampleA, const TPoint2D& lightSampleB,
			BoundedRay& emissionRay, TScalar& pdf) const override;
	TScalar doTotalPower() const override;
	size_t doNumberOfEmissionSamples() const override;
	bool doIsSingular() const override;


	const TPyObjectPtr doGetLightState() const override;
	void doSetLightState(const TPyObjectPtr& state) override;

	TScalar fallOff(TScalar cosTheta) const;

	TPoint3D position_;
	TVector3D direction_;
	TVector3D tangentU_;
	TVector3D tangentV_;
	TSpectrumRef intensity_;
	TAttenuationRef attenuation_;
	TScalar cosOuterAngle_;
	TScalar cosInnerAngle_;
};

}

}

#endif

// EOF
