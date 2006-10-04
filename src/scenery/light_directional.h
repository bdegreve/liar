/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2006  Bram de Greve
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

/** @class liar::scenery::LightLightDirectional 
 *  @brief model of a directional light like the sun
 *  @author Bram de Greve [Bramz]
 */

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
	LightDirectional(const TVector3D& direction, const Spectrum& radiance);

	const TVector3D& direction() const;
	const Spectrum& radiance() const;

	void setDirection(const TVector3D& direction);
	void setRadiance(const Spectrum& radiance);

private:

    LASS_UTIL_ACCEPT_VISITOR;

	void doPreProcess(const TSceneObjectPtr& scene, const TimePeriod& period);
	void doIntersect(const Sample& sample, const BoundedRay& ray, 
		Intersection& result) const;
	const bool doIsIntersecting(const Sample& sample, const BoundedRay& ray) const;
	void doLocalContext(const Sample& sample, const BoundedRay& ray,
		const Intersection& intersection, IntersectionContext& result) const;
	const bool doContains(const Sample& sample, const TPoint3D& point) const;
	const TAabb3D doBoundingBox() const;
	const TScalar doArea() const;

	const Spectrum doSampleEmission(const Sample& sample, const TPoint2D& lightSample, 
		const TPoint3D& target, const TVector3D& targetNormal, BoundedRay& shadowRay, 
		TScalar& pdf) const;
	const Spectrum doSampleEmission(const TPoint2D& sampleA, const TPoint2D& sampleB,
		const TPoint3D& sceneCenter, TScalar sceneRadius, TRay3D& emissionRay, TScalar& pdf) const;
	const Spectrum doTotalPower(TScalar sceneRadius) const;
	const unsigned doNumberOfEmissionSamples() const;

	const TPyObjectPtr doGetLightState() const;
	void doSetLightState(const TPyObjectPtr& state);

    TVector3D direction_;
	TVector3D tangentU_;
	TVector3D tangentV_;
	Spectrum radiance_;
};

}

}

#endif

// EOF