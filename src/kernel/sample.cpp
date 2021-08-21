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

#include "kernel_common.h"
#include "sample.h"
#include "sampler.h"
#include "observer.h"

namespace liar
{
namespace kernel
{

// --- public --------------------------------------------------------------------------------------

Sample::Sample():
	screenSample_(TSample2D(0, 0)),
	lensSample_(TSample2D(0, 0)),
	time_(0.f),
	wavelength_(0),
	wavelengthPdf_(0),
	weight_(1),
	sampler_(0)
{
}


const Sample::TSample2D& Sample::screenSample() const
{
	return screenSample_;
}


void Sample::setScreenSample(const TSample2D& sample)
{
	screenSample_ = sample;
}


const Sample::TSample2D& Sample::lensSample() const
{
	return lensSample_;
}


void Sample::setLensSample(const TSample2D& sample)
{
	lensSample_ = sample;
}


TTime Sample::time() const
{
	return time_;
}


void Sample::setTime(TTime time)
{
	time_ = time;
}


TWavelength Sample::wavelength() const
{
	return wavelength_;
}


TWavelength Sample::wavelength(TScalar& pdf) const
{
	pdf = wavelengthPdf_;
	return wavelength_;
}


void Sample::setWavelengthSample(TSample1D sample)
{
	wavelength_ = standardObserver().sample(sample, wavelengthPdf_);
}


TScalar Sample::weight() const
{
	return weight_;
}



void Sample::setWeight(TScalar weight)
{
	weight_ = weight;
}



const Sample::TSubSequence1D Sample::subSequence1D(int id) const
{
	LASS_ASSERT(sampler_ && id >= 0);
	const size_t k = sampler_->subSequenceOffset1D(id);
	const size_t n = sampler_->subSequenceSize1D(id);
	return TSubSequence1D(&subSequences1D_[0] + k, &subSequences1D_[0] + k + n);
}


const Sample::TSubSequence2D Sample::subSequence2D(int id) const
{
	LASS_ASSERT(sampler_ && id >= 0);
	const size_t k = sampler_->subSequenceOffset2D(id);
	const size_t n = sampler_->subSequenceSize2D(id);
	return TSubSequence2D(&subSequences2D_[0] + k, &subSequences2D_[0] + k + n);
}


void Sample::setSubSample1D(int id, size_t offset, TSample1D sample)
{
	LASS_ASSERT(sampler_ && id >= 0 && offset < sampler_->subSequenceSize1D(id));
	const size_t k = sampler_->subSequenceOffset1D(id);
	subSequences1D_[k + offset] = sample;
}


void Sample::setSubSample2D(int id, size_t offset, TSample2D sample)
{
	LASS_ASSERT(sampler_ && id >= 0 && offset < sampler_->subSequenceSize2D(id));
	const size_t k = sampler_->subSequenceOffset2D(id);
	subSequences2D_[k + offset] = sample;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
