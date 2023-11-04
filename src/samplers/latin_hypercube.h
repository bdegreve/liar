/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2021  Bram de Greve (bramz@users.sourceforge.net)
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
#include "../kernel/sampler_tiled.h"
#include <random>

namespace liar
{
namespace samplers
{

class LIAR_SAMPLERS_DLL LatinHypercube : public SamplerTiled
{
	PY_HEADER(SamplerTiled)
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

	typedef std::mt19937 TNumberGenerator;
	typedef std::uniform_real_distribution<TScalar> TJitterGenerator;

	void init(const TResolution2D& resolution = TResolution2D(320, 240), size_t numberOfSamples = 1);

	const TResolution2D& doResolution() const override;
	size_t doSamplesPerPixel() const override;
	void doSetResolution(const TResolution2D& resolution) override;
	void doSetSamplesPerPixel(size_t samplesPerPixel) override;
	void doSeed(TSeed randomSeed) override;

	void doSampleScreen(const TResolution2D& pixel, size_t subPixel, TSample2D& screenCoordinate) override;
	void doSampleLens(const TResolution2D& pixel, size_t subPixel, TSample2D& lensCoordinate) override;
	void doSampleTime(const TResolution2D& pixel, size_t subPixel, const TimePeriod& period, TTime& time) override;
	void doSampleWavelength(const TResolution2D& pixel, size_t subPixel, TWavelength& wavelength, TScalar& pdf) override;
	void doSampleSubSequence1D(const TResolution2D& pixel, size_t subPixel, TSubSequenceId id, TSample1D* first, TSample1D* last) override;
	void doSampleSubSequence2D(const TResolution2D& pixel, size_t subPixel, TSubSequenceId id, TSample2D* first, TSample2D* last) override;

	const TSamplerPtr doClone() const override;

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

	TSample1D sampleStratum(size_t subPixel, TStrata& strata);
	TScalar jitter(TNumberGenerator& rng) { return isJittered_ ? jitter_(rng) : 0.5f; }

	TNumberGenerator rng_;
	TJitterGenerator jitter_;

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
