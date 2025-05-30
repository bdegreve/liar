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

/** @class liar::Sampler
 *  @brief generates samples to be feed to the ray tracer.
 *  @author Bram de Greve [Bramz]
 *
 *  @warning
 *  THE INTERFACE OF THE SAMPLER IS STILL PRELIMINARY.  The main problem is how to avoid that two
 *  engines use the same sampler and thus screwing each other.  Or if they do use the same sampler,
 *  how can we manage they don't screw each other.
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SAMPLER_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SAMPLER_H

#include "kernel_common.h"
#include "time_period.h"
#include <lass/prim/vector_2d.h>

namespace liar
{
namespace kernel
{

class Sample;
class Sampler;

typedef python::PyObjectPtr<Sampler>::Type TSamplerPtr;
typedef PyObjectRef<Sampler> TSamplerRef;

class LIAR_KERNEL_DLL Sampler: public python::PyObjectPlus
{
	PY_HEADER(python::PyObjectPlus)
public:

	typedef int TSubSequenceId;
	typedef num::Tuint32 TSeed;
	typedef prim::Aabb2D<TScalar> TBucket;

	class LIAR_KERNEL_DLL Task
	{
	public:
		virtual ~Task();
		size_t id() const;
		bool drawSample(Sampler& sampler, const TimePeriod& period, Sample& sample);
	protected:
		Task(size_t id);
	private:
		virtual bool doDrawSample(Sampler& sampler, const TimePeriod& period, Sample& sample) = 0;
		size_t id_;
	};
	typedef util::SharedPtr<Task> TTaskPtr;

	const TBucket& bucket() const { return bucket_; }
	void setBucket(const TBucket& bucket) { bucket_ = bucket; }

	TSubSequenceId requestSubSequence1D(size_t requestedSize);
	TSubSequenceId requestSubSequence2D(size_t requestedSize);
	size_t numSubSequences1D() const;
	size_t numSubSequences2D() const;
	size_t subSequenceSize1D(TSubSequenceId id) const { return id >= 0 ? subSequenceSize1D_[static_cast<size_t>(id)] : 0; }
	size_t subSequenceSize2D(TSubSequenceId id) const { return id >= 0 ? subSequenceSize2D_[static_cast<size_t>(id)] : 0; }
	void clearSubSequenceRequests();

	void seed(TSeed randomSeed) { doSeed(randomSeed); }

	TTaskPtr getTask();

	static TSamplerPtr& defaultSampler();

	const TSamplerPtr clone() const;

	const TPyObjectPtr reduce() const;
	const TPyObjectPtr getState() const;
	void setState(const TPyObjectPtr& state);

protected:

	typedef TScalar TSample1D;
	typedef TPoint2D TSample2D;

	Sampler();

private:

	friend class Sample;
	friend class SamplerTiled; // until we cleared up the mess.

	typedef std::vector<size_t> TSubSequenceSizes;

	virtual TTaskPtr doGetTask() = 0;
	virtual void doSeed(TSeed randomSeed) = 0;

	virtual size_t doRoundSize2D(size_t requestedSize) const;

	virtual const TSamplerPtr doClone() const = 0;

	virtual const TPyObjectPtr doGetState() const = 0;
	virtual void doSetState(const TPyObjectPtr& state) = 0;

	size_t subSequenceOffset1D(TSubSequenceId id) const { return subSequenceOffset1D_[static_cast<size_t>(id)]; }
	size_t subSequenceOffset2D(TSubSequenceId id) const { return subSequenceOffset2D_[static_cast<size_t>(id)]; }

	static TSamplerPtr defaultSampler_;

	TSubSequenceSizes subSequenceSize1D_;
	TSubSequenceSizes subSequenceSize2D_;
	TSubSequenceSizes subSequenceOffset1D_;
	TSubSequenceSizes subSequenceOffset2D_;
	size_t totalSubSequenceSize1D_;
	size_t totalSubSequenceSize2D_;
	TBucket bucket_;
};


}

}

#endif

// EOF
