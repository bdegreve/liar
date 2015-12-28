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
#include "sampler.h"
#include "sample.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(Sampler, "Abstract base class of samplers")
PY_CLASS_METHOD(Sampler, seed)
PY_CLASS_METHOD_NAME(Sampler, reduce, "__reduce__")
PY_CLASS_METHOD_NAME(Sampler, getState, "__getstate__")
PY_CLASS_METHOD_NAME(Sampler, setState, "__setstate__")


TSamplerPtr Sampler::defaultSampler_;
TSamplerProgressivePtr Sampler::defaultProgressiveSampler_;


// --- public --------------------------------------------------------------------------------------

TSamplerPtr& Sampler::defaultSampler()
{
	return defaultSampler_;
}



TSamplerProgressivePtr& Sampler::defaultProgressiveSampler()
{
	return defaultProgressiveSampler_;
}



Sampler::TTaskPtr Sampler::getTask()
{
	return doGetTask();
}



int Sampler::requestSubSequence1D(size_t requestedSize)
{
	if (requestedSize == 0)
	{
		return -1;
	}
	totalSubSequenceSize1D_ += requestedSize;
	subSequenceSize1D_.push_back(requestedSize);
	subSequenceOffset1D_.push_back(subSequenceOffset1D_.back() + requestedSize);
	const int id = static_cast<int>(subSequenceSize1D_.size()) - 1;
	LASS_ASSERT(id >= 0);

	return id;
}



int Sampler::requestSubSequence2D(size_t requestedSize)
{
	if (requestedSize == 0)
	{
		return -1;
	}
	const size_t size = doRoundSize2D(requestedSize);
	totalSubSequenceSize2D_ += size;
	subSequenceSize2D_.push_back(size);
	subSequenceOffset2D_.push_back(subSequenceOffset2D_.back() + size);
	const int id = static_cast<int>(subSequenceSize2D_.size()) - 1;
	LASS_ASSERT(id >= 0);

	if (size != requestedSize)
	{
		LASS_COUT << "Sampler warning: 2D subsequence " << id << ": using size " << size << " instead of " << requestedSize << std::endl;
	}

	return id;
}


size_t Sampler::numSubSequences1D() const
{
	return subSequenceSize1D_.size();
}


size_t Sampler::numSubSequences2D() const
{
	return subSequenceSize2D_.size();
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



const TSamplerPtr Sampler::clone() const
{
	const TSamplerPtr result = doClone();
	LASS_ASSERT(typeid(*result) == typeid(*this));
	return result;
}



const TPyObjectPtr Sampler::reduce() const
{
	return python::makeTuple(
		python::fromNakedToSharedPtrCast<PyObject>(reinterpret_cast<PyObject*>(this->_lassPyGetClassDef()->type())), 
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
	totalSubSequenceSize2D_(0),
	bucket_{ TPoint2D{ 0, 0 }, TPoint2D{ 1, 1 } }
{
	subSequenceOffset1D_.push_back(0);
	subSequenceOffset2D_.push_back(0);
}



// --- private -------------------------------------------------------------------------------------

size_t Sampler::doRoundSize2D(size_t requestedSize) const
{
	return requestedSize;
}



// --- free ----------------------------------------------------------------------------------------


Sampler::Task::~Task()
{
}


Sampler::Task::Task(size_t id):
	id_(id)
{
}


size_t Sampler::Task::id() const
{
	return id_;
}


bool Sampler::Task::drawSample(Sampler& sampler, const TimePeriod& period, Sample& sample)
{
	sample.sampler_ = &sampler; // must get rid of this some how ...
	sample.subSequences1D_.resize(sampler.totalSubSequenceSize1D_);
	sample.subSequences2D_.resize(sampler.totalSubSequenceSize2D_);
	return doDrawSample(sampler, period, sample);
}




// --- SamplerTileBased ----------------------------------------------------------------------------

PY_DECLARE_CLASS_DOC(SamplerTileBased, "Abstract base class of samplers that render a number of samples per pixel")
PY_CLASS_MEMBER_RW(SamplerTileBased, resolution, setResolution)
PY_CLASS_MEMBER_RW(SamplerTileBased, samplesPerPixel, setSamplesPerPixel)

SamplerTileBased::SamplerTileBased() :
	Sampler(),
	nextId_(0)
{
}


SamplerTileBased::TTaskPtr SamplerTileBased::doGetTask()
{
	const TResolution2D res = resolution();

	const TResolution2D begin(num::floor(bucket().min().x * res.x), num::floor(bucket().min().y * res.y));
	const TResolution2D end(num::floor(bucket().max().x * res.x), num::floor(bucket().max().y * res.y));

	const size_t tileSize = 16;
	const size_t n_x = (end.x - begin.x + tileSize - 1) / tileSize;
	const size_t n_y = (end.y - begin.y + tileSize - 1) / tileSize;

	const size_t id = nextId_++;
	if (id >= n_x * n_y)
	{
		return TTaskPtr(0);
	}
	const size_t i = id % n_x;
	const size_t j = id / n_x;
	
	const TResolution2D first(begin.x + i * tileSize, begin.y + j * tileSize);
	const TResolution2D last(std::min(first.x + tileSize, end.x), std::min(first.y + tileSize, end.y));
	return TTaskPtr(new TaskTileBased(id, first, last, this->samplesPerPixel()));
}



void SamplerTileBased::sample(const TResolution2D& pixel, size_t subPixel, const TimePeriod& period, Sample& sample)
{
	doSampleScreen(pixel, subPixel, sample.screenSample_);
	doSampleLens(pixel, subPixel, sample.lensSample_);
	doSampleTime(pixel, subPixel, period, sample.time_);
	doSampleWavelength(pixel, subPixel, sample.wavelength_, sample.wavelengthPdf_);

	const size_t n1D = subSequenceSize1D_.size();
	for (size_t k = 0; k < n1D; ++k)
	{
		const size_t offset = subSequenceOffset1D_[k];
		const size_t size = subSequenceSize1D_[k];
		doSampleSubSequence1D(pixel, subPixel, static_cast<TSubSequenceId>(k),
			&sample.subSequences1D_[offset], &sample.subSequences1D_[offset] + size);
	}

	const size_t n2D = subSequenceSize2D_.size();
	for (size_t k = 0; k < n2D; ++k)
	{
		const size_t offset = subSequenceOffset2D_[k];
		const size_t size = subSequenceSize2D_[k];
		doSampleSubSequence2D(pixel, subPixel, static_cast<TSubSequenceId>(k),
			&sample.subSequences2D_[offset], &sample.subSequences2D_[offset] + size);
	}
}


SamplerTileBased::TaskTileBased::TaskTileBased(size_t id, const TResolution2D& begin, const TResolution2D& end, size_t samplesPerPixel):
	Task(id),
	begin_(begin),
	end_(end),
	pixel_(begin),
	samplesPerPixel_(samplesPerPixel),
	subPixel_(0)
{
}


bool SamplerTileBased::TaskTileBased::doDrawSample(Sampler& sampler, const TimePeriod& period, Sample& sample)
{
	if (subPixel_ == samplesPerPixel_)
	{
		subPixel_ = 0;
		++pixel_.x;
		if (pixel_.x == end_.x)
		{
			pixel_.x = begin_.x;
			++pixel_.y;
			if (pixel_.y == end_.y)
			{
				return false;
			}
		}
	}
	static_cast<SamplerTileBased&>(sampler).sample(pixel_, subPixel_, period, sample);
	++subPixel_;
	return true;
}


// --- SamplerProgressive ----------------------------------------------------------------------------

PY_DECLARE_CLASS_DOC(SamplerProgressive, "Abstract base class of samplers that return an unbounded number of samples over all pixels")

SamplerProgressive::SamplerProgressive():
	Sampler()
{
}

}

}

// EOF
