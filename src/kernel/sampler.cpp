/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2006  Bram de Greve
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  http://liar.sourceforge.net
 */

#include "kernel_common.h"
#include "sampler.h"
#include "sample.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS(Sampler)
PY_CLASS_MEMBER_RW(Sampler, "resolution", resolution, setResolution)
PY_CLASS_MEMBER_RW(Sampler, "samplesPerPixel", samplesPerPixel, setSamplesPerPixel)
PY_CLASS_METHOD(Sampler, seed)
PY_CLASS_METHOD_NAME(Sampler, reduce, "__reduce__")
PY_CLASS_METHOD_NAME(Sampler, getState, "__getstate__")
PY_CLASS_METHOD_NAME(Sampler, setState, "__setstate__")


TSamplerPtr Sampler::defaultSampler_;

// --- public --------------------------------------------------------------------------------------

TSamplerPtr& Sampler::defaultSampler()
{
    return defaultSampler_;
}



const int Sampler::requestSubSequence1D(unsigned requestedSize)
{
	if (requestedSize == 0)
	{
		return -1;
	}
	const unsigned size = doRoundSize1D(requestedSize);
	totalSubSequenceSize1D_ += size;
	subSequenceSize1D_.push_back(size);
	subSequenceOffset1D_.push_back(subSequenceOffset1D_.back() + size);

	LASS_ASSERT(static_cast<int>(subSequenceSize1D_.size()) - 1 >= 0);
	return static_cast<int>(subSequenceSize1D_.size()) - 1;
}



const int Sampler::requestSubSequence2D(unsigned requestedSize)
{
	if (requestedSize == 0)
	{
		return -1;
	}
	const unsigned size = doRoundSize2D(requestedSize);
	totalSubSequenceSize2D_ += size;
	subSequenceSize2D_.push_back(size);
	subSequenceOffset2D_.push_back(subSequenceOffset2D_.back() + size);

	LASS_ASSERT(static_cast<int>(subSequenceSize2D_.size()) - 1 >= 0);
	return static_cast<int>(subSequenceSize2D_.size()) - 1;
}



void Sampler::clearSubSequenceRequests()
{
	totalSubSequenceSize1D_ = 0;
	subSequenceSize1D_.clear();
	subSequenceOffset1D_.clear();
	subSequenceOffset1D_.push_back(0);

	totalSubSequenceSize2D_ = 0;
	subSequenceSize2D_.clear();
	subSequenceOffset2D_.clear();
	subSequenceOffset2D_.push_back(0);
}



void Sampler::sample(const TResolution& pixel, unsigned subPixel, const TimePeriod& period, 
					 Sample& sample)
{
	sample.sampler_ = this;

	doSampleScreen(pixel, subPixel, sample.screenCoordinate_);
	doSampleLens(pixel, subPixel, sample.lensCoordinate_);
	doSampleTime(pixel, subPixel, period, sample.time_);

	sample.subSequences1D_.resize(totalSubSequenceSize1D_);
	const size_t n1D = subSequenceSize1D_.size();
	for (size_t k = 0; k < n1D; ++k)
	{
		const unsigned offset = subSequenceOffset1D_[k];
		const unsigned size = subSequenceSize1D_[k];
		doSampleSubSequence1D(pixel, subPixel, &sample.subSequences1D_[offset],
			&sample.subSequences1D_[offset] + size);
	}

	sample.subSequences2D_.resize(totalSubSequenceSize2D_);
	const size_t n2D = subSequenceSize2D_.size();
	for (size_t k = 0; k < n2D; ++k)
	{
		const unsigned offset = subSequenceOffset2D_[k];
		const unsigned size = subSequenceSize2D_[k];
		doSampleSubSequence2D(pixel, subPixel, &sample.subSequences2D_[offset],
			&sample.subSequences2D_[offset] + size);
	}
}



void Sampler::sample(const TimePeriod& period, Sample& sample)
{
	// make up a pixel and subpixel =)
	TResolution pixel(0, 0);
	unsigned subPixel = 0;

	Sampler::sample(pixel, subPixel, period, sample);
}



const TSamplerPtr Sampler::clone() const
{
	const TSamplerPtr result = doClone();
	LASS_ASSERT(typeid(*result) == typeid(*this));
	return result;
}



const TPyObjectPtr Sampler::reduce() const
{
	return python::makeTuple(
		python::fromNakedToSharedPtrCast<PyObject>(reinterpret_cast<PyObject*>(this->GetType())), 
		python::makeTuple(), this->getState());
}



const TPyObjectPtr Sampler::getState() const
{
	return python::makeTuple(
		subSequenceSize1D_,
		subSequenceSize2D_,
		subSequenceOffset1D_,
		subSequenceOffset2D_,
		totalSubSequenceSize1D_,
		totalSubSequenceSize2D_,
		doGetState());
}



void Sampler::setState(const TPyObjectPtr& state)
{
	TPyObjectPtr derivedState;
	python::decodeTuple(
		state,
		subSequenceSize1D_,
		subSequenceSize2D_,
		subSequenceOffset1D_,
		subSequenceOffset2D_,
		totalSubSequenceSize1D_,
		totalSubSequenceSize2D_,
		derivedState);
	doSetState(derivedState);
}



// --- protected -----------------------------------------------------------------------------------

Sampler::Sampler():
	totalSubSequenceSize1D_(0),
	totalSubSequenceSize2D_(0)
{
	subSequenceOffset1D_.push_back(0);
	subSequenceOffset2D_.push_back(0);
}



// --- private -------------------------------------------------------------------------------------

const unsigned Sampler::doRoundSize1D(unsigned requestedSize) const
{
	return requestedSize;
}



const unsigned Sampler::doRoundSize2D(unsigned requestedSize) const
{
	return requestedSize;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
