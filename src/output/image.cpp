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
PY_CLASS_CONSTRUCTOR_3(Image, std::string, Image::TSize, TScalar)
PY_CLASS_MEMBER_R(Image, "size", size)
PY_CLASS_MEMBER_RW(Image, "gamma", gamma, setGamma)



// --- public --------------------------------------------------------------------------------------

Image::Image(const std::string& iFilename, const TSize& iSize, TScalar iGamma):
    RenderTarget(&Type),
	image_(),
    filename_(iFilename),
    size_(iSize),
    gamma_(iGamma),
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



const Image::TSize& Image::size() const
{
	return size_;
}



const TScalar Image::gamma() const
{
    return gamma_;
}



void Image::setGamma(TScalar iGamma)
{
    gamma_ = iGamma;
}



// --- protected -----------------------------------------------------------------------------------

void Image::doBeginRender()
{
    image_.reset(size_.y, size_.x);
	weights_.clear();
	weights_.resize(size_.x * size_.y, TNumTraits::zero);
}



void Image::doWriteRender(const kernel::Sample& iSample, const TSpectrum& iRadiance)
{
    LASS_ASSERT(size_.x > 0 && size_.y > 0);

	const TPoint2D& position = iSample.screenCoordinate();
    LASS_ASSERT(position.x >= 0 && position.y >= 0 && position.x < 1 && position.y < 1);
    const unsigned i = static_cast<unsigned>(num::floor(position.x * size_.x));
    const unsigned j = static_cast<unsigned>(num::floor(position.y * size_.y));
    LASS_ASSERT(i < size_.x && j < size_.y);
    image_(j, i) += iRadiance * iSample.weight();
	weights_[j * size_.x + i] += TNumTraits::one;

    isSaved_ = false;
}



void Image::doEndRender()
{
	for (size_t j = 0; j < size_.y; ++j)
	{
		for (size_t i = 0; i < size_.x; ++i)
		{
			image_(j, i) /= weights_[j * size_.x + i];
		}
	}
    image_.filterGamma(gamma_);
    image_.save(filename_);
    isSaved_ = true;
}



// --- private -------------------------------------------------------------------------------------



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF