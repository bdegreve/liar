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

	const Spectrum sampleEmission(const Sample& cameraSample, const TPoint2D& lightSample, 
		const TPoint3D& target, const TVector3D& targetNormal, BoundedRay& shadowRay,
		TScalar& pdf) const
	{ 
		return doSampleEmission(
			cameraSample, lightSample, target, targetNormal, shadowRay, pdf);
	}
	const Spectrum sampleEmission(const TPoint2D& sampleA, const TPoint2D& sampleB,
		const TPoint3D& sceneCenter, TScalar sceneRadius, TRay3D& emissionRay, TScalar& pdf) const
	{
		return doSampleEmission(sampleA, sampleB, sceneCenter, sceneRadius, emissionRay, pdf);
	}
	const Spectrum totalPower(TScalar sceneRadius) const
	{
		return doTotalPower(sceneRadius);
	}
	const unsigned numberOfEmissionSamples() const 
	{ 
		return doNumberOfEmissionSamples(); 
	}

	const bool isShadowless() const { return isShadowless_; }
	void setShadowless(bool iIsShadowless) { isShadowless_ = iIsShadowless; }

protected:

    SceneLight();

private:
    
	LASS_UTIL_ACCEPT_VISITOR

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);
	
	virtual const Spectrum doSampleEmission(const Sample& sample, const TPoint2D& lightSample,
		const TPoint3D& target, const TVector3D& targetNormal, BoundedRay& shadowRay, 
		TScalar& pdf) const = 0;
	virtual const Spectrum doSampleEmission(const TPoint2D& sampleA, const TPoint2D& sampleB,
		const TPoint3D& sceneCenter, TScalar sceneRadius, TRay3D& emissionRay, TScalar& pdf) const = 0;
	virtual const Spectrum doTotalPower(TScalar sceneRadius) const = 0;
	virtual const unsigned doNumberOfEmissionSamples() const = 0;
	
	virtual const TPyObjectPtr doGetLightState() const = 0;
	virtual void doSetLightState(const TPyObjectPtr& state) = 0;

	bool isShadowless_;
};

typedef python::PyObjectPtr<SceneLight>::Type TSceneLightPtr;


}

}

#endif

// EOF
