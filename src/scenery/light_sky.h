/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::scenery::LightSky
 *  @brief model of a point light
 *  @author Bram de Greve [Bramz]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_LIGHT_SKY_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_LIGHT_SKY_H

#include "scenery_common.h"
#include "../kernel/scene_light.h"
#include "../kernel/texture.h"

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL LightSky: public SceneLight
{
	PY_HEADER(SceneLight)
public:

	LightSky();
	LightSky(const TTexturePtr& radiance_);

	const TTexturePtr& radiance() const;
	// const unsigned numberOfEmissionSamples() const; [via SceneLight]
	const TResolution2D& samplingResolution() const;

	void setRadiance(const TTexturePtr& radiance);
	void setNumberOfEmissionSamples(unsigned iNumberOfSamples);
	void setSamplingResolution(const TResolution2D& resolution);

private:

	typedef std::vector<TScalar> TMap;
	typedef std::vector<XYZ> TXYZMap;

	LASS_UTIL_VISITOR_DO_ACCEPT;

	void doPreProcess(const TSceneObjectPtr& scene, const TimePeriod& period);

	void doIntersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const;
	bool doIsIntersecting(const Sample& sample, const BoundedRay& ray) const;
	void doLocalContext(const Sample& sample, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const;
	bool doContains(const Sample& sample, const TPoint3D& point) const;
	const TAabb3D doBoundingBox() const;
	TScalar doArea() const;

	const XYZ doEmission(const Sample& sample, const TRay3D& ray, BoundedRay& shadowRay, TScalar& pdf) const;
	const XYZ doSampleEmission(
			const Sample& sample, const TPoint2D& lightSample, const TPoint3D& target, 
			const TVector3D& targetNormal, BoundedRay& shadowRay, TScalar& pdf) const;
	const XYZ doSampleEmission(
			const Sample& cameraSample, const TPoint2D& lightSampleA, const TPoint2D& lightSampleB, 
			const TAabb3D& sceneBound, BoundedRay& emissionRay, TScalar& pdf) const;
	const XYZ doTotalPower(const TAabb3D& sceneBound) const;
	size_t doNumberOfEmissionSamples() const;
	bool doIsSingular() const;

	const TPyObjectPtr doGetLightState() const;
	void doSetLightState(const TPyObjectPtr& state);

	void buildPdf(TMap& pdf, TXYZMap& radianceMap, XYZ& averageRadiance) const;
	void buildCdf(const TMap& iPdf, TMap& oMarginalCdfU, TMap& oConditionalCdfV) const;
	void sampleMap(const TPoint2D& sample, TScalar&, TScalar& j, TScalar& pdf) const;
	const TVector3D direction(TScalar i, TScalar j) const;
	const XYZ lookUpRadiance(const Sample& sample, TScalar i, TScalar j) const;

	XYZ averageRadiance_;
	TTexturePtr radiance_;
	TMap marginalCdfU_;
	TMap conditionalCdfV_;
	TXYZMap radianceMap_;
	unsigned numberOfSamples_;
	TResolution2D resolution_;
	TVector2D invResolution_;
	TScalar radius_;
};

}

}

#endif

// EOF
