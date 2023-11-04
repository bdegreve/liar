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

#include "samplers_common.h"
#include "latin_hypercube.h"
#include "../kernel/xyz.h"
#include "../kernel/observer.h"
#include <lass/stde/range_algorithm.h>
#include <lass/stde/access_iterator.h>

namespace liar
{
namespace samplers
{

PY_DECLARE_CLASS_DOC(LatinHypercube, "(jittered) stratified sampler");
PY_CLASS_CONSTRUCTOR_0(LatinHypercube)
PY_CLASS_CONSTRUCTOR_1(LatinHypercube, const TResolution2D&)
PY_CLASS_CONSTRUCTOR_2(LatinHypercube, const TResolution2D&, size_t)
PY_CLASS_MEMBER_RW(LatinHypercube, jittered, setJittered)

// --- public --------------------------------------------------------------------------------------

LatinHypercube::LatinHypercube():
	isJittered_(true)
{
	init();
}



LatinHypercube::LatinHypercube(const TResolution2D& resolution):
	isJittered_(true)
{
	init(resolution);
}



LatinHypercube::LatinHypercube(const TResolution2D& resolution, size_t numberOfSamplesPerPixel):
	isJittered_(true)
{
	init(resolution, numberOfSamplesPerPixel);
}



bool LatinHypercube::jittered() const
{
	return isJittered_;
}



void LatinHypercube::setJittered(bool enabled)
{
	isJittered_ = enabled;
}



// --- protected -----------------------------------------------------------------------------------




// --- private -------------------------------------------------------------------------------------

void LatinHypercube::init(const TResolution2D& resolution, size_t numberOfSamplesPerPixel)
{
	setResolution(resolution);
	setSamplesPerPixel(numberOfSamplesPerPixel);
}



const TResolution2D& LatinHypercube::doResolution() const
{
	return resolution_;
}



size_t LatinHypercube::doSamplesPerPixel() const
{
	return samplesPerPixel_;
}



void LatinHypercube::doSetResolution(const TResolution2D& resolution)
{
	LASS_ASSERT(resolution.x > 0 && resolution.y > 0);
	resolution_ = resolution;
	reciprocalResolution_ = TVector2D(resolution).reciprocal();
}



void LatinHypercube::doSetSamplesPerPixel(size_t samplesPerPixel)
{
	samplesPerPixel_ = samplesPerPixel;
	stratumSize_ = num::inv(static_cast<TScalar>(samplesPerPixel_));
	timeStrata_.resize(samplesPerPixel_);
	wavelengthStrata_.resize(samplesPerPixel_);
	screenStrataX_.resize(samplesPerPixel_);
	screenStrataY_.resize(samplesPerPixel_);
	lensStrataX_.resize(samplesPerPixel_);
	lensStrataY_.resize(samplesPerPixel_);
	subSequences2d_.clear();

	for (size_t i = 0; i < samplesPerPixel_; ++i)
	{
		TSample1D x = static_cast<TSample1D>(i);
		screenStrataX_[i] = x;
		screenStrataY_[i] = x;
		lensStrataX_[i] = x;
		lensStrataY_[i] = x;
		timeStrata_[i] = x;
		wavelengthStrata_[i] = x;
	}
}



void LatinHypercube::doSeed(TSeed randomSeed)
{
	rng_.seed(randomSeed);
}



void LatinHypercube::doSampleScreen(const TResolution2D& LASS_UNUSED(pixel), size_t subPixel, TSample2D& screenCoordinate)
{
	LASS_ASSERT(pixel.x < resolution_.x && pixel.y < resolution_.y);
	TVector2D position(sampleStratum(subPixel, screenStrataX_), sampleStratum(subPixel, screenStrataY_));
	position += TVector2D(pixel);
	position *= reciprocalResolution_;
	screenCoordinate = TSample2D(position);
}



void LatinHypercube::doSampleLens(const TResolution2D& LASS_UNUSED(pixel), size_t subPixel, TSample2D& lensCoordinate)
{
	LASS_ASSERT(pixel.x < resolution_.x && pixel.y < resolution_.y);
	lensCoordinate = TSample2D(sampleStratum(subPixel, lensStrataX_), sampleStratum(subPixel, lensStrataY_));
}



void LatinHypercube::doSampleTime(const TResolution2D& LASS_UNUSED(pixel), size_t subPixel, const TimePeriod& period, TTime& time)
{
	LASS_ASSERT(pixel.x < resolution_.x && pixel.y < resolution_.y);
	const TScalar tau = sampleStratum(subPixel, timeStrata_);
	time = period.interpolate(tau);
}



void LatinHypercube::doSampleWavelength(const TResolution2D& LASS_UNUSED(pixel), size_t subPixel, TWavelength& wavelength, TScalar& pdf)
{
	LASS_ASSERT(pixel.x < resolution_.x && pixel.y < resolution_.y);
	const TScalar sample = sampleStratum(subPixel, wavelengthStrata_);
	wavelength = standardObserver().sample(sample, pdf);
}



void LatinHypercube::doSampleSubSequence1D(const TResolution2D&, size_t subPixel, TSubSequenceId id, TSample1D* first, TSample1D* last)
{
	const size_t nSubPixels = this->samplesPerPixel();
	const size_t subSeqSize = this->subSequenceSize1D(id);
	const size_t size = nSubPixels * subSeqSize;

	const size_t i = num::numCast<size_t>(id);
	if (i >= subSequences1d_.size())
	{
		LASS_ENFORCE(subPixel == 0);
		subSequences1d_.resize(i + 1);
	}

	if (subPixel == 0)
	{
		subSequences1d_[i].resize(size);
		const TScalar scale = num::inv(static_cast<TScalar>(size));

		// generate interleaved samples: stratum1,subpixel1, stratum1,subpixel2, ... stratum2,subpixel1,stratum2,subpixel2
		TSubSequence1D::iterator p = subSequences1d_[i].begin();
		for (size_t k = 0; k < subSeqSize; ++k)
		{
			// sample one stratum for all subpixels
			TSubSequence1D::iterator start = p;
			for (size_t dk = 0; dk < nSubPixels; ++dk)
			{
				*p++ = (static_cast<TScalar>(k * nSubPixels + dk) + jitter(rng_)) * scale;
			}
			std::shuffle(start, p, rng_);
		}
	}

	// pick a subpixel worth of samples
	//
	LASS_ASSERT(last - first == static_cast<std::ptrdiff_t>(subSeqSize));
	const TSubSequence1D& subSequence = subSequences1d_[i];
	for (size_t k = 0; k < subSeqSize; ++k)
	{
		first[k] = subSequence[k * nSubPixels + subPixel];
	}
	std::shuffle(first, last, rng_); // to avoid inter-sequence coherence
}



void LatinHypercube::doSampleSubSequence2D(const TResolution2D& LASS_UNUSED(pixel), size_t subPixel, TSubSequenceId id, TSample2D* first, TSample2D* last)
{
	const size_t nSubPixels = this->samplesPerPixel();
	const size_t subSeqSize = this->subSequenceSize2D(id);
	const size_t size = nSubPixels * subSeqSize;

	const size_t i = num::numCast<size_t>(id);
	if (i >= subSequences2d_.size())
	{
		LASS_ENFORCE(subPixel == 0);
		subSequences2d_.resize(i + 1);
	}

	if (subPixel == 0)
	{
		subSequences2d_[i].resize(size);
		const TScalar scale = num::inv(static_cast<TScalar>(size));

		// generate interleaved samples: stratum1,subpixel1, stratum1,subpixel2, ... stratum2,subpixel1,stratum2,subpixel2
		TSubSequence2D::iterator p = subSequences2d_[i].begin();
		for (size_t k = 0; k < subSeqSize; ++k)
		{
			// sample one stratum for all subpixels, along the diagonal
			TSubSequence2D::iterator start = p;
			for (size_t dk = 0; dk < nSubPixels; ++dk)
			{
				p->x = (static_cast<TScalar>(k * nSubPixels + dk) + jitter(rng_)) * scale;
				p->y = (static_cast<TScalar>(k * nSubPixels + dk) + jitter(rng_)) * scale;
				++p;
			}

			// shuffle samples within stratum
			std::shuffle(stde::member_iterator(start, &TSample2D::x), stde::member_iterator(p, &TSample2D::x), rng_);
			std::shuffle(stde::member_iterator(start, &TSample2D::y), stde::member_iterator(p, &TSample2D::y), rng_);
		}
	}

	// pick a subpixel worth of samples
	//
	LASS_ASSERT(last - first == static_cast<std::ptrdiff_t>(subSeqSize));
	const TSubSequence2D& subSequence = subSequences2d_[i];
	for (size_t k = 0; k < subSeqSize; ++k)
	{
		first[k] = subSequence[k * nSubPixels + subPixel];
	}

	// and shuffle again. for a single sequence, it's sufficient to only shuffle the ys,
	// but to avoid inter-sequence coherence, we shuffle the xs too.
	//
	std::shuffle(stde::member_iterator(first, &TSample2D::x), stde::member_iterator(last, &TSample2D::x), rng_);
	std::shuffle(stde::member_iterator(first, &TSample2D::y), stde::member_iterator(last, &TSample2D::y), rng_);
}



const TSamplerPtr LatinHypercube::doClone() const
{
	return TSamplerPtr(new LatinHypercube(*this));
}



const TPyObjectPtr LatinHypercube::doGetState() const
{
	std::ostringstream rngState;
	LASS_ENFORCE_STREAM(rngState << rng_);
	return python::makeTuple(rngState.str(), resolution_, samplesPerPixel_, isJittered_);
}



void LatinHypercube::doSetState(const TPyObjectPtr& state)
{
	std::string rngState;
	TResolution2D resolution;
	size_t strataPerPixel;

	python::decodeTuple(state, rngState, resolution, strataPerPixel, isJittered_);

	std::istringstream stream(rngState);
	LASS_ENFORCE_STREAM(stream >> rng_);
	setResolution(resolution);
	setSamplesPerPixel(strataPerPixel);
}



LatinHypercube::TSample1D LatinHypercube::sampleStratum(size_t subPixel, TStrata& strata)
{
	LASS_ASSERT(subPixel < samplesPerPixel_);
	if (subPixel == 0)
	{
		std::shuffle(strata.begin(), strata.end(), rng_);
	}
	return (strata[subPixel] + jitter(rng_)) * stratumSize_;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
