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

TBsdfPtr Sum::doBsdf(const Sample& sample, const IntersectionContext& context) const
{
	SumBsdf::TComponents components;
	components.reserve(children_.size());
	for (TChildren::const_iterator i = children_.begin(); i != children_.end(); ++i)
	{
		components.push_back((*i)->bsdf(sample, context));
	}
	return TBsdfPtr(new SumBsdf(sample, context, caps(), components));
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

Sum::SumBsdf::SumBsdf(const Sample& sample, const IntersectionContext& context, unsigned caps, const TComponents& components):
	Bsdf(sample, context, caps),
	components_(components)
{
}

BsdfOut Sum::SumBsdf::doCall(const TVector3D& omegaIn, const TVector3D& omegaOut, unsigned allowedCaps) const
{
	size_t usedComponents = 0;
	BsdfOut out;
	for (TComponents::const_iterator i = components_.begin(); i != components_.end(); ++i)
	{
		const Bsdf* bsdf = i->get();
		if (!bsdf->compatibleCaps(allowedCaps))
		{
			continue;
		}
		out += bsdf->call(omegaIn, omegaOut, allowedCaps);
		++usedComponents;
	}
	if (usedComponents)
	{
		out.pdf /= usedComponents;
	}
	return out;
}

SampleBsdfOut Sum::SumBsdf::doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, unsigned allowedCaps) const
{
	if (allowedCaps != activeCaps_)
	{
		activeComponents_.clear();
		for (TComponents::const_iterator i = components_.begin(); i != components_.end(); ++i)
		{
			const Bsdf* bsdf = i->get();
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
	/*
	const BsdfOut bsdfOut = this->bsdf(omegaIn, out.omegaOut, allowedCaps);
	out.value = bsdfOut.value;
	out.pdf = bsdfOut.pdf;
	out.usedCaps = allowedCaps & caps();
	/*/
	out.pdf /= n;
	return out;
}



// --- free ----------------------------------------------------------------------------------------




}

}

// EOF
