/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2021  Bram de Greve (bramz@users.sourceforge.net)
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
#include "mirror.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(Mirror, "perfect mirror shader")
PY_CLASS_CONSTRUCTOR_0(Mirror)
PY_CLASS_CONSTRUCTOR_1(Mirror, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(Mirror, reflectance, setReflectance, "texture for reflectance component")
PY_CLASS_MEMBER_RW_DOC(Mirror, fuzz, setFuzz, "fuzziness factor for reflection direction")

// --- public --------------------------------------------------------------------------------------

Mirror::Mirror():
	Shader(BsdfCaps::reflection | BsdfCaps::specular),
	reflectance_(Texture::white()),
	fuzz_(nullptr)
{
}



Mirror::Mirror(const TTexturePtr& reflectance):
	Shader(BsdfCaps::reflection | BsdfCaps::specular),
	reflectance_(reflectance),
	fuzz_(nullptr)
{
}



const TTexturePtr& Mirror::reflectance() const
{
	return reflectance_;
}



void Mirror::setReflectance(const TTexturePtr& reflectance)
{
	reflectance_ = reflectance;
}



const TTexturePtr& Mirror::fuzz() const
{
	return fuzz_;
}



void Mirror::setFuzz(const TTexturePtr& fuzz)
{
	fuzz_ = fuzz;
}


// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

size_t Mirror::doNumReflectionSamples() const
{
	return 1;
}



TBsdfPtr Mirror::doBsdf(const Sample& sample, const IntersectionContext& context) const
{
	return TBsdfPtr(new Bsdf(
		sample,
		context,
		reflectance_->lookUp(sample, context, Reflectant),
		fuzz_ ? fuzz_->scalarLookUp(sample, context) : 0,
		BsdfCaps::reflection | (fuzz_ ? BsdfCaps::specular : BsdfCaps::specular)));
}



const TPyObjectPtr Mirror::doGetState() const
{
	return python::makeTuple(reflectance_, fuzz_);
}



void Mirror::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, reflectance_, fuzz_);
}


// --- Bsdf ----------------------------------------------------------------------------------------

Mirror::Bsdf::Bsdf(const Sample& sample, const IntersectionContext& context, const Spectral& reflectance, TScalar fuzz, BsdfCaps caps):
	kernel::Bsdf(sample, context, caps),
	reflectance_(reflectance),
	fuzz_(fuzz)
{
}



BsdfOut Mirror::Bsdf::doEvaluate(const TVector3D&, const TVector3D&, BsdfCaps) const
{
	return BsdfOut();
}



SampleBsdfOut Mirror::Bsdf::doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar, BsdfCaps LASS_UNUSED(allowedCaps)) const
{
	LASS_ASSERT(omegaIn.z > 0);
	LASS_ASSERT(kernel::hasCaps(allowedCaps, caps()));
	TVector3D omegaOut(-omegaIn.x, -omegaIn.y, omegaIn.z);
	if (fuzz_ > 0)
	{
		TScalar pf;
		omegaOut += fuzz_ * num::uniformSphere(sample, pf).position();
		// we ignore the pdf of the fuzz factor because we don't distribute the power
		// over the fuzzy directions (i.e. we don't scale down the reflectance).
		// in other words, we put all the energy in this one sample
		return SampleBsdfOut(omegaOut, omegaOut.z > 0 ? reflectance_ : Spectral(), omegaOut.z, caps());
	}
	else
	{
		// pdf is in fact 1, but then we need to divide reflectance_ by omegaOut.z
		return SampleBsdfOut(omegaOut, reflectance_, omegaOut.z, caps());
	}
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
