/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2024  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::SceneLight
 *  @brief base class of all light emiting scene objects
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SCENE_LIGHT_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SCENE_LIGHT_H

#include "kernel_common.h"
#include "scene_object.h"

namespace liar
{
namespace kernel
{

class Intersection;

class LIAR_KERNEL_DLL SceneLight: public SceneObject
{
	PY_HEADER(SceneObject)
public:

	/** Return radiance emitted by this light source and received at ray.support() along that ray.
		Adjusts shadowRay to be bounded by ray.support() and the point of emission.
		Sets pdf to the the same value it would be if sampleEmssion(cameraSample, lightSample, target, shadowRay, pdf)
		would result the same shadowRay.
	 */
	const Spectral emission(const Sample& sample, const TRay3D& ray, BoundedRay& shadowRay, TScalar& pdf) const
	{
		return doEmission(sample, ray, shadowRay, pdf);
	}

	/** Sample radiance emitted by this light source and received at target (to be used as shadowRay.support())
	 */
	const Spectral sampleEmission(
			const Sample& cameraSample, const TPoint2D& lightSample, const TPoint3D& target,
			BoundedRay& shadowRay, TScalar& pdf) const
	{
		return doSampleEmission(cameraSample, lightSample, target, shadowRay, pdf);
	}

	/** Sample radiance emitted by this light source and received at target (to be used as shadowRay.support()) with known normal.
	 */
	const Spectral sampleEmission(
			const Sample& cameraSample, const TPoint2D& lightSample, const TPoint3D& target, const TVector3D& targetNormal,
			BoundedRay& shadowRay, TScalar& pdf) const
	{
		return doSampleEmission(cameraSample, lightSample, target, targetNormal, shadowRay, pdf);
	}

	/** Generate an emissionRay.
	 */
	const Spectral sampleEmission(
			const Sample& cameraSample, const TPoint2D& lightSampleA, const TPoint2D& lightSampleB,
			BoundedRay& emissionRay, TScalar& pdf) const
	{
		return doSampleEmission(cameraSample, lightSampleA, lightSampleB, emissionRay, pdf);
	}

	/** Return total power output for light, but averaged over all frequencies
	 */
	TScalar totalPower() const
	{
		return doTotalPower();
	}

	size_t numberOfEmissionSamples() const
	{
		return doNumberOfEmissionSamples();
	}
	bool isSingular() const
	{
		return doIsSingular();
	}

	bool isShadowless() const { return isShadowless_; }
	void setShadowless(bool iIsShadowless) { isShadowless_ = iIsShadowless; }


protected:

	SceneLight();

private:

	LASS_UTIL_VISITOR_DO_ACCEPT;

	const SceneLight* doAsLight() const override;

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;


	virtual const Spectral doEmission(const Sample& sample, const TRay3D& ray, BoundedRay& shadowRay, TScalar& pdf) const = 0;
	virtual const Spectral doSampleEmission(
			const Sample& sample, const TPoint2D& lightSample, const TPoint3D& target,
			BoundedRay& shadowRay, TScalar& pdf) const = 0;
	virtual const Spectral doSampleEmission(
			const Sample& sample, const TPoint2D& lightSample, const TPoint3D& target, const TVector3D& targetNormal,
			BoundedRay& shadowRay, TScalar& pdf) const;
	virtual const Spectral doSampleEmission(
			const Sample& cameraSample, const TPoint2D& lightSampleA, const TPoint2D& lightSampleB,
			BoundedRay& emissionRay, TScalar& pdf) const = 0;
	virtual TScalar doTotalPower() const = 0;
	virtual size_t doNumberOfEmissionSamples() const = 0;
	virtual bool doIsSingular() const = 0;

	virtual const TPyObjectPtr doGetLightState() const = 0;
	virtual void doSetLightState(const TPyObjectPtr& state) = 0;

	bool isShadowless_;
};

typedef python::PyObjectPtr<SceneLight>::Type TSceneLightPtr;


/** @class liar::SceneLight
 *  @brief global lights can only exists in global space, and cannot be transformed
 *  @author Bram de Greve [Bramz]
 */
class LIAR_KERNEL_DLL SceneLightGlobal: public SceneLight
{
	PY_HEADER(SceneLight)
public:
	void setSceneBound(const TSphere3D& bound);
private:
	LASS_UTIL_VISITOR_DO_ACCEPT;
	virtual void doSetSceneBound(const TSphere3D& bound) = 0;
};

}

}

#endif

// EOF
