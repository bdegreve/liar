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
#include "sum.h"

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(Sum, "sum of shaders")
PY_CLASS_CONSTRUCTOR_1(Sum, const Sum::TChildren&)

// --- public --------------------------------------------------------------------------------------

Sum::Sum(const TChildren& children):
	Shader(Bsdf::capsNone),
	children_(children)
{
	TBsdfCaps caps;
	for (TChildren::const_iterator i = children.begin(); i != children.end(); ++i)
	{
		caps |= (*i)->caps();
	}
	setCaps(caps);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const XYZ Sum::doEmission(const Sample& sample, const IntersectionContext& context, const TVector3D& omegaOut) const
{
	XYZ result;
	for (TChildren::const_iterator i = children_.begin(); i != children_.end(); ++i)
	{
		result += (*i)->emission(sample, context, omegaOut);
	}
	return result;
}



TBsdfPtr Sum::doBsdf(const Sample& sample, const IntersectionContext& context) const
{
	TBsdfPtr::Rebind<SumBsdf>::Type result(new SumBsdf(sample, context, caps()));
	for (TChildren::const_iterator i = children_.begin(); i != children_.end(); ++i)
	{
		result->components_.push_back((*i)->bsdf(sample, context));
	}
	return result;
}



void Sum::doRequestSamples(const TSamplerPtr& sampler)
{
	for (TChildren::const_iterator i = children_.begin(); i != children_.end(); ++i)
	{
		(*i)->requestSamples(sampler);
	}
}



size_t Sum::doNumReflectionSamples() const
{
	size_t n = 0;
	for (TChildren::const_iterator i = children_.begin(); i != children_.end(); ++i)
	{
		n = std::max(n, (*i)->numReflectionSamples());
	}
	return n;
}



size_t Sum::doNumTransmissionSamples() const
{
	size_t n = 0;
	for (TChildren::const_iterator i = children_.begin(); i != children_.end(); ++i)
	{
		n = std::max(n, (*i)->numTransmissionSamples());
	}
	return n;
}



const TPyObjectPtr Sum::doGetState() const
{
	return python::makeTuple(children_);
}



void Sum::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, children_);
}


// --- bsdf ----------------------------------------------------------------------------------------

Sum::SumBsdf::SumBsdf(const Sample& sample, const IntersectionContext& context, TBsdfCaps caps):
	Bsdf(sample, context, caps),
	activeCaps_(0)
{
}

BsdfOut Sum::SumBsdf::doEvaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, TBsdfCaps allowedCaps) const
{
	size_t usedComponents = 0;
	BsdfOut out;
	for (TComponents::const_iterator i = components_.begin(); i != components_.end(); ++i)
	{
		const Bsdf* bsdf = i->get();
		if (!bsdf)
		{
			continue;
		}
		if (!bsdf->compatibleCaps(allowedCaps))
		{
			continue;
		}
		out += bsdf->evaluate(omegaIn, omegaOut, allowedCaps);
		++usedComponents;
	}
	if (usedComponents)
	{
		out.pdf /= usedComponents;
	}
	return out;
}



SampleBsdfOut Sum::SumBsdf::doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, TBsdfCaps allowedCaps) const
{
	// this implementation is not thread safe, but that's OK, as a BSDF is not supposed to be shared between multiple threads.

	if (allowedCaps != activeCaps_)
	{
		activeComponents_.clear();
		for (TComponents::const_iterator i = components_.begin(); i != components_.end(); ++i)
		{
			const Bsdf* bsdf = i->get();
			if (!bsdf)
			{
				continue;
			}
			if (!bsdf->compatibleCaps(allowedCaps))
			{
				continue;
			}			
			activeComponents_.push_back(bsdf);
		}
		activeCaps_ = allowedCaps;
	}

	if (activeComponents_.empty())
	{
		return SampleBsdfOut();
	}

	const size_t n = activeComponents_.size();
	const size_t k = std::min(static_cast<size_t>(num::floor(n * componentSample)), n - 1);

	SampleBsdfOut out = activeComponents_[k]->sample(omegaIn, sample, componentSample, allowedCaps);
	out.pdf /= n;
	if (componentSample > 1)
	{
		//const BsdfOut bsdfOut = this->bsdf(omegaIn, out.omegaOut, out.usedCaps);
		//out.value = bsdfOut.value;
		//out.pdf = bsdfOut.pdf ???  no it isn't.
	}
	return out;
}



// --- free ----------------------------------------------------------------------------------------




}

}

// EOF
