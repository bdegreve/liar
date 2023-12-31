/** @file
*  @author Bram de Greve (bramz@users.sourceforge.net)
*
*  LiAR isn't a raytracer
*  Copyright (C) 2004-2023  Bram de Greve (bramz@users.sourceforge.net)
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

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_LIGHT_PROJECTION_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_LIGHT_PROJECTION_H

#include "scenery_common.h"
#include "../kernel/attenuation.h"
#include "../kernel/projection.h"
#include "../kernel/scene_light.h"
#include "../kernel/texture.h"
#include <lass/util/progress_indicator.h>

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL LightProjection: public SceneLight
{
	PY_HEADER(SceneLight)
public:

	LightProjection();

	const TTexturePtr& intensity() const;
	void setIntensity(const TTexturePtr& iIntensity);

	const TProjectionPtr& projection() const;
	void setProjection(const TProjectionPtr& projection);

	const TAttenuationPtr& attenuation() const;
	void setAttenuation(const TAttenuationPtr& iAttenuation);

	const TResolution2D& samplingResolution() const;
	void setSamplingResolution(const TResolution2D& resolution);

private:

	typedef std::vector<TScalar> TMap;

	LASS_UTIL_VISITOR_DO_ACCEPT;

	void doPreProcess(const TimePeriod& period);

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

	void buildPdf(TMap& pdf, TScalar& power, util::ProgressIndicator& progress) const;
	void buildCdf(const TMap& iPdf, TMap& oMarginalCdfU, TMap& oConditionalCdfV, util::ProgressIndicator& progress) const;
	void sampleMap(const TPoint2D& sample, TScalar&, TScalar& j, TScalar& pdf) const;

	TTexturePtr intensity_;
	TProjectionPtr projection_;
	TAttenuationPtr attenuation_;
	TMap marginalCdfU_;
	TMap conditionalCdfV_;
	TResolution2D resolution_;
	TVector2D invResolution_;
	TScalar power_;
};

}

}

#endif

// EOF
