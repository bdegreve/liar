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
 *  http://liar.bramz.org
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

	const Spectrum emission(const Sample& sample, const TRay3D& ray,
		BoundedRay& shadowRay, TScalar& pdf) const
	{
		return doEmission(sample, ray, shadowRay, pdf);
	}
	const Spectrum sampleEmission(const Sample& cameraSample, const TPoint2D& lightSample, 
		const TPoint3D& target, const TVector3D& targetNormal, 
		BoundedRay& shadowRay, TScalar& pdf) const
	{ 
		return doSampleEmission(
			cameraSample, lightSample, target, targetNormal, shadowRay, pdf);
	}
	const Spectrum sampleEmission(const Sample& cameraSample, 
		const TPoint2D& lightSampleA, const TPoint2D& lightSampleB, const TAabb3D& sceneBound, 
		BoundedRay& emissionRay, TScalar& pdf) const
	{
		return doSampleEmission(
			cameraSample, lightSampleA, lightSampleB, sceneBound, emissionRay, pdf);
	}
	const Spectrum totalPower(const TAabb3D& sceneBound) const
	{
		return doTotalPower(sceneBound);
	}
	const unsigned numberOfEmissionSamples() const 
	{ 
		return doNumberOfEmissionSamples(); 
	}
	const bool isSingular() const
	{
		return doIsSingular();
	}

	const bool isShadowless() const { return isShadowless_; }
	void setShadowless(bool iIsShadowless) { isShadowless_ = iIsShadowless; }


protected:

	SceneLight();

private:

	LASS_UTIL_VISITOR_DO_ACCEPT;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);
	
	virtual const Spectrum doEmission(const Sample& sample, const TRay3D& ray,
		BoundedRay& shadowRay, TScalar& pdf) const = 0;
	virtual const Spectrum doSampleEmission(const Sample& sample, const TPoint2D& lightSample,
		const TPoint3D& target, const TVector3D& targetNormal, 
		BoundedRay& shadowRay, TScalar& pdf) const = 0;
	virtual const Spectrum doSampleEmission(const Sample& cameraSample, 
		const TPoint2D& lightSampleA, const TPoint2D& lightSampleB, const TAabb3D& sceneBound, 
		BoundedRay& emissionRay, TScalar& pdf) const = 0;
	virtual const Spectrum doTotalPower(const TAabb3D& sceneBound) const = 0;
	virtual const unsigned doNumberOfEmissionSamples() const = 0;
	virtual const bool doIsSingular() const = 0;
	
	virtual const TPyObjectPtr doGetLightState() const = 0;
	virtual void doSetLightState(const TPyObjectPtr& state) = 0;

	bool isShadowless_;
};

typedef python::PyObjectPtr<SceneLight>::Type TSceneLightPtr;


}

}

#endif

// EOF
