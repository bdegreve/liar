/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
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
 *  http://liar.sourceforge.net
 */

/** @class liar::scenery::LightPoint
 *  @brief model of a point light
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_LIGHT_AREA_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_LIGHT_AREA_H

#include "scenery_common.h"
#include "../kernel/attenuation.h"
#include "../kernel/scene_light.h"

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL LightArea: public SceneLight
{
    PY_HEADER(SceneLight)
public:

	LightArea(const TSceneObjectPtr& iSurface);

	const TSceneObjectPtr& surface() const;
	const Spectrum& radiance() const;
	const TAttenuationPtr& attenuation() const;
	// const unsigned numberOfEmissionSamples() const; [via SceneLight]
	const bool isDoubleSided() const;

	void setRadiance(const Spectrum& radiance);
	void setAttenuation(const TAttenuationPtr& iAttenuation);
	void setNumberOfEmissionSamples(unsigned iNumberOfSamples);
	void setDoubleSided(bool iIsDoubleSided);

private:

	LASS_UTIL_ACCEPT_VISITOR;

	void doIntersect(const Sample& sample, const BoundedRay& ray, 
		Intersection& result) const;
	const bool doIsIntersecting(const Sample& sample, const BoundedRay& ray) const;
	void doLocalContext(const Sample& sample, const BoundedRay& ray,
		const Intersection& intersection, IntersectionContext& result) const;
	const bool doContains(const Sample& sample, const TPoint3D& point) const;

	const TAabb3D doBoundingBox() const;
	const TScalar doArea() const;

	const Spectrum doEmission(const Sample& sample, const TRay3D& ray, 
		BoundedRay& shadowRay, TScalar& pdf) const;
	const Spectrum doSampleEmission(const Sample& sample, const TPoint2D& lightSample, 
		const TPoint3D& target, const TVector3D& targetNormal, 
		BoundedRay& shadowRay, TScalar& pdf) const;
	const Spectrum doSampleEmission(const Sample& cameraSample, 
		const TPoint2D& lightSampleA, const TPoint2D& lightSampleB, const TAabb3D& sceneBound, 
		BoundedRay& emissionRay, TScalar& pdf) const;
	const Spectrum doTotalPower(const TAabb3D& sceneBound) const;
	const unsigned doNumberOfEmissionSamples() const;
	const bool doIsSingular() const;

	const TPyObjectPtr doGetLightState() const;
	void doSetLightState(const TPyObjectPtr& state);

	TSceneObjectPtr surface_;
	Spectrum radiance_;
	TAttenuationPtr attenuation_;
	unsigned numberOfEmissionSamples_;
	bool isSingleSided_;
};

}

}

#endif

// EOF
