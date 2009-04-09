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

class LIAR_KERNEL_DLL Sampler: public python::PyObjectPlus
{
    PY_HEADER(python::PyObjectPlus)
public:

    const TResolution2D& resolution() const { return doResolution(); }
    const size_t samplesPerPixel() const { return doSamplesPerPixel(); }
    void setResolution(const TResolution2D& resolution) { doSetResolution(resolution); }
    void setSamplesPerPixel(size_t samplesPerPixel) { doSetSamplesPerPixel(samplesPerPixel); }
	void seed(size_t randomSeed) { doSeed(randomSeed); }

    const int requestSubSequence1D(size_t requestedSize);
    const int requestSubSequence2D(size_t requestedSize);
	void clearSubSequenceRequests();

    void sample(const TResolution2D& pixel, size_t subPixel, const TimePeriod& period, 
		Sample& sample);
	void sample(const TimePeriod& period, Sample& sample);

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

	typedef std::vector<size_t> TSubSequenceSizes;

    virtual const TResolution2D& doResolution() const = 0;
    virtual const size_t doSamplesPerPixel() const = 0;
    virtual void doSetResolution(const TResolution2D& resolution) = 0;
    virtual void doSetSamplesPerPixel(size_t samplesPerPixel) = 0;
	virtual void doSeed(size_t randomSeed) = 0;
    
	virtual void doSampleScreen(const TResolution2D& pixel, size_t subPixel, 
		TSample2D& oScreenCoordinate) = 0;
	virtual void doSampleLens(const TResolution2D& pixel, size_t subPixel, 
		TSample2D& oLensCoordinate) = 0;
	virtual void doSampleTime(const TResolution2D& pixel, size_t subPixel, 
		const TimePeriod& period, TTime& oTime) = 0;
	virtual void doSampleSubSequence1D(const TResolution2D& pixel, size_t subPixel, 
		TSample1D* oBegin, TSample1D* oEnd) = 0;
	virtual void doSampleSubSequence2D(const TResolution2D& pixel, size_t subPixel, 
		TSample2D* oBegin, TSample2D* oEnd) = 0;

	virtual const size_t doRoundSize1D(size_t requestedSize) const;
	virtual const size_t doRoundSize2D(size_t requestedSize) const;

	virtual const TSamplerPtr doClone() const = 0;

	virtual const TPyObjectPtr doGetState() const = 0;
	virtual void doSetState(const TPyObjectPtr& state) = 0;

	const size_t subSequenceSize1D(int id) const { return subSequenceSize1D_[id]; }
	const size_t subSequenceSize2D(int id) const { return subSequenceSize2D_[id]; }
	const size_t subSequenceOffset1D(int id) const { return subSequenceOffset1D_[id]; }
	const size_t subSequenceOffset2D(int id) const { return subSequenceOffset2D_[id]; }

    static TSamplerPtr defaultSampler_;

	TSubSequenceSizes subSequenceSize1D_;
	TSubSequenceSizes subSequenceSize2D_;
	TSubSequenceSizes subSequenceOffset1D_;
	TSubSequenceSizes subSequenceOffset2D_;
	size_t totalSubSequenceSize1D_;
	size_t totalSubSequenceSize2D_;
};

typedef python::PyObjectPtr<Sampler>::Type TSamplerPtr;

}

}

#endif

// EOF

