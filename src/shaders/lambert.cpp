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

#include "shaders_common.h"
#include "lambert.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS(Lambert)
PY_CLASS_CONSTRUCTOR_0(Lambert)
PY_CLASS_CONSTRUCTOR_1(Lambert, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(Lambert, "diffuse", diffuse, setDiffuse, "texture for diffuse component")

// --- public --------------------------------------------------------------------------------------

Lambert::Lambert():
	Shader(capsReflection | capsDiffuse),
	diffuse_(Texture::white())
{
}



Lambert::Lambert(const TTexturePtr& iDiffuse):
	Shader(capsReflection | capsDiffuse),
	diffuse_(iDiffuse)
{
}



const TTexturePtr& Lambert::diffuse() const
{
	return diffuse_;
}



void Lambert::setDiffuse(const TTexturePtr& iDiffuse)
{
	diffuse_ = iDiffuse;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void Lambert::doBsdf(
		const Sample& sample, const IntersectionContext& context, const TVector3D& omegaIn,
		const BsdfIn* first, const BsdfIn* last, BsdfOut* result) const
{
	LASS_ASSERT(omegaIn.z > 0);
	const Spectrum diffuseOverPi = diffuse_->lookUp(sample, context) / TNumTraits::pi;
	while (first != last)
	{
		const TScalar cosTheta = first->omegaOut.z;
		if (testCaps(first->allowedCaps, caps()) && cosTheta > 0)
		{
			result->value = diffuseOverPi;
			result->pdf = cosTheta / TNumTraits::pi;
		}
		++first;
		++result;
	}
}


void Lambert::doSampleBsdf(
		const Sample& sample, const IntersectionContext& context, const TVector3D& omegaIn,
		const SampleBsdfIn* first, const SampleBsdfIn* last, SampleBsdfOut* result) const
{
	LASS_ASSERT(omegaIn.z > 0);
	const Spectrum diffuseOverPi = diffuse_->lookUp(sample, context) / TNumTraits::pi;
	while (first != last)
	{
		if (testCaps(first->allowedCaps, caps()))
		{
			result->omegaOut = num::cosineHemisphere(first->sample, result->pdf).position();
			result->value = diffuseOverPi;
			result->usedCaps = caps();
		}
		++first;
		++result;
	}
}



const TPyObjectPtr Lambert::doGetState() const
{
	return python::makeTuple(diffuse_);
}



void Lambert::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, diffuse_);
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
