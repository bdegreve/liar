/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2005  Bram de Greve
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
PY_CLASS_METHOD(Sampler, seed);

TSamplerPtr Sampler::defaultSampler_;

// --- public --------------------------------------------------------------------------------------

TSamplerPtr& Sampler::defaultSampler()
{
    return defaultSampler_;
}



// --- protected -----------------------------------------------------------------------------------

Sampler::Sampler(PyTypeObject* iType):
    python::PyObjectPlus(iType),
	totalSubSequenceSize1D_(0),
	totalSubSequenceSize2D_(0)
{
	subSequenceOffset1D_.push_back(0);
	subSequenceOffset2D_.push_back(0);
}



const int Sampler::requestSubSequence1D(unsigned iRequestedSize)
{
	const unsigned size = doRoundSize1D(iRequestedSize);
	totalSubSequenceSize1D_ += size;
	subSequenceSize1D_.push_back(size);
	subSequenceOffset1D_.push_back(subSequenceOffset1D_.back() + size);

	LASS_ASSERT(static_cast<int>(subSequenceSize1D_.size()) - 1 >= 0);
	return static_cast<int>(subSequenceSize1D_.size()) - 1;
}



const int Sampler::requestSubSequence2D(unsigned iRequestedSize)
{
	const unsigned size = doRoundSize2D(iRequestedSize);
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


void Sampler::sample(const TResolution& iPixel, unsigned iSubPixel, const TimePeriod& iPeriod, 
					 Sample& oSample)
{
	oSample.sampler_ = this;

	doSampleScreen(iPixel, iSubPixel, oSample.screenCoordinate_);
	doSampleLens(iPixel, iSubPixel, oSample.lensCoordinate_);
	doSampleTime(iPixel, iSubPixel, iPeriod, oSample.time_);

	oSample.subSequences1D_.resize(totalSubSequenceSize1D_);
	const size_t n1D = subSequenceSize1D_.size();
	for (size_t k = 0; k < n1D; ++k)
	{
		const unsigned offset = subSequenceOffset1D_[k];
		const unsigned size = subSequenceSize1D_[k];
		doSampleSubSequence1D(iPixel, iSubPixel, &oSample.subSequences1D_[offset],
			&oSample.subSequences1D_[offset + size]);
	}

	oSample.subSequences2D_.resize(totalSubSequenceSize2D_);
	const size_t n2D = subSequenceSize2D_.size();
	for (size_t k = 0; k < n2D; ++k)
	{
		const unsigned offset = subSequenceOffset2D_[k];
		const unsigned size = subSequenceSize2D_[k];
		doSampleSubSequence2D(iPixel, iSubPixel, &oSample.subSequences2D_[offset],
			&oSample.subSequences2D_[offset + size]);
	}
}



// --- private -------------------------------------------------------------------------------------

const unsigned Sampler::doRoundSize1D(unsigned iRequestedSize) const
{
	return iRequestedSize;
}



const unsigned Sampler::doRoundSize2D(unsigned iRequestedSize) const
{
	return iRequestedSize;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF