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

#include "shaders_common.h"
#include "lambert.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(Lambert, "perfect lambert shader")
PY_CLASS_CONSTRUCTOR_0(Lambert)
PY_CLASS_CONSTRUCTOR_1(Lambert, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(Lambert, diffuse, setDiffuse, "texture for diffuse component")

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

TBsdfPtr Lambert::doBsdf(const Sample& sample, const IntersectionContext& context) const
{
	const XYZ diffuseOverPi = diffuse_->lookUp(sample, context) / TNumTraits::pi;
	return TBsdfPtr(new LambertBsdf(sample, context, diffuseOverPi));
}



const TPyObjectPtr Lambert::doGetState() const
{
	return python::makeTuple(diffuse_);
}



void Lambert::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, diffuse_);
}


// --- bsdf ----------------------------------------------------------------------------------------

LambertBsdf::LambertBsdf(const Sample& sample, const IntersectionContext& context, const XYZ& diffuseOverPi):
	Bsdf(sample, context),
	diffuseOverPi_(diffuseOverPi)
{
}

BsdfOut LambertBsdf::doCall(const TVector3D& omegaIn, const TVector3D& omegaOut, unsigned allowedCaps) const
{
	const TScalar cosTheta = omegaOut.z;
	if (testCaps(allowedCaps, Shader::capsReflection | Shader::capsDiffuse) && cosTheta > 0)
	{
		return BsdfOut(diffuseOverPi_, cosTheta / TNumTraits::pi);
	}
	return BsdfOut();
}

SampleBsdfOut LambertBsdf::doSample(const TVector3D& omegaIn, const TPoint2D& sample, unsigned allowedCaps) const
{
	SampleBsdfOut out;
	if (testCaps(allowedCaps, Shader::capsReflection | Shader::capsDiffuse))
	{
		out.omegaOut = num::cosineHemisphere(sample, out.pdf).position();
		out.value = diffuseOverPi_;
		out.usedCaps = Shader::capsReflection | Shader::capsDiffuse;
	}
	return out;
}



// --- free ----------------------------------------------------------------------------------------




}

}

// EOF
