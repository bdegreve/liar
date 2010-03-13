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

#include "samplers_common.h"
#include "stratifier.h"
#include "../kernel/xyz.h"
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
	jitterGenerator_(numberGenerator_),
	isJittered_(true)
{
	init();
}



Stratifier::Stratifier(const TResolution2D& resolution):
	jitterGenerator_(numberGenerator_),
	isJittered_(true)
{
	init(resolution);
}



Stratifier::Stratifier(const TResolution2D& resolution, size_t numberOfSamplesPerPixel):
	jitterGenerator_(numberGenerator_),
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
	stratum1DSize_ = TNumTraits::one / strataPerPixel_;
	stratum2DSize_ = TVector2D(TNumTraits::one, TNumTraits::one) / static_cast<TScalar>(strataPerAxis);
	timeStrata_.resize(strataPerPixel_);
	frequencyStrata_.resize(strataPerPixel_);
	screenStrata_.resize(strataPerPixel_);
	lensStrata_.resize(strataPerPixel_);
	subSequences2d_.clear();

	for (size_t i = 0; i < strataPerPixel_; ++i)
	{
		timeStrata_[i] = static_cast<TSample1D>(i);
		frequencyStrata_[i] = static_cast<TSample1D>(i);
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
	numberGenerator_.seed(randomSeed);
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



void Stratifier::doSampleFrequency(const TResolution2D& LASS_UNUSED(pixel), size_t subPixel, TScalar& frequency)
{
	LASS_ASSERT(pixel.x < resolution_.x && pixel.y < resolution_.y);
	const TScalar phi = sampleStratum(subPixel, frequencyStrata_);
	XYZ xyz;
	TScalar pdf;
	frequency = standardObserver().sample(XYZ(1, 1, 1), phi, xyz, pdf);
}



void Stratifier::doSampleSubSequence1D(const TResolution2D& LASS_UNUSED(pixel), size_t, TSubSequenceId, TSample1D* first, TSample1D* last)
{
	const ptrdiff_t size = last - first;
	const TScalar scale = 1.f / size;
	for (ptrdiff_t k = 0; k < size; ++k, ++first)
	{
		*first = (k + jitterGenerator_()) * scale;
	}
	LASS_ASSERT(first == last);
}



void Stratifier::doSampleSubSequence2D(const TResolution2D& LASS_UNUSED(pixel), size_t subPixel, TSubSequenceId id, TSample2D* first, TSample2D* last)
{
	const size_t nSubPixels = this->samplesPerPixel();
	const size_t subSeqSize = this->subSequenceSize2D(id);
	const size_t size = nSubPixels * subSeqSize;

	LASS_ASSERT(id >= 0);
	if (static_cast<size_t>(id) >= subSequences2d_.size())
	{
		subSequences2d_.resize(id + 1);
		subSequences2d_[id] = TSubSequence2D(nSubPixels * subSeqSize);
	}		
	
	if (subPixel == 0)
	{
		const size_t sqrtNSubPixels = static_cast<size_t>(num::sqrt(static_cast<double>(nSubPixels)));
		LASS_ASSERT(sqrtNSubPixels * sqrtNSubPixels == nSubPixels);
		const size_t sqrtSubSeqSize = static_cast<size_t>(num::sqrt(static_cast<double>(subSeqSize)));
		LASS_ASSERT(sqrtSubSeqSize * sqrtSubSeqSize == subSeqSize);
		const TScalar scale = TNumTraits::one / (sqrtNSubPixels * sqrtSubSeqSize);

		// generate interleaved samples: stratum1,subpixel1, stratum1,subpixel2, ... stratum2,subpixel1,stratum2,subpixel2
		TSubSequence2D::iterator p = subSequences2d_[id].begin();	
		for (size_t i = 0; i < sqrtSubSeqSize; ++i)
		{
			for (size_t j = 0; j < sqrtSubSeqSize; ++j)
			{
				// sample one stratum for all subpixels
				TSubSequence2D::iterator start = p;
				for (size_t di = 0; di < sqrtNSubPixels; ++di)
				{
					for (size_t dj = 0; dj < sqrtNSubPixels; ++dj)
					{
						p->x = ((i * sqrtNSubPixels + di) + jitterGenerator_()) * scale;
						p->y = ((j * sqrtNSubPixels + dj) + jitterGenerator_()) * scale;
						++p;
					}
				}
				std::random_shuffle(start, p, numberGenerator_);
			}
		}
	}

	// pick a subpixel worth of samples
	//
	LASS_ASSERT(last - first == subSeqSize);
	TSubSequence2D::iterator p = subSequences2d_[id].begin();	
	for (size_t k = 0; k < subSeqSize; ++k)
	{
		first[k] = p[k * nSubPixels + subPixel];
	}
}



size_t Stratifier::doRoundSize1D(size_t requestedSize) const
{
	return requestedSize;
}



size_t Stratifier::doRoundSize2D(size_t requestedSize) const
{
	const TScalar realSqrt = num::sqrt(static_cast<TScalar>(requestedSize));
	const size_t lowSqrt = static_cast<size_t>(num::floor(realSqrt));
	const size_t highSqrt = lowSqrt + 1;

	return (realSqrt - lowSqrt) < (highSqrt - requestedSize) ? num::sqr(lowSqrt) : num::sqr(highSqrt);
}



const TSamplerPtr Stratifier::doClone() const
{
	return TSamplerPtr(new Stratifier(*this));
}



const TPyObjectPtr Stratifier::doGetState() const
{
	std::vector<TNumberGenerator::TValue> numGenState;
	numberGenerator_.getState(std::back_inserter(numGenState));
	return python::makeTuple(numGenState, resolution_,	strataPerPixel_,
		timeStrata_, lensStrata_, isJittered_);
}



void Stratifier::doSetState(const TPyObjectPtr& state)
{
	std::vector<TNumberGenerator::TValue> numGenState;
	TResolution2D resolution;
	size_t strataPerPixel;
	TStrata2D lensStrata;
	TStrata1D timeStrata;

	python::decodeTuple(state, numGenState, resolution, strataPerPixel, lensStrata, timeStrata, isJittered_);

	numberGenerator_.setState(numGenState.begin(), numGenState.end());
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
		stde::random_shuffle_r(strata, numberGenerator_);	
	}
	return (strata[subPixel] + (isJittered_ ? jitterGenerator_() : 0.5f)) * stratum1DSize_;
}



const Stratifier::TSample2D Stratifier::sampleStratum(size_t subPixel, TStrata2D& strata)
{
	LASS_ASSERT(subPixel < strataPerPixel_);
	if (subPixel == 0)
	{
		stde::random_shuffle_r(strata, numberGenerator_);	
	}
	TVector2D position = strata[subPixel].position();
	position.x += isJittered_ ? jitterGenerator_() : .5f;
	position.y += isJittered_ ? jitterGenerator_() : .5f;
	position *= stratum2DSize_;
	return TSample2D(position);
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
