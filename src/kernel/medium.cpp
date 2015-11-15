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

#include "kernel_common.h"
#include "medium.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(Medium, "Volumetric shader")

// --- public --------------------------------------------------------------------------------------

Medium::~Medium()
{
}



const Spectral& Medium::refractionIndex() const
{
	return refractionIndex_;
}



void Medium::setRefractionIndex(const Spectral& refractionIndex)
{
	refractionIndex_ = refractionIndex;
}



size_t Medium::priority() const
{
	return priority_;
}



void Medium::setPriority(size_t priority)
{
	priority_ = priority;
}



void Medium::requestSamples(const TSamplerPtr& sampler)
{
	const size_t n = numScatterSamples();
	if (idStepSamples_ < 0 && n > 0)
	{
		idStepSamples_ = sampler->requestSubSequence1D(n);
		idLightSamples_ = sampler->requestSubSequence1D(n);
		idSurfaceSamples_ = sampler->requestSubSequence2D(n);
	}
	doRequestSamples(sampler);
}



// --- protected -----------------------------------------------------------------------------------

Medium::Medium():
	refractionIndex_(1),
	priority_(0),
	idStepSamples_(-1),
	idLightSamples_(-1),
	idSurfaceSamples_(-1)
{
}



// --- private -------------------------------------------------------------------------------------

void Medium::doRequestSamples(const TSamplerPtr&)
{
}



size_t Medium::doNumScatterSamples() const
{
	return 0;
}



// --- free ----------------------------------------------------------------------------------------



// -------------------------------------------------------------------------------------------------

MediumStack::MediumStack(const TMediumPtr& defaultMedium):
	stack_(),
	default_(defaultMedium)
{
}



const Spectral MediumStack::transmittance(const Sample& sample, const BoundedRay& ray) const
{
	return medium() ? medium()->transmittance(sample, ray) : Spectral(1);
}



const Spectral MediumStack::transmittance(const Sample& sample, const BoundedRay& ray, TScalar farLimit) const
{
	return transmittance(sample, bound(ray, ray.nearLimit(), farLimit));
}



const Spectral MediumStack::transmittance(const Sample& sample, const DifferentialRay& ray, TScalar farLimit) const
{
	return transmittance(sample, ray.centralRay(), farLimit);
}



const Spectral MediumStack::emission(const Sample& sample, const BoundedRay& ray) const
{
	return medium() ? medium()->emission(sample, ray) : Spectral(0);
}



const Spectral MediumStack::sampleScatterOut(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const
{
	if (!medium())
	{
		pdf = 0;
		return Spectral(0);
	}
	return medium()->sampleScatterOut(sample, ray, tScatter, pdf);
}



const Spectral MediumStack::sampleScatterOutOrTransmittance(const Sample& sample, TScalar scatterSample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const
{
	if (!medium())
	{
		tScatter = ray.farLimit();
		pdf = 1;
		return Spectral(1);
	}
	return medium()->sampleScatterOutOrTransmittance(sample, scatterSample, ray, tScatter, pdf);
}



const Spectral MediumStack::samplePhase(const Sample& sample, const TPoint2D& phaseSample, const TPoint3D& position, const TVector3D& dirIn, TVector3D& dirOut, TScalar& pdf) const
{
	if (!medium())
	{
		pdf = 0;
		dirOut = TVector3D();
		return Spectral(0);
	}
	return medium()->samplePhase(sample, phaseSample, position, dirIn, dirOut, pdf);
}

/*
void MediumStack::push(const Medium* medium)
{
	struct LessPriority
	{
		bool operator()(const Medium* a, const Medium* b) const
		{
			return a->priority() < b->priority();
		}
	};
	TStack::iterator i = std::upper_bound(stack_.begin(), stack_.end(), medium);
	stack_.insert(i, medium);
}
*/
/*
void MediumStack::pop(const Medium*)
{
}
*/


const Medium* MediumStack::medium() const
{
	return stack_.empty() ? default_.get() : stack_.back();
}



// -------------------------------------------------------------------------------------------------

MediumChanger::MediumChanger(MediumStack& stack, const Medium* medium, SolidEvent solidEvent):
	stack_(stack.stack_),
	medium_(medium),
	event_(solidEvent)
{
	if (medium_)
	{
		switch (event_)
		{
		case seEntering:
			stack_.push_back(medium_);
			break;
		case seLeaving:
			if (stack_.empty())
			{
				// our first generation view ray is leaving an object.  Regard this as a nop.
				event_ = seNoEvent;
			}
			else
			{
				LASS_ASSERT(stack_.back() == medium_);
				stack_.pop_back();
			}
			break;
		default:
			break;
		}
	}
}



MediumChanger::~MediumChanger()
{
	if (medium_)
	{
		switch (event_)
		{
		case seEntering:
			LASS_ASSERT(!stack_.empty() && stack_.back() == medium_);
			stack_.pop_back();
			break;
		case seLeaving:
			stack_.push_back(medium_);
			break;
		default:
			break;
		};
	}
}

}

}

// EOF
