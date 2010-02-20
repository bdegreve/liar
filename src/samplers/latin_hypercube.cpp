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
#include "latin_hypercube.h"
#include "../kernel/xyz.h"
#include <lass/stde/range_algorithm.h>

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
	jitterGenerator_(numberGenerator_),
	isJittered_(true)
{
	init();
}



LatinHypercube::LatinHypercube(const TResolution2D& resolution):
	jitterGenerator_(numberGenerator_),
	isJittered_(true)
{
	init(resolution);
}



LatinHypercube::LatinHypercube(const TResolution2D& resolution, size_t numberOfSamplesPerPixel):
	jitterGenerator_(numberGenerator_),
	isJittered_(true)
{
	init(resolution, numberOfSamplesPerPixel);
}



const bool LatinHypercube::jittered() const
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



const size_t LatinHypercube::doSamplesPerPixel() const
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
	stratumSize_ = TNumTraits::one / samplesPerPixel_;
	timeStrata_.resize(samplesPerPixel_);
	frequencyStrata_.resize(samplesPerPixel_);
	screenStrataX_.resize(samplesPerPixel_);
	screenStrataY_.resize(samplesPerPixel_);
	lensStrataX_.resize(samplesPerPixel_);
	lensStrataY_.resize(samplesPerPixel_);

	for (size_t i = 0; i < samplesPerPixel_; ++i)
	{
		TSample1D x = static_cast<TSample1D>(i);
		screenStrataX_[i] = x;
		screenStrataY_[i] = x;
		lensStrataX_[i] = x;
		lensStrataY_[i] = x;
		timeStrata_[i] = x;
		frequencyStrata_[i] = x;
	}
}



void LatinHypercube::doSeed(size_t randomSeed)
{
	numberGenerator_.seed(randomSeed);
}



void LatinHypercube::doSampleScreen(const TResolution2D& pixel, size_t subPixel, 
								TSample2D& screenCoordinate)
{
	LASS_ASSERT(pixel.x < resolution_.x && pixel.y < resolution_.y);
	TVector2D position(sampleStratum(subPixel, screenStrataX_), sampleStratum(subPixel, screenStrataY_));
	position += TVector2D(pixel);
	position *= reciprocalResolution_;
	screenCoordinate = TSample2D(position);
}



void LatinHypercube::doSampleLens(const TResolution2D& pixel, size_t subPixel, 
							  TSample2D& lensCoordinate)
{
	LASS_ASSERT(pixel.x < resolution_.x && pixel.y < resolution_.y);
	lensCoordinate = TSample2D(sampleStratum(subPixel, lensStrataX_), sampleStratum(subPixel, lensStrataY_));
}



void LatinHypercube::doSampleTime(const TResolution2D& pixel, size_t subPixel, 
							  const TimePeriod& period, TTime& time)
{
	LASS_ASSERT(pixel.x < resolution_.x && pixel.y < resolution_.y);
	const TScalar tau = sampleStratum(subPixel, timeStrata_);
	time = period.interpolate(tau);
}



void LatinHypercube::doSampleFrequency(const TResolution2D& pixel, size_t subPixel, TScalar& frequency)
{
	LASS_ASSERT(pixel.x < resolution_.x && pixel.y < resolution_.y);
	const TScalar phi = sampleStratum(subPixel, frequencyStrata_);
	XYZ xyz;
	TScalar pdf;
	frequency = standardObserver().sample(XYZ(1, 1, 1), phi, xyz, pdf);
}



void LatinHypercube::doSampleSubSequence1D(
		const TResolution2D& pixel, size_t subPixel, TSample1D* first, TSample1D* last)
{
	const ptrdiff_t size = last - first;
	const TScalar scale = 1.f / size;
	for (ptrdiff_t k = 0; k < size; ++k)
	{
		first[k] = (k + jitterGenerator_()) * scale;
	}
}



void LatinHypercube::doSampleSubSequence2D(
		const TResolution2D& pixel, size_t subPixel, TSample2D* first, TSample2D* last)
{
	const size_t size = last - first;
	const TScalar scale = 1.f / size;
	for (size_t k = 0; k < size; ++k)
	{
		first[k].y = (k + jitterGenerator_()) * scale;
	}
	std::random_shuffle(first, last, numberGenerator_);
	for (size_t k = 0; k < size; ++k)
	{
		first[k].x = (k + jitterGenerator_()) * scale;
	}
}



const size_t LatinHypercube::doRoundSize1D(size_t requestedSize) const
{
	return requestedSize;
}



const size_t LatinHypercube::doRoundSize2D(size_t requestedSize) const
{
	return requestedSize;
}



const TSamplerPtr LatinHypercube::doClone() const
{
	return TSamplerPtr(new LatinHypercube(*this));
}



const TPyObjectPtr LatinHypercube::doGetState() const
{
	std::vector<TNumberGenerator::TValue> numGenState;
	numberGenerator_.getState(std::back_inserter(numGenState));
	return python::makeTuple(numGenState, resolution_, samplesPerPixel_, isJittered_);
}



void LatinHypercube::doSetState(const TPyObjectPtr& state)
{
	std::vector<TNumberGenerator::TValue> numGenState;
	TResolution2D resolution;
	size_t strataPerPixel;

	python::decodeTuple(state, numGenState, resolution, strataPerPixel, isJittered_);

	numberGenerator_.setState(numGenState.begin(), numGenState.end());
	setResolution(resolution);
	setSamplesPerPixel(strataPerPixel);
}



const LatinHypercube::TSample1D LatinHypercube::sampleStratum(size_t subPixel, TStrata& strata)
{
	LASS_ASSERT(subPixel < samplesPerPixel_);
	if (subPixel == 0)
	{
		stde::random_shuffle_r(strata, numberGenerator_);	
	}
	return (strata[subPixel] + (isJittered_ ? jitterGenerator_() : 0.5f)) * stratumSize_;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF