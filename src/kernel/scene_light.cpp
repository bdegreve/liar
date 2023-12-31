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

#include "kernel_common.h"
#include "scene_light.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(SceneLight, "Abstract base class of scene lights")
PY_CLASS_MEMBER_RW_DOC(SceneLight, isShadowless, setShadowless,
	"True or False\n"
	"determines if light can be blocked to cause shadows.  By default it's false which "
	"means shadows will be casted\n")

PY_DECLARE_CLASS_DOC(SceneLightGlobal, "Abstract base class of global scene light")

// --- public --------------------------------------------------------------------------------------





// --- protected -----------------------------------------------------------------------------------

SceneLight::SceneLight():
	isShadowless_(false)
{
}


// --- private -------------------------------------------------------------------------------------

const SceneLight* SceneLight::doAsLight() const
{
	return this;
}



const TPyObjectPtr SceneLight::doGetState() const
{
	return python::makeTuple(isShadowless_, doGetLightState());
}



void SceneLight::doSetState(const TPyObjectPtr& state)
{
	TPyObjectPtr lightState;
	python::decodeTuple(state, isShadowless_, lightState);
	doSetLightState(lightState);
}



const Spectral SceneLight::doSampleEmission(
		const Sample& sample, const TPoint2D& lightSample, const TPoint3D& target, const TVector3D&,
		BoundedRay& shadowRay, TScalar& pdf) const
{
	// the default implementation is a fallback on the sampling mechanism without knowledge the target normal.
	return this->sampleEmission(sample, lightSample, target, shadowRay, pdf);
}



// --- free ----------------------------------------------------------------------------------------



// --- SceneLightGlobal ----------------------------------------------------------------------------

void SceneLightGlobal::setSceneBound(const TSphere3D& bound)
{
	doSetSceneBound(bound);
}

}

}

// EOF
