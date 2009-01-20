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
#include <lass/stde/range_algorithm.h>

namespace liar
{
namespace samplers
{

PY_DECLARE_CLASS(Stratifier);
PY_CLASS_CONSTRUCTOR_0(Stratifier)
PY_CLASS_CONSTRUCTOR_1(Stratifier, const TResolution2D&)
PY_CLASS_CONSTRUCTOR_2(Stratifier, const TResolution2D&, unsigned)
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



Stratifier::Stratifier(const TResolution2D& resolution, unsigned numberOfSamplesPerPixel):
	jitterGenerator_(numberGenerator_),
	isJittered_(true)
{
	init(resolution, numberOfSamplesPerPixel);
}



const bool Stratifier::jittered() const
{
	return isJittered_;
}



void Stratifier::setJittered(bool enabled)
{
	isJittered_ = enabled;
}



// --- protected -----------------------------------------------------------------------------------




// --- private -------------------------------------------------------------------------------------

void Stratifier::init(const TResolution2D& resolution, unsigned numberOfSamplesPerPixel)
{
	setResolution(resolution);
	setSamplesPerPixel(numberOfSamplesPerPixel);
}



const TResolution2D& Stratifier::doResolution() const
{
    return resolution_;
}



const unsigned Stratifier::doSamplesPerPixel() const
{
    return strataPerPixel_;
}



void Stratifier::doSetResolution(const TResolution2D& resolution)
{
    LASS_ASSERT(resolution.x > 0 && resolution.y > 0);
    resolution_ = resolution;
    reciprocalResolution_ = TVector2D(resolution).reciprocal();
}



void Stratifier::doSetSamplesPerPixel(unsigned samplesPerPixel)
{
    const unsigned strataPerAxis = static_cast<unsigned>(num::ceil(
        num::sqrt(static_cast<TScalar>(samplesPerPixel))));
    
    strataPerPixel_ = strataPerAxis * strataPerAxis;
    stratum1DSize_ = TNumTraits::one / strataPerPixel_;
	stratum2DSize_ = 
		TVector2D(TNumTraits::one, TNumTraits::one) / static_cast<TScalar>(strataPerAxis);
    timeStrata_.resize(strataPerPixel_);
    screenStrata_.resize(strataPerPixel_);
    lensStrata_.resize(strataPerPixel_);

	for (unsigned i = 0; i < strataPerPixel_; ++i)
	{
		timeStrata_[i] = static_cast<TScalar>(i);
	}

    for (unsigned j = 0; j < strataPerAxis; ++j)
    {
        for (unsigned i = 0; i < strataPerAxis; ++i)
        {
            screenStrata_[j * strataPerAxis + i] = TVector2D(i, j);
			lensStrata_[j * strataPerAxis + i] = TVector2D(i, j);
        }
    }
}



void Stratifier::doSeed(unsigned randomSeed)
{
	numberGenerator_.seed(randomSeed);
}



void Stratifier::doSampleScreen(const TResolution2D& pixel, unsigned subPixel, 
								TSample2D& oScreenCoordinate)
{
    LASS_ASSERT(pixel.x < resolution_.x && pixel.y < resolution_.y);
    LASS_ASSERT(subPixel < strataPerPixel_);

	TVector2D position = screenStrata_[subPixel];
	position.x += isJittered_ ? jitterGenerator_() : .5f;
	position.y += isJittered_ ? jitterGenerator_() : .5f;
	position *= stratum2DSize_;

	position += TVector2D(pixel);
	position *= reciprocalResolution_;

	oScreenCoordinate = TPoint2D(position);
}



void Stratifier::doSampleLens(const TResolution2D& pixel, unsigned subPixel, 
							  TSample2D& oLensCoordinate)
{
    LASS_ASSERT(pixel.x < resolution_.x && pixel.y < resolution_.y);
    LASS_ASSERT(subPixel < strataPerPixel_);

	if (subPixel == 0)
	{
		stde::random_shuffle_r(lensStrata_, numberGenerator_);	
	}
	TVector2D position = lensStrata_[subPixel];
	position.x += isJittered_ ? jitterGenerator_() : .5f;
	position.y += isJittered_ ? jitterGenerator_() : .5f;
	position *= stratum2DSize_;

	oLensCoordinate = TSample2D(position);
}



void Stratifier::doSampleTime(const TResolution2D& pixel, unsigned subPixel, 
							  const TimePeriod& period, TTime& oTime)
{
    LASS_ASSERT(pixel.x < resolution_.x && pixel.y < resolution_.y);
    LASS_ASSERT(subPixel < strataPerPixel_);

	if (subPixel == 0)
	{
		stde::random_shuffle_r(timeStrata_, numberGenerator_);	
	}
	const TScalar tau = 
		(timeStrata_[subPixel] + (isJittered_ ? jitterGenerator_() : 0.5f)) * stratum1DSize_;
	oTime = period.interpolate(tau);
}



void Stratifier::doSampleSubSequence1D(const TResolution2D& pixel, unsigned subPixel, 
									   TSample1D* oBegin, TSample1D* oEnd)
{
	const ptrdiff_t size = oEnd - oBegin;
	const TScalar scale = 1.f / size;
	for (ptrdiff_t k = 0; k < size; ++k, ++oBegin)
	{
		*oBegin = (k + jitterGenerator_()) * scale;
	}
	LASS_ASSERT(oBegin == oEnd);
}



void Stratifier::doSampleSubSequence2D(const TResolution2D& pixel, unsigned subPixel, 
									   TSample2D* oBegin, TSample2D* oEnd)
{
	const size_t size = oEnd - oBegin;
	const size_t sqrtSize = static_cast<size_t>(num::sqrt(static_cast<double>(size)));
	LASS_ASSERT(sqrtSize * sqrtSize == size);

	const TScalar scale = TNumTraits::one / sqrtSize;
	for (size_t i = 0; i < sqrtSize; ++i)
	{
		for (size_t j = 0; j < sqrtSize; ++j, ++oBegin)
		{
			oBegin->x = (i + jitterGenerator_()) * scale;
			oBegin->y = (j + jitterGenerator_()) * scale;
		}
	}

	LASS_ASSERT(oBegin == oEnd);
}



const unsigned Stratifier::doRoundSize1D(unsigned requestedSize) const
{
	return requestedSize;
}



const unsigned Stratifier::doRoundSize2D(unsigned requestedSize) const
{
	const TScalar realSqrt = num::sqrt(static_cast<TScalar>(requestedSize));
	const unsigned lowSqrt = static_cast<unsigned>(num::floor(realSqrt));
	const unsigned highSqrt = lowSqrt + 1;

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
	unsigned strataPerPixel;
	TStrata2D lensStrata;
	TStrata1D timeStrata;

	python::decodeTuple(state, numGenState, resolution, strataPerPixel, lensStrata,
        timeStrata,	isJittered_);

	numberGenerator_.setState(numGenState.begin(), numGenState.end());
	setResolution(resolution);
	setSamplesPerPixel(strataPerPixel);
	lensStrata_.swap(lensStrata);
	timeStrata_.swap(timeStrata);
}


void Stratifier::shuffleTimeStrata()
{
	typedef num::DistributionUniform<unsigned, TNumberGenerator, num::rtRightOpen> TRandomSelector;

	// shuffle strata in linear time guaranteed.
	//
	for (unsigned i = 0; i < strataPerPixel_; ++i)
	{
		TRandomSelector selector(numberGenerator_, i, strataPerPixel_);
		std::swap(timeStrata_[i], timeStrata_[selector()]);
	}
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
