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
#include "linear_interpolator.h"
#include <lass/stde/extended_iterator.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(LinearInterpolator, "interpolates shaders using gray value of control texture as parameter")
PY_CLASS_CONSTRUCTOR_0(LinearInterpolator)
PY_CLASS_CONSTRUCTOR_2(LinearInterpolator, const LinearInterpolator::TKeyShaders&, const TTextureRef&)
PY_CLASS_MEMBER_RW(LinearInterpolator, keys, setKeys)
PY_CLASS_MEMBER_RW(LinearInterpolator, control, setControl)
PY_CLASS_METHOD(LinearInterpolator, addKey)

// --- public --------------------------------------------------------------------------------------

LinearInterpolator::LinearInterpolator():
	Shader(BsdfCaps::none),
	keys_(),
	control_(Texture::black())
{
}



LinearInterpolator::LinearInterpolator(const TKeyShaders& keyShaders, const TTextureRef& controlTexture):
	Shader(BsdfCaps::none),
	keys_(),
	control_(controlTexture)
{
	setKeys(keyShaders);
}



/** return list of key shaders
 */
const LinearInterpolator::TKeyShaders& LinearInterpolator::keys() const
{
	return keys_;
}



/** return control texture
 */
const TTextureRef& LinearInterpolator::control() const
{
	return control_;
}



/** set list of key shaders
 */
void LinearInterpolator::setKeys(const TKeyShaders& keyShaders)
{
	keys_ = keyShaders;
	std::sort(keys_.begin(), keys_.end(), LesserKey());

	BsdfCaps caps = BsdfCaps::none;
	for (TKeyShaders::const_iterator i = keys_.begin(); i != keys_.end(); ++i)
	{
		caps |= i->second->caps();
	}
	setCaps(caps);
}



/** set control texture
 */
void LinearInterpolator::setControl(const TTextureRef& contolTexture)
{
	control_ = contolTexture;
}



/** add a key texture to the list
 */
void LinearInterpolator::addKey(const TValue keyValue, const TShaderRef& keyShader)
{
	TKeyShader key(keyValue, keyShader);
	TKeyShaders::iterator i = std::lower_bound(keys_.begin(), keys_.end(), key, LesserKey());
	keys_.insert(i, key);

	setCaps(this->caps() | keyShader->caps());
}



// --- protected -----------------------------------------------------------------------------------




// --- private -------------------------------------------------------------------------------------

const Spectral LinearInterpolator::doEmission(const Sample& sample, const IntersectionContext& context, const TVector3D& omegaOut) const
{
	if (keys_.empty())
	{
		return Spectral();
	}

	const TValue controlValue = control_->scalarLookUp(sample, context);
	TKeyShaders::const_iterator i = std::lower_bound(keys_.begin(), keys_.end(), controlValue,
		[](const TKeyShader& key, TValue x) { return key.first < x; });

	if (i == keys_.begin())
	{
		LASS_ASSERT(controlValue <= keys_.front().first);
		return keys_.front().second->emission(sample, context, omegaOut);
	}
	if (i == keys_.end())
	{
		LASS_ASSERT(controlValue >= keys_.back().first);
		return keys_.back().second->emission(sample, context, omegaOut);
	}

	const TKeyShaders::const_iterator prevI = stde::prev(i);
	LASS_ASSERT(prevI->first != i->first); // due to lower_bound

	const TValue t = (controlValue - prevI->first) / (i->first - prevI->first);

	const TVector3D omegaLocal = context.bsdfToLocal().transform(omegaOut);

	const TShaderRef& shaderA = prevI->second;
	IntersectionContext contextA = context;
	shaderA->shadeContext(sample, contextA);
	const TVector3D omegaA = contextA.localToBsdf().transform(omegaLocal);

	const TShaderRef& shaderB = i->second;
	IntersectionContext contextB = context;
	shaderB->shadeContext(sample, contextB);
	const TVector3D omegaB = contextB.localToBsdf().transform(omegaLocal);

	return lerp(
		shaderA->emission(sample, contextA, omegaA),
		shaderB->emission(sample, contextB, omegaB),
		t);
}


TBsdfPtr LinearInterpolator::doBsdf(const Sample& sample, const IntersectionContext& context) const
{
	if (keys_.empty())
	{
		return TBsdfPtr();
	}

	const TValue controlValue = control_->scalarLookUp(sample, context);
	TKeyShaders::const_iterator i = std::lower_bound(keys_.begin(), keys_.end(), controlValue,
		[](const TKeyShader& key, TValue x) { return key.first < x; });

	if (i == keys_.begin())
	{
		LASS_ASSERT(controlValue <= keys_.front().first);
		return keys_.front().second->bsdf(sample, context);
	}
	if (i == keys_.end())
	{
		LASS_ASSERT(controlValue >= keys_.back().first);
		return keys_.back().second->bsdf(sample, context);
	}

	const TKeyShaders::const_iterator prevI = stde::prev(i);
	LASS_ASSERT(prevI->first != i->first); // due to lower_bound

	const TValue t = (controlValue - prevI->first) / (i->first - prevI->first);

	const TShaderRef& shaderA = prevI->second;
	IntersectionContext contextA = context;
	contextA.setShader(shaderA);
	shaderA->shadeContext(sample, contextA);

	const TShaderRef& shaderB = i->second;
	IntersectionContext contextB = context;
	contextB.setShader(shaderB);
	shaderB->shadeContext(sample, contextB);

	return TBsdfPtr(new Bsdf(sample, context, caps(), contextA, contextB, t));
}



void LinearInterpolator::doRequestSamples(const TSamplerPtr& sampler)
{
	for (TKeyShaders::const_iterator i = keys_.begin(); i != keys_.end(); ++i)
	{
		i->second->requestSamples(sampler);
	}
}



size_t LinearInterpolator::doNumReflectionSamples() const
{
	size_t n = 0;
	for (TKeyShaders::const_iterator i = keys_.begin(); i != keys_.end(); ++i)
	{
		n = std::max(n, i->second->numReflectionSamples());
	}
	return n;
}



size_t LinearInterpolator::doNumTransmissionSamples() const
{
	size_t n = 0;
	for (TKeyShaders::const_iterator i = keys_.begin(); i != keys_.end(); ++i)
	{
		n = std::max(n, i->second->numTransmissionSamples());
	}
	return n;
}



const TPyObjectPtr LinearInterpolator::doGetState() const
{
	return python::makeTuple(keys_, control_);
}



void LinearInterpolator::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, keys_, control_);
}



// --- bsdf ----------------------------------------------------------------------------------------

LinearInterpolator::Bsdf::Bsdf(
		const Sample& sample, const IntersectionContext& context, BsdfCaps caps,
		const IntersectionContext& a, const IntersectionContext& b, TValue t) :
	kernel::Bsdf(sample, context, caps),
	a_(a),
	b_(b),
	t_(t)
{
	LASS_ASSERT(t >= 0 && t <= 1);
}


namespace
{

TVector3D bsdfToBsdf(const IntersectionContext& from, const IntersectionContext& to, const TVector3D& omega)
{
	return to.worldToBsdf(from.bsdfToWorld(omega));
}

}


BsdfOut LinearInterpolator::Bsdf::doEvaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, BsdfCaps allowedCaps) const
{
	// LinearInterpolator does not shade context, but a_ and b_ might.
	// so you need to transform omegaIn and omegaOut back to world and then to bsdf again

	// FIXME: get rid of the cosTheta correction
	// * static_cast<TValue>(num::abs(wOut.z / omegaOut.z));
	LIAR_ASSERT(num::abs(omegaOut.z) >= TNumTraits::minStrictPositive, "Cannot evaluate with very small omegaOut.z: " << omegaOut);

	TScalar pa = a_.bsdf()->compatibleCaps(allowedCaps) ? (1 - t_) : 0;
	TScalar pb = b_.bsdf()->compatibleCaps(allowedCaps) ? t_ : 0;
	LASS_ASSERT(pa >= 0 && pb >= 0);
	const TScalar ptot = pa + pb;

	BsdfOut out;
	if (pa > 0)
	{
		const TVector3D wIn = bsdfToBsdf(context(), a_, omegaIn);
		if (wIn.z > 0)
		{
			const TVector3D wOut = bsdfToBsdf(context(), a_, omegaOut);
			out = a_.bsdf()->evaluate(wIn, wOut, allowedCaps);
			out.value *= (1 - t_) * static_cast<TValue>(num::abs(wOut.z / omegaOut.z));
			out.pdf *= pa / ptot;
		}
	}
	if (pb > 0)
	{
		const TVector3D wIn = bsdfToBsdf(context(), b_, omegaIn);
		if (wIn.z > 0)
		{
			const TVector3D wOut = bsdfToBsdf(context(), b_, omegaOut);
			const BsdfOut outB = b_.bsdf()->evaluate(wIn, wOut, allowedCaps);
			out.value.fma(outB.value, t_ * static_cast<TValue>(num::abs(wOut.z / omegaOut.z)));
			out.pdf += outB.pdf * pb / ptot;
		}
	}
	return out;
}



SampleBsdfOut LinearInterpolator::Bsdf::doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, BsdfCaps allowedCaps) const
{
	// LinearInterpolator does not shade context, but a_ and b_ might.
	// so you need to transform omegaIn and omegaOut back to world and then to bsdf again

	const IntersectionContext* a = &a_;
	const IntersectionContext* b = &b_;
	TValue ta = 1 - t_;
	TValue tb = t_;
	TValue pdfA = a->bsdf()->compatibleCaps(allowedCaps) ? ta : 0;
	TValue pdfB = b->bsdf()->compatibleCaps(allowedCaps) ? tb : 0;
	LASS_ASSERT(pdfA >= 0 && pdfB >= 0);
	const TValue pdfTot = pdfA + pdfB;
	if (pdfTot <= 0)
	{
		return SampleBsdfOut();
	}
	pdfA /= pdfTot;
	pdfB /= pdfTot;

	if (componentSample >= pdfA)
	{
		componentSample -= pdfA;
		std::swap(a, b);
		std::swap(ta, tb);
		std::swap(pdfA, pdfB);
	}
	LIAR_ASSERT(pdfA > 0, pdfA);
	constexpr TScalar maxSample = TNumTraits::one - TNumTraits::epsilon / 2; // largest scalar under 1
	static_assert(maxSample < TNumTraits::one);
	const TScalar compSampleA = std::min(componentSample / pdfA, maxSample);
	LIAR_ASSERT(compSampleA < 1, "componentSample=" << componentSample << ", pdfA=" << pdfA << ", compSampleA=" << compSampleA);

	const TVector3D wInA = bsdfToBsdf(context(), *a, omegaIn);
	if (wInA.z <= 0)
	{
		return SampleBsdfOut();
	}
	SampleBsdfOut out = a->bsdf()->sample(wInA, sample, compSampleA, allowedCaps);
	if (out.pdf == 0)
	{
		return SampleBsdfOut();
	}
	const TVector3D wOutA = out.omegaOut;
	out.omegaOut = bsdfToBsdf(*a, context(), wOutA);
	if (out.omegaOut.z == 0)
	{
		return SampleBsdfOut();
	}
	out.value *= ta * static_cast<TValue>(num::abs(wOutA.z / out.omegaOut.z));
	out.pdf *= pdfA;

	const TVector3D wInB = bsdfToBsdf(context(), *b, omegaIn);
	if (wInB.z > 0)
	{
		const TVector3D wOutB = bsdfToBsdf(context(), *b, out.omegaOut);
		BsdfOut outB = b->bsdf()->evaluate(wInB, wOutB, out.usedCaps);
		if (outB.pdf > 0)
		{
			out.value.fma(outB.value, tb * static_cast<TValue>(num::abs(wOutB.z / out.omegaOut.z)));
			out.pdf += outB.pdf * pdfB;
		}
	}

	return out;
}




// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
