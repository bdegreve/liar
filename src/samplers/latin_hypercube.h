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

/** @class liar::samplers::Stratifier
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SAMPLERS_LATIN_HYPERCUBE_H
#define LIAR_GUARDIAN_OF_INCLUSION_SAMPLERS_LATIN_HYPERCUBE_H

#include "samplers_common.h"
#include "../kernel/sampler.h"
#include <lass/num/random.h>
#include <lass/num/distribution.h>

namespace liar
{
namespace samplers
{

class LIAR_SAMPLERS_DLL LatinHypercube: public Sampler
{
	PY_HEADER(Sampler)
public:

	LatinHypercube();
	LatinHypercube(const TResolution2D& resolution, size_t numberOfSamplesPerPixel);
	LatinHypercube(const TResolution2D& resolution);

	bool jittered() const;
	void setJittered(bool enabled);

private:

	typedef std::vector<TSample1D> TStrata;

	typedef std::vector<TSample1D> TSubSequence1D;
	typedef std::vector<TSample2D> TSubSequence2D;
	typedef std::vector<TSubSequence1D> TSubSequence1DList;
	typedef std::vector<TSubSequence2D> TSubSequence2DList;

	typedef num::RandomMT19937 TNumberGenerator;
	typedef num::DistributionUniform<TScalar, TNumberGenerator, num::rtRightOpen> TJitterGenerator;

	void init(const TResolution2D& resolution = TResolution2D(320, 240), size_t numberOfSamples = 1);

	virtual const TResolution2D& doResolution() const;
	virtual size_t doSamplesPerPixel() const;
	virtual void doSetResolution(const TResolution2D& resolution);
	virtual void doSetSamplesPerPixel(size_t samplesPerPixel);
	virtual void doSeed(TSeed randomSeed);

	virtual void doSampleScreen(const TResolution2D& pixel, size_t subPixel, TSample2D& screenCoordinate);
	virtual void doSampleLens(const TResolution2D& pixel, size_t subPixel, TSample2D& lensCoordinate);
	virtual void doSampleTime(const TResolution2D& pixel, size_t subPixel, const TimePeriod& period, TTime& time);
	virtual void doSampleWavelength(const TResolution2D& pixel, size_t subPixel, TScalar& wavelength);
	virtual void doSampleSubSequence1D(const TResolution2D& pixel, size_t subPixel, TSubSequenceId id, TSample1D* first, TSample1D* last);
	virtual void doSampleSubSequence2D(const TResolution2D& pixel, size_t subPixel, TSubSequenceId id, TSample2D* first, TSample2D* last);

	virtual size_t doRoundSize1D(size_t requestedSize) const;
	virtual size_t doRoundSize2D(size_t requestedSize) const;

	virtual const TSamplerPtr doClone() const;

	virtual const TPyObjectPtr doGetState() const;
	virtual void doSetState(const TPyObjectPtr& state);

	TSample1D sampleStratum(size_t subPixel, TStrata& strata);

	TNumberGenerator numberGenerator_;
	TJitterGenerator jitterGenerator_;

	TResolution2D resolution_;
	TVector2D reciprocalResolution_;
	TScalar stratumSize_;
	TStrata screenStrataX_;
	TStrata screenStrataY_;
	TStrata lensStrataX_;
	TStrata lensStrataY_;
	TStrata timeStrata_;
	TStrata wavelengthStrata_;
	TSubSequence1DList subSequences1d_;
	TSubSequence2DList subSequences2d_;
	size_t samplesPerPixel_;
	bool isJittered_;
};



}

}

#endif

// EOF
