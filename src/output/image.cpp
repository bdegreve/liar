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

#include "output_common.h"
#include "image.h"
#include "../kernel/image_codec.h"
#include <lass/io/file_attribute.h>

namespace liar
{
namespace output
{

PY_DECLARE_CLASS_DOC(Image, "simple image render target");
PY_CLASS_CONSTRUCTOR_2(Image, std::string, const TResolution2D&)
PY_CLASS_MEMBER_RW(Image, filename, setFilename)
PY_CLASS_MEMBER_RW(Image, rgbSpace, setRgbSpace)
PY_CLASS_MEMBER_RW(Image, exposure, setExposure)
PY_CLASS_MEMBER_RW(Image, fStops, setFStops)
PY_CLASS_MEMBER_RW(Image, gain, setGain)



// --- public --------------------------------------------------------------------------------------

Image::Image(const std::string& filename, const TResolution2D& resolution):
    filename_(filename),
    resolution_(resolution),
	rgbSpace_(),
	exposure_(0.f),
    gain_(1.f),
    isSaved_(true)
{
}



Image::~Image()
{
    if (!isSaved_)
    {
        endRender();
    }
}



const std::string& Image::filename() const
{
	return filename_;
}



const TRgbSpacePtr& Image::rgbSpace() const
{
	return rgbSpace_;
}



const TScalar Image::exposure() const
{
    return exposure_;
}



const TScalar Image::fStops() const
{
	return num::log2(exposure_);
}



const TScalar Image::gain() const
{
    return gain_;
}



void Image::setFilename(const std::string& filename)
{
	filename_ = filename;
}



void Image::setRgbSpace(const TRgbSpacePtr& rgbSpace)
{
	rgbSpace_ = rgbSpace;
}



void Image::setExposure(TScalar exposure)
{
    exposure_ = exposure;
}



void Image::setFStops(TScalar fStops)
{
	exposure_ = num::pow(TScalar(2), fStops);
}



void Image::setGain(TScalar gain)
{
    gain_ = gain;
}



// --- private -------------------------------------------------------------------------------------

const TResolution2D Image::doResolution() const
{
	return resolution_;
}



void Image::doBeginRender()
{
	const size_t n = resolution_.x * resolution_.y;
	renderBuffer_.clear();
	renderBuffer_.resize(n);
	totalWeight_.clear();
	totalWeight_.resize(n);
	alphaBuffer_.clear();
	alphaBuffer_.resize(n);
}



void Image::doWriteRender(const OutputSample* first, const OutputSample* last)
{
    LASS_ASSERT(resolution_.x > 0 && resolution_.y > 0);

	LASS_LOCK(lock_)
	{
		while (first != last)
		{
			const TPoint2D& position = first->screenCoordinate();
			const int i = static_cast<int>(num::floor(position.x * resolution_.x));
			const int j = static_cast<int>(num::floor(position.y * resolution_.y));
			if (i >= 0 && static_cast<size_t>(i) < resolution_.x && j >= 0 && static_cast<size_t>(j) < resolution_.y)
			{
				const size_t k = j * resolution_.x + i;
				const TScalar alpha = first->weight() * first->alpha();
				renderBuffer_[k] += first->radiance() * alpha;
				alphaBuffer_[k] += alpha;
				totalWeight_[k] += first->weight();
			}
			++first;
		}
	}

	isSaved_ = false;
}



void Image::doEndRender()
{
	if (isSaved_)
	{
		return;
	}

	if (exposure_ > 0) 
	{
		for (size_t j = 0; j < resolution_.y; ++j)
		{
			for (size_t i = 0; i < resolution_.x; ++i)
			{
				const size_t k = j * resolution_.x + i;
				const TScalar w = totalWeight_[j * resolution_.x + i];
				if (w > 0)
				{
					XYZ& xyz = renderBuffer_[k];
					xyz *= gain_ / w;
					xyz.x = 1 - num::exp(-exposure_ * xyz.x);
					xyz.y = 1 - num::exp(-exposure_ * xyz.y);
					xyz.z = 1 - num::exp(-exposure_ * xyz.z);
					alphaBuffer_[k] /= w;
				}
			}
		}
	}
	else
	{
		for (size_t j = 0; j < resolution_.y; ++j)
		{
			for (size_t i = 0; i < resolution_.x; ++i)
			{
				const size_t k = j * resolution_.x + i;
				const TScalar w = totalWeight_[j * resolution_.x + i];
				if (w > 0)
				{
					renderBuffer_[k] *= gain_ / w;
					alphaBuffer_[k] /= w;
				}
			}
		}
	}

	ImageWriter writer(filename_, resolution_, rgbSpace_, "");
	writer.writeFull(&renderBuffer_[0], &alphaBuffer_[0]);
	isSaved_ = true;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
