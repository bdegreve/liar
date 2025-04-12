/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2025  Bram de Greve (bramz@users.sourceforge.net)
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
PY_CLASS_CONSTRUCTOR_1(Lambert, const TTextureRef&)
PY_CLASS_MEMBER_RW_DOC(Lambert, diffuse, setDiffuse, "texture for diffuse component")

// --- public --------------------------------------------------------------------------------------

Lambert::Lambert():
	Shader(BsdfCaps::reflection | BsdfCaps::diffuse),
	diffuse_(Texture::white())
{
}



Lambert::Lambert(const TTextureRef& diffuse):
	Shader(BsdfCaps::reflection | BsdfCaps::diffuse),
	diffuse_(diffuse)
{
}



const TTextureRef& Lambert::diffuse() const
{
	return diffuse_;
}



void Lambert::setDiffuse(const TTextureRef& diffuse)
{
	diffuse_ = diffuse;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

TBsdfPtr Lambert::doBsdf(const Sample& sample, const IntersectionContext& context) const
{
	const Spectral diffuse = diffuse_->lookUp(sample, context, SpectralType::Reflectant);
	return TBsdfPtr(new LambertBsdf(sample, context, caps(), diffuse));
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

LambertBsdf::LambertBsdf(const Sample& sample, const IntersectionContext& context, BsdfCaps caps, const Spectral& diffuse) :
	Bsdf(sample, context, caps),
	diffuseOverPi_(diffuse / num::NumTraits<Spectral::TValue>::pi)
{
}

BsdfOut LambertBsdf::doEvaluate(const TVector3D&, const TVector3D& omegaOut, BsdfCaps LASS_UNUSED(allowedCaps)) const
{
	LASS_ASSERT(shaders::hasCaps(allowedCaps, caps()));
	const TScalar cosTheta = num::abs(omegaOut.z);
	if (cosTheta <= 0)
	{
		return BsdfOut();
	}
	return BsdfOut(diffuseOverPi_, cosTheta / TNumTraits::pi);
}

SampleBsdfOut LambertBsdf::doSample(const TVector3D&, const TPoint2D& sample, TScalar, BsdfCaps LASS_UNUSED(allowedCaps)) const
{
	LASS_ASSERT(shaders::hasCaps(allowedCaps, caps()));
	SampleBsdfOut out;
	out.omegaOut = num::cosineHemisphere(sample, out.pdf).position();
	out.value = diffuseOverPi_;
	out.usedCaps = caps();
	return out;
}



// --- free ----------------------------------------------------------------------------------------




}

}

// EOF
