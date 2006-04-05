/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2005  Bram de Greve
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "output_common.h"
#include "image.h"

namespace liar
{
namespace output
{

PY_DECLARE_CLASS(Image);
PY_CLASS_CONSTRUCTOR_2(Image, std::string, Image::TSize)
PY_CLASS_MEMBER_R(Image, "size", size)
PY_CLASS_MEMBER_RW(Image, "filename", filename, setFilename)
PY_CLASS_MEMBER_RW(Image, "rgbSpace", rgbSpace, setRgbSpace)
PY_CLASS_MEMBER_RW(Image, "gamma", gamma, setGamma)
PY_CLASS_MEMBER_RW(Image, "exposureTime", exposureTime, setExposureTime)



// --- public --------------------------------------------------------------------------------------

Image::Image(const std::string& iFilename, const TSize& iSize):
    RenderTarget(&Type),
	image_(),
    filename_(iFilename),
    size_(iSize),
	rgbSpace_(RgbSpace::defaultSpace()),
    gamma_(1.f),
	exposureTime_(-1.f),
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



const Image::TSize& Image::size() const
{
	return size_;
}



const TRgbSpacePtr& Image::rgbSpace() const
{
	return rgbSpace_;
}



const TScalar Image::gamma() const
{
    return gamma_;
}



const TScalar Image::exposureTime() const
{
    return exposureTime_;
}



void Image::setFilename(const std::string& iFilename)
{
	filename_ = iFilename;
}



void Image::setRgbSpace(const TRgbSpacePtr& iRgbSpace)
{
	rgbSpace_ = iRgbSpace;
}



void Image::setGamma(TScalar iGamma)
{
    gamma_ = iGamma;
}



void Image::setExposureTime(TScalar iTime)
{
    exposureTime_ = iTime;
}



// --- private -------------------------------------------------------------------------------------

void Image::doBeginRender()
{
    image_.reset(size_.y, size_.x);
	numberOfSamples_.clear();
	numberOfSamples_.resize(size_.x * size_.y, 0);
}



void Image::doWriteRender(const OutputSample* iFirst, const OutputSample* iLast)
{
    LASS_ASSERT(size_.x > 0 && size_.y > 0);

	while (iFirst != iLast)
	{
		const TPoint2D& position = iFirst->screenCoordinate();
		LASS_ASSERT(position.x >= 0 && position.y >= 0 && position.x < 1 && position.y < 1);
		const unsigned i = static_cast<unsigned>(num::floor(position.x * size_.x));
		const unsigned j = static_cast<unsigned>(num::floor(position.y * size_.y));
		if (i < 0 || i >= size_.x || j < 0 || j >= size_.y)
		{
			return;
		}
		LASS_ASSERT(i < size_.x && j < size_.y);

		TVector3D xyz = iFirst->radiance().xyz();
		xyz *= iFirst->weight();
		
		image_(j, i) += rgbSpace_->convert(xyz);
		++numberOfSamples_[j * size_.x + i];

		++iFirst;
	}

	isSaved_ = false;
}



void Image::doEndRender()
{
	for (unsigned j = 0; j < size_.y; ++j)
	{
		for (unsigned i = 0; i < size_.x; ++i)
		{
			const unsigned nSamples = numberOfSamples_[j * size_.x + i];
			image_(j, i) /= static_cast<TScalar>(nSamples ? nSamples : 1);
		}
	}
    image_.filterGamma(gamma_);
	if (exposureTime_ > 0)
	{
		image_.filterExposure(exposureTime_);
	}
    image_.save(filename_);
    isSaved_ = true;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF