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
#include "flip.h"
#include <lass/util/bit_manip.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(Flip,
	"Flips reflection and transmission. The z component of omegaOut is\n"
	"negated, so that reflective materials become transparent and the\n"
	"other way around.\n"
	"Flip(child)"
	)
PY_CLASS_CONSTRUCTOR_1(Flip, const TShaderRef&)
PY_CLASS_MEMBER_RW(Flip, child, setChild)

// --- public --------------------------------------------------------------------------------------

Flip::Flip(const TShaderRef& child):
	Shader(Bsdf::flip(child->caps())),
	child_(child)
{
}



const TShaderRef& Flip::child() const
{
	return child_;
}



void Flip::setChild(const TShaderRef& child)
{
	child_ = child;
	setCaps(Bsdf::flip(child->caps()));
}



// --- protected -----------------------------------------------------------------------------------




// --- private -------------------------------------------------------------------------------------

void Flip::doShadeContext(const Sample& sample, IntersectionContext& context) const
{
	return child_->shadeContext(sample, context);
}



const Spectral Flip::doEmission(const Sample& sample, const IntersectionContext& context, const TVector3D& omegaOut) const
{
	return child_->emission(sample, context, Bsdf::flip(omegaOut));
}



TBsdfPtr Flip::doBsdf(const Sample& sample, const IntersectionContext& context) const
{
	return TBsdfPtr(new Bsdf(sample, context, caps(), child_->bsdf(sample, context)));
}



void Flip::doRequestSamples(const TSamplerPtr& sampler)
{
	child_->requestSamples(sampler);
}



size_t Flip::doNumReflectionSamples() const
{
	return child_->numTransmissionSamples();
}


size_t Flip::doNumTransmissionSamples() const
{
	return child_->numReflectionSamples();
}


const TPyObjectPtr Flip::doGetState() const
{
	return python::makeTuple(child_);
}



void Flip::doSetState(const TPyObjectPtr& state)
{
	TShaderRef child;
	python::decodeTuple(state, child);
	setChild(child);
}



// --- bsdf ----------------------------------------------------------------------------------------

Flip::Bsdf::Bsdf(
		const Sample& sample, const IntersectionContext& context, BsdfCaps caps, TBsdfPtr&& child):
	kernel::Bsdf(sample, context, caps), // caps are already flipped.
	child_(std::forward<TBsdfPtr>(child))
{
}



BsdfCaps Flip::Bsdf::flip(BsdfCaps caps)
{
	BsdfCaps out = caps;
	util::clearMasked<BsdfCaps>(out, BsdfCaps::reflection | BsdfCaps::transmission);
	util::setMaskedIf<BsdfCaps>(out, BsdfCaps::reflection, util::checkMaskedAll<BsdfCaps>(caps, BsdfCaps::transmission));
	util::setMaskedIf<BsdfCaps>(out, BsdfCaps::transmission, util::checkMaskedAll<BsdfCaps>(caps, BsdfCaps::reflection));
	return out;
}



TVector3D Flip::Bsdf::flip(const TVector3D& omega)
{
	return TVector3D(omega.x, omega.y, -omega.z);
}



BsdfOut Flip::Bsdf::doEvaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, BsdfCaps allowedCaps) const
{
	return child_->evaluate(omegaIn, flip(omegaOut), flip(allowedCaps));
}



SampleBsdfOut Flip::Bsdf::doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, BsdfCaps allowedCaps) const
{
	SampleBsdfOut out = child_->sample(omegaIn, sample, componentSample, flip(allowedCaps));
	out.omegaOut = flip(out.omegaOut);
	out.usedCaps = flip(out.usedCaps);
	return out;
}




// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
