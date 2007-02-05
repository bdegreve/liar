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

#include "kernel_common.h"
#include "rgb_space.h"
#include <lass/num/impl/matrix_solve.h>

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS(RgbSpace);
PY_CLASS_CONSTRUCTOR_3(RgbSpace, const TVector3D&, const TVector3D&, const TVector3D&);
PY_CLASS_MEMBER_R(RgbSpace, red);
PY_CLASS_MEMBER_R(RgbSpace, green);
PY_CLASS_MEMBER_R(RgbSpace, blue);
PY_CLASS_STATIC_METHOD(RgbSpace, defaultSpace);
PY_CLASS_STATIC_METHOD(RgbSpace, setDefaultSpace);

//// load default RGB space.  We use the CIE RGB space here.
//// you can find it here: http://www.brucelindbloom.com/index.html?WorkingSpaceInfo.html
////
//TRgbSpacePtr RgbSpace::defaultSpace_ = TRgbSpacePtr(new RgbSpace(
//	TVector3D(0.490, 0.177, 0.000),
//	TVector3D(0.310, 0.812, 0.010), 
//	TVector3D(0.200, 0.011, 0.990)));

TRgbSpacePtr RgbSpace::defaultSpace_ = TRgbSpacePtr(new RgbSpace(
	TVector3D(1.f, 0.f, 0.f),
	TVector3D(0.f, 1.f, 0.f), 
	TVector3D(0.f, 0.f, 1.f)));




RgbSpace::RgbSpace(const TVector3D& red, const TVector3D& green, const TVector3D& blue):
	x_(1, 0, 0),
	y_(0, 1, 0),
	z_(0, 0, 1),
	red_(red),
	green_(green),
	blue_(blue)
{
	TScalar rgb2xyz[9] =
	{
		red_.x, green_.x, blue_.x,
		red_.y, green_.y, blue_.y,
		red_.z, green_.z, blue_.z
	};
	TScalar xyz2rgb[9] =
	{
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
	};

	if (!num::impl::cramer3<TScalar>(rgb2xyz, xyz2rgb, xyz2rgb + 9))
	{
		LASS_THROW("RGB space forms a singular matrix, cannot invert.");
	}

	x_ = prim::ColorRGBA(xyz2rgb[0], xyz2rgb[1], xyz2rgb[2]);
	y_ = prim::ColorRGBA(xyz2rgb[3], xyz2rgb[4], xyz2rgb[5]);
	z_ = prim::ColorRGBA(xyz2rgb[6], xyz2rgb[7], xyz2rgb[8]);
}



const TVector3D RgbSpace::convert(const prim::ColorRGBA& rgb) const
{
	return red_ * rgb.r + green_ * rgb.g + blue_ * rgb.b;
}



const prim::ColorRGBA RgbSpace::convert(const TVector3D& xyz) const
{
	return prim::ColorRGBA(
		x_.r * xyz.x + y_.r * xyz.y + z_.r * xyz.z,
		x_.g * xyz.x + y_.g * xyz.y + z_.g * xyz.z,
		x_.b * xyz.x + y_.b * xyz.y + z_.b * xyz.z,
		1);
}



const TVector3D& RgbSpace::red() const
{
	return red_;
}



const TVector3D& RgbSpace::green() const
{
	return green_;
}



const TVector3D& RgbSpace::blue() const
{
	return blue_;
}



const TRgbSpacePtr& RgbSpace::defaultSpace()
{
	return defaultSpace_;
}


void RgbSpace::setDefaultSpace(const TRgbSpacePtr& iDefault)
{
	defaultSpace_ = iDefault;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------



// --- free ----------------------------------------------------------------------------------------

Spectrum rgb(const prim::ColorRGBA& iColor)
{
	return xyz(RgbSpace::defaultSpace()->convert(iColor));
}

Spectrum rgb(const prim::ColorRGBA& iColor, const TRgbSpacePtr& iSpace)
{
	return xyz(iSpace->convert(iColor));
}

Spectrum rgb(TScalar red, TScalar green, TScalar blue)
{
	return xyz(RgbSpace::defaultSpace()->convert(prim::ColorRGBA(red, green, blue)));
}

Spectrum rgb(TScalar red, TScalar green, TScalar blue, const TRgbSpacePtr& iSpace)
{
	return xyz(iSpace->convert(prim::ColorRGBA(red, green, blue)));
}
/*
Spectrum rgb(const prim::ColorRGBA& iColor, const TSpectrumFormatPtr& iFormat)
{
	return xyz(RgbSpace::defaultSpace()->convert(iColor), iFormat);
}

Spectrum rgb(const prim::ColorRGBA& iColor, const TRgbSpacePtr& iSpace, const TSpectrumFormatPtr& iFormat)
{
	return xyz(iSpace->convert(iColor), iFormat);
}

Spectrum rgb(TScalar red, TScalar green, TScalar blue, const TSpectrumFormatPtr& iFormat)
{
	return xyz(RgbSpace::defaultSpace()->convert(prim::ColorRGBA(red, green, blue)), iFormat);
}

Spectrum rgb(TScalar red, TScalar green, TScalar blue, const TRgbSpacePtr& iSpace, const TSpectrumFormatPtr& iFormat)
{
	return xyz(iSpace->convert(prim::ColorRGBA(red, green, blue)), iFormat);
}
*/

}
}

// EOF
