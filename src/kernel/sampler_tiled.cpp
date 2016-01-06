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

#include "kernel_common.h"
#include "sampler_tiled.h"
#include "sample.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(SamplerTiled, "Abstract base class of samplers that render a number of samples per pixel")
	PY_CLASS_MEMBER_RW(SamplerTiled, resolution, setResolution)
	PY_CLASS_MEMBER_RW(SamplerTiled, samplesPerPixel, setSamplesPerPixel)



// --- public --------------------------------------------------------------------------------------



// --- protected -----------------------------------------------------------------------------------

SamplerTiled::SamplerTiled() :
	Sampler(),
	nextId_(0)
{
}



// --- private -------------------------------------------------------------------------------------

SamplerTiled::TTaskPtr SamplerTiled::doGetTask()
{
	const TResolution2D res = resolution();

	const TResolution2D begin(num::floor(bucket().min().x * res.x), num::floor(bucket().min().y * res.y));
	const TResolution2D end(num::floor(bucket().max().x * res.x), num::floor(bucket().max().y * res.y));

	const size_t tileSize = 16;
	const size_t n_x = (end.x - begin.x + tileSize - 1) / tileSize;
	const size_t n_y = (end.y - begin.y + tileSize - 1) / tileSize;

	const size_t id = nextId_++;
	if (id >= n_x * n_y)
	{
		return TTaskPtr(0);
	}
	const size_t i = id % n_x;
	const size_t j = id / n_x;

	const TResolution2D first(begin.x + i * tileSize, begin.y + j * tileSize);
	const TResolution2D last(std::min(first.x + tileSize, end.x), std::min(first.y + tileSize, end.y));
	return TTaskPtr(new TaskTiled(id, first, last, this->samplesPerPixel()));
}



void SamplerTiled::sample(const TResolution2D& pixel, size_t subPixel, const TimePeriod& period, Sample& sample)
{
	doSampleScreen(pixel, subPixel, sample.screenSample_);
	doSampleLens(pixel, subPixel, sample.lensSample_);
	doSampleTime(pixel, subPixel, period, sample.time_);
	doSampleWavelength(pixel, subPixel, sample.wavelength_, sample.wavelengthPdf_);

	const size_t n1D = subSequenceSize1D_.size();
	for (size_t k = 0; k < n1D; ++k)
	{
		const size_t offset = subSequenceOffset1D_[k];
		const size_t size = subSequenceSize1D_[k];
		doSampleSubSequence1D(pixel, subPixel, static_cast<TSubSequenceId>(k),
			&sample.subSequences1D_[offset], &sample.subSequences1D_[offset] + size);
	}

	const size_t n2D = subSequenceSize2D_.size();
	for (size_t k = 0; k < n2D; ++k)
	{
		const size_t offset = subSequenceOffset2D_[k];
		const size_t size = subSequenceSize2D_[k];
		doSampleSubSequence2D(pixel, subPixel, static_cast<TSubSequenceId>(k),
			&sample.subSequences2D_[offset], &sample.subSequences2D_[offset] + size);
	}
}



// --- free ----------------------------------------------------------------------------------------




// --- TaskTiled -----------------------------------------------------------------------------------

SamplerTiled::TaskTiled::TaskTiled(size_t id, const TResolution2D& begin, const TResolution2D& end, size_t samplesPerPixel):
	Task(id),
	begin_(begin),
	end_(end),
	pixel_(begin),
	samplesPerPixel_(samplesPerPixel),
	subPixel_(0)
{
}


bool SamplerTiled::TaskTiled::doDrawSample(Sampler& sampler, const TimePeriod& period, Sample& sample)
{
	if (subPixel_ == samplesPerPixel_)
	{
		subPixel_ = 0;
		++pixel_.x;
		if (pixel_.x == end_.x)
		{
			pixel_.x = begin_.x;
			++pixel_.y;
			if (pixel_.y == end_.y)
			{
				return false;
			}
		}
	}
	static_cast<SamplerTiled&>(sampler).sample(pixel_, subPixel_, period, sample);
	++subPixel_;
	return true;
}


}

}

// EOF
