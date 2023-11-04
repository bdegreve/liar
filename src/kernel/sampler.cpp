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


// --- public --------------------------------------------------------------------------------------

TSamplerPtr& Sampler::defaultSampler()
{
	return defaultSampler_;
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


}

}

// EOF
