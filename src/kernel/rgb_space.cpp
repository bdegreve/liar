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
PY_CLASS_MEMBER_R(RgbSpace, "red", red);
PY_CLASS_MEMBER_R(RgbSpace, "green", green);
PY_CLASS_MEMBER_R(RgbSpace, "blue", blue);
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




RgbSpace::RgbSpace(const TVector3D& iRed, const TVector3D& iGreen, const TVector3D& iBlue):
	python::PyObjectPlus(&Type),
	red_(iRed),
	green_(iGreen),
	blue_(iBlue),
	x_(1, 0, 0),
	y_(0, 1, 0),
	z_(0, 0, 1)
{
	TScalar matrix[9] =
	{
		red_.x, green_.x, blue_.x,
		red_.y, green_.y, blue_.y,
		red_.z, green_.z, blue_.z
	};

	if (!num::impl::cramer3<TScalar>(matrix, x_))
	{
		LASS_THROW("RGB space forms a singular matrix, cannot invert.");
	}

	num::impl::cramer3<TScalar>(matrix, y_);
	num::impl::cramer3<TScalar>(matrix, z_);
}



const TVector3D RgbSpace::convert(const prim::ColorRGBA& iRgb) const
{
	return red_ * iRgb.r + green_ * iRgb.g + blue_ * iRgb.b;
}



const prim::ColorRGBA RgbSpace::convert(const TVector3D& iXyz) const
{
	return prim::ColorRGBA(
		x_.r * iXyz.x + y_.r * iXyz.y + z_.r * iXyz.z,
		x_.g * iXyz.x + y_.g * iXyz.y + z_.g * iXyz.z,
		x_.b * iXyz.x + y_.b * iXyz.y + z_.b * iXyz.z,
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

Spectrum rgb(TScalar iRed, TScalar iGreen, TScalar iBlue)
{
	return xyz(RgbSpace::defaultSpace()->convert(prim::ColorRGBA(iRed, iGreen, iBlue)));
}

Spectrum rgb(TScalar iRed, TScalar iGreen, TScalar iBlue, const TRgbSpacePtr& iSpace)
{
	return xyz(iSpace->convert(prim::ColorRGBA(iRed, iGreen, iBlue)));
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

Spectrum rgb(TScalar iRed, TScalar iGreen, TScalar iBlue, const TSpectrumFormatPtr& iFormat)
{
	return xyz(RgbSpace::defaultSpace()->convert(prim::ColorRGBA(iRed, iGreen, iBlue)), iFormat);
}

Spectrum rgb(TScalar iRed, TScalar iGreen, TScalar iBlue, const TRgbSpacePtr& iSpace, const TSpectrumFormatPtr& iFormat)
{
	return xyz(iSpace->convert(prim::ColorRGBA(iRed, iGreen, iBlue)), iFormat);
}
*/

}
}

// EOF
