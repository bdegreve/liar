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

#include "samplers_common.h"
#include "stratifier.h"
#include "../kernel/xyz.h"
#include "../kernel/observer.h"
#include <lass/stde/range_algorithm.h>

namespace liar
{
namespace samplers
{

PY_DECLARE_CLASS_DOC(Stratifier, "(jittered) stratified sampler");
PY_CLASS_CONSTRUCTOR_0(Stratifier)
PY_CLASS_CONSTRUCTOR_1(Stratifier, const TResolution2D&)
PY_CLASS_CONSTRUCTOR_2(Stratifier, const TResolution2D&, size_t)
PY_CLASS_MEMBER_RW(Stratifier, jittered, setJittered)

// --- public --------------------------------------------------------------------------------------

Stratifier::Stratifier():
	isJittered_(true)
{
	init();
}



Stratifier::Stratifier(const TResolution2D& resolution):
	isJittered_(true)
{
	init(resolution);
}



Stratifier::Stratifier(const TResolution2D& resolution, size_t numberOfSamplesPerPixel):
	isJittered_(true)
{
	init(resolution, numberOfSamplesPerPixel);
}



bool Stratifier::jittered() const
{
	return isJittered_;
}



void Stratifier::setJittered(bool enabled)
{
	isJittered_ = enabled;
}



// --- protected -----------------------------------------------------------------------------------




// --- private -------------------------------------------------------------------------------------

void Stratifier::init(const TResolution2D& resolution, size_t numberOfSamplesPerPixel)
{
	setResolution(resolution);
	setSamplesPerPixel(numberOfSamplesPerPixel);
}



const TResolution2D& Stratifier::doResolution() const
{
	return resolution_;
}



size_t Stratifier::doSamplesPerPixel() const
{
	return strataPerPixel_;
}



void Stratifier::doSetResolution(const TResolution2D& resolution)
{
	LASS_ASSERT(resolution.x > 0 && resolution.y > 0);
	resolution_ = resolution;
	reciprocalResolution_ = TVector2D(resolution).reciprocal();
}



void Stratifier::doSetSamplesPerPixel(size_t samplesPerPixel)
{
	const size_t strataPerAxis = static_cast<size_t>(num::round(num::sqrt(static_cast<TScalar>(samplesPerPixel))));
	if (strataPerAxis * strataPerAxis != samplesPerPixel)
	{
		LASS_COUT << "Sampler warning: samples per pixel: using size " << strataPerAxis * strataPerAxis
			<< " instead of " << samplesPerPixel << std::endl;
	}

	strataPerPixel_ = strataPerAxis * strataPerAxis;
	stratum1DSize_ = num::inv(static_cast<TScalar>(strataPerPixel_));
	stratum2DSize_ = TVector2D(TNumTraits::one, TNumTraits::one) / static_cast<TScalar>(strataPerAxis);
	timeStrata_.resize(strataPerPixel_);
	wavelengthStrata_.resize(strataPerPixel_);
	screenStrata_.resize(strataPerPixel_);
	lensStrata_.resize(strataPerPixel_);
	subSequences2d_.clear();

	for (size_t i = 0; i < strataPerPixel_; ++i)
	{
		timeStrata_[i] = static_cast<TSample1D>(i);
		wavelengthStrata_[i] = static_cast<TSample1D>(i);
	}

	for (size_t j = 0; j < strataPerAxis; ++j)
	{
		for (size_t i = 0; i < strataPerAxis; ++i)
		{
			screenStrata_[j * strataPerAxis + i] = TSample2D(i, j);
			lensStrata_[j * strataPerAxis + i] = TSample2D(i, j);
		}
	}
}



void Stratifier::doSeed(TSeed randomSeed)
{
	std::seed_seq seq{ randomSeed };
	rng_.seed(seq);
}



void Stratifier::doSampleScreen(const TResolution2D& LASS_UNUSED(pixel), size_t subPixel, TSample2D& screenCoordinate)
{
	LASS_ASSERT(pixel.x < resolution_.x && pixel.y < resolution_.y);
	TVector2D position = sampleStratum(subPixel, screenStrata_).position();
	position += TVector2D(pixel);
	position *= reciprocalResolution_;
	screenCoordinate = TSample2D(position);
}



void Stratifier::doSampleLens(const TResolution2D& LASS_UNUSED(pixel), size_t subPixel, TSample2D& lensCoordinate)
{
	LASS_ASSERT(pixel.x < resolution_.x && pixel.y < resolution_.y);
	lensCoordinate = sampleStratum(subPixel, lensStrata_);
}



void Stratifier::doSampleTime(const TResolution2D& LASS_UNUSED(pixel), size_t subPixel, const TimePeriod& period, TTime& time)
{
	LASS_ASSERT(pixel.x < resolution_.x && pixel.y < resolution_.y);
	const TScalar tau = sampleStratum(subPixel, timeStrata_);
	time = period.interpolate(tau);
}



void Stratifier::doSampleWavelength(const TResolution2D& LASS_UNUSED(pixel), size_t subPixel, TWavelength& wavelength, TScalar& pdf)
{
	LASS_ASSERT(pixel.x < resolution_.x && pixel.y < resolution_.y);
	const TScalar sample = sampleStratum(subPixel, wavelengthStrata_);
	wavelength = standardObserver().sample(sample, pdf);
}



void Stratifier::doSampleSubSequence1D(const TResolution2D&, size_t subPixel, TSubSequenceId id, TSample1D* first, TSample1D* last)
{
	const size_t nSubPixels = strataPerPixel_;
	const size_t subSeqSize = this->subSequenceSize1D(id);
	const size_t size = nSubPixels * subSeqSize;

	LIAR_ASSERT(id >= 0, "subsequence id must be non-negative: id=" << id);
	const size_t i = static_cast<size_t>(id);
	if (i >= subSequences1d_.size())
	{
		LIAR_ASSERT(subPixel == 0, "subpixel must be zero for new subsequence: subPixel=" << subPixel);
		subSequences1d_.resize(i + 1);
	}
	TSubSequence1D& subSequence = subSequences1d_[i];

	if (subPixel == 0)
	{
		subSequence.resize(size);
		const TScalar scale = num::inv(static_cast<TScalar>(size));

		// generate interleaved samples: stratum1,subpixel1, stratum1,subpixel2, ... stratum2,subpixel1,stratum2,subpixel2
		TSubSequence1D::iterator p = subSequence.begin();
		for (size_t k = 0; k < subSeqSize; ++k)
		{
			const TScalar k0 = k * nSubPixels;
			// sample one stratum for all subpixels
			TSubSequence1D::iterator start = p;
			for (size_t dk = 0; dk < nSubPixels; ++dk)
			{
				*p++ = (k0 + static_cast<TScalar>(dk) + jitter(rng_)) * scale;
			}
			std::shuffle(start, p, rng_);
		}
	}

	// pick a subpixel worth of samples
	//
	LASS_ASSERT(last - first == static_cast<std::ptrdiff_t>(subSeqSize));
	for (size_t k = 0; k < subSeqSize; ++k)
	{
		first[k] = subSequence[k * nSubPixels + subPixel];
	}
	std::shuffle(first, last, rng_); // to avoid inter-sequence coherence
}



void Stratifier::doSampleSubSequence2D(const TResolution2D& LASS_UNUSED(pixel), size_t subPixel, TSubSequenceId id, TSample2D* first, TSample2D* last)
{
	const size_t nSubPixels = strataPerPixel_;
	const size_t subSeqSize = this->subSequenceSize2D(id);

	LIAR_ASSERT(id >= 0, "subsequence id must be non-negative: id=" << id);
	const size_t i = static_cast<size_t>(id);
	if (i >= subSequences2d_.size())
	{
		LIAR_ASSERT(subPixel == 0, "subpixel must be zero for new subsequence: subPixel=" << subPixel);
		subSequences2d_.resize(i + 1);
	}
	TSubSequence2D& subSequence = subSequences2d_[i];

	if (subPixel == 0)
	{
		subSequence.resize(nSubPixels * subSeqSize);
		const size_t sqrtNSubPixels = static_cast<size_t>(num::sqrt(static_cast<TScalar>(nSubPixels)));
		LASS_ASSERT(sqrtNSubPixels * sqrtNSubPixels == nSubPixels);
		const size_t sqrtSubSeqSize = static_cast<size_t>(num::sqrt(static_cast<TScalar>(subSeqSize)));
		LASS_ASSERT(sqrtSubSeqSize * sqrtSubSeqSize == subSeqSize);
		const TScalar scale = num::inv(static_cast<TScalar>(sqrtNSubPixels * sqrtSubSeqSize));

		// generate interleaved samples: stratum1,subpixel1, stratum1,subpixel2, ... stratum2,subpixel1,stratum2,subpixel2
		TSubSequence2D::iterator p = subSequence.begin();
		for (size_t u = 0; u < sqrtSubSeqSize; ++u)
		{
			const TScalar u0 = u * sqrtNSubPixels;
			for (size_t v = 0; v < sqrtSubSeqSize; ++v)
			{
				const TScalar v0 = v * sqrtNSubPixels;
				// sample one stratum for all subpixels
				TSubSequence2D::iterator start = p;
				for (size_t du = 0; du < sqrtNSubPixels; ++du)
				{
					for (size_t dv = 0; dv < sqrtNSubPixels; ++dv)
					{
						p->x = (u0 + static_cast<TScalar>(du) + jitter(rng_)) * scale;
						p->y = (v0 + static_cast<TScalar>(dv) + jitter(rng_)) * scale;
						++p;
					}
				}
				std::shuffle(start, p, rng_);
			}
		}
	}

	// pick a subpixel worth of samples
	//
	LASS_ASSERT(last - first == static_cast<std::ptrdiff_t>(subSeqSize));
	for (size_t k = 0; k < subSeqSize; ++k)
	{
		first[k] = subSequence[k * nSubPixels + subPixel];
	}
	std::shuffle(first, last, rng_); // to avoid inter-sequence coherence
}



size_t Stratifier::doRoundSize2D(size_t requestedSize) const
{
	const TScalar realSqrt = num::sqrt(static_cast<TScalar>(requestedSize));
	const size_t lowSqrt = static_cast<size_t>(num::floor(realSqrt));
	const size_t highSqrt = lowSqrt + 1;
	const size_t lowSize = num::sqr(lowSqrt);
	const size_t highSize = num::sqr(highSqrt);
	LASS_ASSERT(requestedSize >= lowSqrt && requestedSize <= highSize);
	return (requestedSize - lowSize) < (highSize - requestedSize) ? lowSize : highSize;
}



const TSamplerPtr Stratifier::doClone() const
{
	return TSamplerPtr(new Stratifier(*this));
}



const TPyObjectPtr Stratifier::doGetState() const
{
	std::ostringstream rngState;
	LASS_ENFORCE_STREAM(rngState << rng_);
	return python::makeTuple(rngState.str(), resolution_, strataPerPixel_,
		timeStrata_, lensStrata_, isJittered_);
}



void Stratifier::doSetState(const TPyObjectPtr& state)
{
	std::string rngState;
	TResolution2D resolution;
	size_t strataPerPixel;
	TStrata2D lensStrata;
	TStrata1D timeStrata;

	python::decodeTuple(state, rngState, resolution, strataPerPixel, lensStrata, timeStrata, isJittered_);

	std::istringstream stream(rngState);
	LASS_ENFORCE_STREAM(stream >> rng_);
	setResolution(resolution);
	setSamplesPerPixel(strataPerPixel);
	lensStrata_.swap(lensStrata);
	timeStrata_.swap(timeStrata);
}



Stratifier::TSample1D Stratifier::sampleStratum(size_t subPixel, TStrata1D& strata)
{
	LASS_ASSERT(subPixel < strataPerPixel_);
	if (subPixel == 0)
	{
		std::shuffle(strata.begin(), strata.end(), rng_);
	}
	return (strata[subPixel] + jitter(rng_)) * stratum1DSize_;
}



const Stratifier::TSample2D Stratifier::sampleStratum(size_t subPixel, TStrata2D& strata)
{
	LASS_ASSERT(subPixel < strataPerPixel_);
	if (subPixel == 0)
	{
		std::shuffle(strata.begin(), strata.end(), rng_);
	}
	TVector2D position = strata[subPixel].position();
	position.x += jitter(rng_);
	position.y += jitter(rng_);
	position *= stratum2DSize_;
	return TSample2D(position);
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
