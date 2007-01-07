/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
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
 *  http://liar.sourceforge.net
 */

#include "output_common.h"
#include "image.h"

namespace liar
{
namespace output
{

PY_DECLARE_CLASS(Image);
PY_CLASS_CONSTRUCTOR_2(Image, std::string, const Image::TResolution&)
PY_CLASS_MEMBER_RW(Image, "filename", filename, setFilename)
PY_CLASS_MEMBER_RW(Image, "rgbSpace", rgbSpace, setRgbSpace)
PY_CLASS_MEMBER_RW(Image, "exposure", exposure, setExposure)
PY_CLASS_MEMBER_RW(Image, "fStops", fStops, setFStops)
PY_CLASS_MEMBER_RW(Image, "gamma", gamma, setGamma)
PY_CLASS_MEMBER_RW(Image, "gain", gain, setGain)



// --- public --------------------------------------------------------------------------------------

Image::Image(const std::string& filename, const TResolution& resolution):
	image_(),
    filename_(filename),
    resolution_(resolution),
	rgbSpace_(RgbSpace::defaultSpace()),
	exposure_(0.f),
    gamma_(1.f),
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



const TScalar Image::gamma() const
{
    return gamma_;
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



void Image::setGamma(TScalar gammaExponent)
{
    gamma_ = gammaExponent;
}



void Image::setGain(TScalar gain)
{
    gain_ = gain;
}



// --- private -------------------------------------------------------------------------------------

const Image::TResolution Image::doResolution() const
{
	return resolution_;
}



void Image::doBeginRender()
{
    image_.reset(resolution_.y, resolution_.x);
	totalWeight_.clear();
	totalWeight_.resize(resolution_.x * resolution_.y, 0);
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
			if (i >= 0 && i < resolution_.x && j >= 0 && j < resolution_.y)
			{
				const TScalar alpha = first->weight() * first->alpha();
				const TVector3D xyz = alpha * first->radiance().xyz();
				
				prim::ColorRGBA rgba = rgbSpace_->convert(xyz);
				rgba.a = alpha;

				image_(j, i) += rgba;
				totalWeight_[j * resolution_.x + i] += first->weight();
			}
			++first;
		}
	}

	isSaved_ = false;
}



void Image::doEndRender()
{
	for (unsigned j = 0; j < resolution_.y; ++j)
	{
		for (unsigned i = 0; i < resolution_.x; ++i)
		{
			const TScalar w = totalWeight_[j * resolution_.x + i];
			if (w > 0)
			{
				image_(j, i) /= w;
			}
		}
	}
	if (exposure_ > 0)
	{
		image_.filterExposure(exposure_);
	}
	if (gamma_ != 1)
	{
		image_.filterGamma(gamma_);
	}
	if (gain_ != 1)
	{
		const unsigned n = resolution_.x * resolution_.y;
		for (unsigned i = 0; i < n; ++i)
		{
			image_[i] *= gain_;
		}
	}
    image_.save(filename_);
    isSaved_ = true;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
