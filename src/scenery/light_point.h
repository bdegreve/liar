/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
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

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_LIGHT_POINT_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_LIGHT_POINT_H

#include "scenery_common.h"
#include "../kernel/spectrum.h"
#include "../kernel/attenuation.h"
#include "../kernel/scene_light.h"

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL LightPoint: public SceneLight
{
	PY_HEADER(SceneLight)
public:

	LightPoint();
	LightPoint(const TPoint3D& iPosition, const TSpectrumPtr& iIntensity);

	const TPoint3D& position() const;
	const TSpectrumPtr& intensity() const;
	const TAttenuationPtr& attenuation() const;

	void setPosition(const TPoint3D& iPosition);
	void setIntensity(const TSpectrumPtr& iIntensity);
	void setAttenuation(const TAttenuationPtr& iAttenuation);

private:

	LASS_UTIL_VISITOR_DO_ACCEPT;

	void doIntersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const;
	bool doIsIntersecting(const Sample& sample, const BoundedRay& ray) const;
	void doLocalContext(const Sample& sample, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const;
	bool doContains(const Sample& sample, const TPoint3D& point) const;
	const TAabb3D doBoundingBox() const;
	TScalar doArea() const;
	TScalar doArea(const TVector3D& normal) const;

	const Spectral doEmission(const Sample& sample, const TRay3D& ray, BoundedRay& shadowRay, TScalar& pdf) const;
	const Spectral doSampleEmission(
			const Sample& sample, const TPoint2D& lightSample, const TPoint3D& target,
			BoundedRay& shadowRay, TScalar& pdf) const;
	const Spectral doSampleEmission(
			const Sample& cameraSample, const TPoint2D& lightSampleA, const TPoint2D& lightSampleB,
			BoundedRay& emissionRay, TScalar& pdf) const;
	TScalar doTotalPower() const;
	size_t doNumberOfEmissionSamples() const;
	bool doIsSingular() const;

	const TPyObjectPtr doGetLightState() const;
	void doSetLightState(const TPyObjectPtr& state);

	TPoint3D position_;
	TSpectrumPtr intensity_;
	TAttenuationPtr attenuation_;
};

}

}

#endif

// EOF
