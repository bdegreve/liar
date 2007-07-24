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
 *  http://liar.bramz.org
 */

#include "kernel_common.h"
#include "rgb_space.h"
#include <lass/num/impl/matrix_solve.h>

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS(RgbSpace);
PY_CLASS_CONSTRUCTOR_5(RgbSpace, const TPoint2D&, const TPoint2D&, const TPoint2D&, const TPoint2D&, TScalar);
PY_CLASS_MEMBER_R(RgbSpace, red);
PY_CLASS_MEMBER_R(RgbSpace, green);
PY_CLASS_MEMBER_R(RgbSpace, blue);
PY_CLASS_MEMBER_R(RgbSpace, white);
PY_CLASS_MEMBER_R(RgbSpace, gamma);
PY_CLASS_METHOD_NAME(RgbSpace, operator==, "__eq__");
PY_CLASS_METHOD_NAME(RgbSpace, operator!=, "__ne__");
PY_CLASS_STATIC_METHOD(RgbSpace, defaultSpace);
PY_CLASS_STATIC_METHOD(RgbSpace, setDefaultSpace);

//// load default RGB space.  We use the sRGB space here.
//// you can find it here: http://www.brucelindbloom.com/index.html?WorkingSpaceInfo.html
////
//TRgbSpacePtr RgbSpace::defaultSpace_ = TRgbSpacePtr(new RgbSpace(
//	TVector3D(0.490, 0.177, 0.000),
//	TVector3D(0.310, 0.812, 0.010), 
//	TVector3D(0.200, 0.011, 0.990)));

//TRgbSpacePtr RgbSpace::defaultSpace_ = TRgbSpacePtr(new RgbSpace(
//	TVector3D(1.f, 0.f, 0.f),
//	TVector3D(0.f, 1.f, 0.f), 
//	TVector3D(0.f, 0.f, 1.f)));

TRgbSpacePtr RgbSpace::defaultSpace_ = sRGB;



RgbSpace::RgbSpace(
		const TPoint2D& red, const TPoint2D& green, const TPoint2D& blue, 
		const TPoint2D& white, TScalar gamma):
	r_(red.x , red.y, 1 - red.x - red.y),
	g_(green.x , green.y, 1 - green.x - green.y),
	b_(blue.x , blue.y, 1 - blue.x - blue.y),
	red_(red),
	green_(green),
	blue_(blue),
	white_(white),
	gamma_(gamma),
	invGamma_(num::inv(gamma))
{
	// rescale for white

	TScalar primaries[9] =
	{
		r_.x, g_.x, b_.x,
		r_.y, g_.y, b_.y,
		r_.z, g_.z, b_.z
	};
	TScalar w[9] =
	{
		white.x / white.y,
		1,
		(1 - white.x - white.y) / white.y
	};
	if (!num::impl::cramer3<TScalar>(primaries, w, w + 3))
	{
		LASS_THROW("RGB primaries form a singular matrix, cannot solve.");
	}
	
	r_ *= w[0];
	g_ *= w[1];
	b_ *= w[2];

	// invert matrix

	TScalar rgb2xyz[9] =
	{
		r_.x, g_.x, b_.x,
		r_.y, g_.y, b_.y,
		r_.z, g_.z, b_.z
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
	TScalar dummy;
	return convert(rgb, dummy);
}



const TVector3D RgbSpace::convert(const prim::ColorRGBA& rgb, TScalar& alpha) const
{
	alpha = rgb.a;
	return r_ * rgb.r + g_ * rgb.g + b_ * rgb.b;
}



const prim::ColorRGBA RgbSpace::convert(const TVector3D& xyz) const
{
	return convert(xyz, TScalar(1));
}



const prim::ColorRGBA RgbSpace::convert(const TVector3D& xyz, TScalar alpha) const
{
	return prim::ColorRGBA(
		x_.r * xyz.x + y_.r * xyz.y + z_.r * xyz.z,
		x_.g * xyz.x + y_.g * xyz.y + z_.g * xyz.z,
		x_.b * xyz.x + y_.b * xyz.y + z_.b * xyz.z,
		alpha);
}



const TVector3D RgbSpace::convertGamma(const prim::ColorRGBA& rgb) const
{
	TScalar dummy;
	return convertGamma(rgb, dummy);
}



const TVector3D RgbSpace::convertGamma(const prim::ColorRGBA& rgb, TScalar& alpha) const
{
	alpha = num::clamp<TScalar>(rgb.a, 0, 1);
	return r_ * num::pow(num::clamp<TScalar>(rgb.r, 0, 1), gamma_) 
		+ g_ * num::pow(num::clamp<TScalar>(rgb.g, 0, 1), gamma_) 
		+ b_ * num::pow(num::clamp<TScalar>(rgb.b, 0, 1), gamma_);
}



const prim::ColorRGBA RgbSpace::convertGamma(const TVector3D& xyz) const
{
	return convertGamma(xyz, TScalar(1));
}



const prim::ColorRGBA RgbSpace::convertGamma(const TVector3D& xyz, TScalar alpha) const
{
	return prim::ColorRGBA(
		num::pow(num::clamp<TScalar>(x_.r * xyz.x + y_.r * xyz.y + z_.r * xyz.z, 0, 1), invGamma_),
		num::pow(num::clamp<TScalar>(x_.g * xyz.x + y_.g * xyz.y + z_.g * xyz.z, 0, 1), invGamma_),
		num::pow(num::clamp<TScalar>(x_.b * xyz.x + y_.b * xyz.y + z_.b * xyz.z, 0, 1), invGamma_),
		num::clamp<TScalar>(alpha, 0, 1));
}



const TPoint2D& RgbSpace::red() const
{
	return red_;
}



const TPoint2D& RgbSpace::green() const
{
	return green_;
}



const TPoint2D& RgbSpace::blue() const
{
	return blue_;
}



const TPoint2D& RgbSpace::white() const
{
	return white_;
}



const TScalar RgbSpace::gamma() const
{
	return gamma_;
}



bool RgbSpace::operator==(const RgbSpace& other) const
{
	return r_ == other.r_ && g_ == other.g_ && b_ == other.b_;
}



bool RgbSpace::operator!=(const RgbSpace& other) const
{
	return !(*this == other);
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

std::string RgbSpace::doPyRepr() const
{
	std::ostringstream buffer;
	buffer << "liar.RgbSpace(" << red_ << ", " << green_ << ", " << blue_ << ", " 
		<< white_ << ", " << gamma_ << ")";
	return buffer.str();
}



// --- free ----------------------------------------------------------------------------------------

Spectrum rgb(const prim::ColorRGBA& rgba)
{
	return xyz(RgbSpace::defaultSpace()->convert(rgba));
}

Spectrum rgb(const prim::ColorRGBA& rgba, const TRgbSpacePtr& rgbSpace)
{
	return xyz(rgbSpace->convert(rgba));
}

Spectrum rgb(TScalar red, TScalar green, TScalar blue)
{
	return xyz(RgbSpace::defaultSpace()->convert(prim::ColorRGBA(red, green, blue)));
}

Spectrum rgb(TScalar red, TScalar green, TScalar blue, const TRgbSpacePtr& rgbSpace)
{
	return xyz(rgbSpace->convert(prim::ColorRGBA(red, green, blue)));
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
