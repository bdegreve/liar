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

/** @class liar::SceneLight
 *  @brief base class of all light emiting scene objects
 *  @author Bram de Greve [BdG]
 */

#pragma once
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

	const Spectrum sampleEmission(const Sample& iCameraSample, const TVector2D& iLightSample, 
		const TPoint3D& iDestination, BoundedRay& oShadowRay, TScalar& oPdf) const 
	{ 
		return doSampleEmission(iCameraSample, iLightSample, iDestination, oShadowRay, oPdf);
	}

	const unsigned numberOfEmissionSamples() const 
	{ 
		return doNumberOfEmissionSamples(); 
	}

	const bool isShadowless() const { return isShadowless_; }
	void setShadowless(bool iIsShadowless) { isShadowless_ = iIsShadowless; }

protected:

    SceneLight(PyTypeObject* iType);

private:
    
	LASS_UTIL_ACCEPT_VISITOR
	
	virtual const Spectrum doSampleEmission(const Sample& iSample, const TVector2D& iLightSample,
		const TPoint3D& iDestination, BoundedRay& oShadowRay, TScalar& oPdf) const = 0;
	virtual const unsigned doNumberOfEmissionSamples() const = 0;

	bool isShadowless_;
};

typedef python::PyObjectPtr<SceneLight>::Type TSceneLightPtr;


}

}

#endif

// EOF
