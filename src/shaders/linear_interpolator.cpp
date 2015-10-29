/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
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
PY_CLASS_CONSTRUCTOR_2(LinearInterpolator, const LinearInterpolator::TKeyShaders&, const TTexturePtr&)
PY_CLASS_MEMBER_RW(LinearInterpolator, keys, setKeys)
PY_CLASS_MEMBER_RW(LinearInterpolator, control, setControl)
PY_CLASS_METHOD(LinearInterpolator, addKey)

// --- public --------------------------------------------------------------------------------------

LinearInterpolator::LinearInterpolator():
	Shader(Bsdf::capsNone),
	keys_(),
	control_(Texture::black())
{
}



LinearInterpolator::LinearInterpolator(const TKeyShaders& keyShaders, const TTexturePtr& controlTexture):
	Shader(Bsdf::capsNone),
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
const TTexturePtr& LinearInterpolator::control() const
{
	return control_;
}



/** set list of key shaders
 */
void LinearInterpolator::setKeys(const TKeyShaders& keyShaders)
{
	keys_ = keyShaders;
	std::sort(keys_.begin(), keys_.end(), LesserKey());

	TBsdfCaps caps = 0;
	for (TKeyShaders::const_iterator i = keys_.begin(); i != keys_.end(); ++i)
	{
		caps |= i->second->caps();
	}
	setCaps(caps);
}



/** set control texture
 */
void LinearInterpolator::setControl(const TTexturePtr& contolTexture)
{
	control_ = contolTexture;
}



/** add a key texture to the list
 */
void LinearInterpolator::addKey(const TScalar keyValue, const TShaderPtr& keyShader)
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

	const TScalar keyValue = average(control_->lookUp(sample, context));
	TKeyShader sentinel(keyValue, TShaderPtr());
	TKeyShaders::const_iterator i = std::lower_bound(keys_.begin(), keys_.end(), sentinel, LesserKey());

	if (i == keys_.begin())
	{
		return keys_.front().second->emission(sample, context, omegaOut);
	}
	if (i == keys_.end())
	{
		return keys_.back().second->emission(sample, context, omegaOut);
	}

	const TKeyShaders::const_iterator prevI = stde::prev(i);
	LASS_ASSERT(prevI->first != i->first); // due to lower_bound

	const TScalar t = (keyValue - prevI->first) / (i->first - prevI->first);

	return lerp(
		prevI->second->emission(sample, context, omegaOut),
		i->second->emission(sample, context, omegaOut),
		t);
}



TBsdfPtr LinearInterpolator::doBsdf(const Sample& sample, const IntersectionContext& context) const
{
	if (keys_.empty())
	{
		return TBsdfPtr();
	}

	const TScalar keyValue = average(control_->lookUp(sample, context));
	TKeyShader sentinel(keyValue, TShaderPtr());
	TKeyShaders::const_iterator i = std::lower_bound(keys_.begin(), keys_.end(), sentinel, LesserKey());

	if (i == keys_.begin())
	{
		return keys_.front().second->bsdf(sample, context);
	}
	if (i == keys_.end())
	{
		return keys_.back().second->bsdf(sample, context);
	}

	const TKeyShaders::const_iterator prevI = stde::prev(i);
	LASS_ASSERT(prevI->first != i->first); // due to lower_bound

	const TBsdfPtr a = prevI->second->bsdf(sample, context);
	const TBsdfPtr b = i->second->bsdf(sample, context);
	const TScalar t = (keyValue - prevI->first) / (i->first - prevI->first);

	return TBsdfPtr(new Bsdf(sample, context, caps(), a, b, t));
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
		const Sample& sample, const IntersectionContext& context, TBsdfCaps caps,
		const TBsdfPtr& a, const TBsdfPtr& b, TScalar t):
	kernel::Bsdf(sample, context, caps),
	a_(a),
	b_(b),
	t_(t)
{
	LASS_ASSERT(t >= 0 && t <= 1);
}



BsdfOut LinearInterpolator::Bsdf::doEvaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, TBsdfCaps allowedCaps) const
{
	TScalar pa = a_->compatibleCaps(allowedCaps) ? (1 - t_) : 0;
	TScalar pb = b_->compatibleCaps(allowedCaps) ? t_ : 0;
	LASS_ASSERT(pa >= 0 && pb >= 0);
	const TScalar ptot = pa + pb;

	BsdfOut out;
	if (pa > 0)
	{
		out = a_->evaluate(omegaIn, omegaOut, allowedCaps);
		out.value *= (1 - t_);
		out.pdf *= pa / ptot;
	}
	if (pb > 0)
	{
		const BsdfOut outB = b_->evaluate(omegaIn, omegaOut, allowedCaps);
		out.value += outB.value * t_;
		out.pdf += outB.pdf * pa / ptot;
	}
	return out;
}



SampleBsdfOut LinearInterpolator::Bsdf::doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, TBsdfCaps allowedCaps) const
{
	TScalar pa = a_->compatibleCaps(allowedCaps) ? (1 - t_) : 0;
	TScalar pb = b_->compatibleCaps(allowedCaps) ? t_ : 0;
	LASS_ASSERT(pa >= 0 && pb >= 0);
	const TScalar ptot = pa + pb;
	if (ptot <= 0)
	{
		return SampleBsdfOut();
	}
	pa /= ptot;
	pb /= ptot;

	SampleBsdfOut out;
	if (componentSample < pa)
	{
		out = a_->sample(omegaIn, sample, componentSample, allowedCaps);
		out.value *= (1 - t_);
		out.pdf *= pa;
	}
	else
	{
		LASS_ASSERT(pb > 0);
		out = a_->sample(omegaIn, sample, componentSample, allowedCaps);
		out.value *= t_;
		out.pdf *= pb;
	}
	return out;

}




// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
