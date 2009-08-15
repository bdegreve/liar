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

#include "kernel_common.h"
#include "rgb_space.h"
#include <lass/num/impl/matrix_solve.h>

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(RgbSpace, "XYZ-RGB convertor");
PY_CLASS_CONSTRUCTOR_5(RgbSpace, const TPoint2D&, const TPoint2D&, const TPoint2D&, const TPoint2D&, TScalar);
PY_CLASS_MEMBER_R(RgbSpace, red);
PY_CLASS_MEMBER_R(RgbSpace, green);
PY_CLASS_MEMBER_R(RgbSpace, blue);
PY_CLASS_MEMBER_R(RgbSpace, white);
PY_CLASS_MEMBER_R(RgbSpace, gamma);
PY_CLASS_METHOD_NAME(RgbSpace, operator==, python::methods::_eq_);
PY_CLASS_METHOD_NAME(RgbSpace, operator!=, python::methods::_ne_);
PY_CLASS_METHOD_NAME(RgbSpace, reduce, "__reduce__")
PY_CLASS_METHOD_NAME(RgbSpace, getState, "__getstate__")
PY_CLASS_METHOD_NAME(RgbSpace, setState, "__setstate__")
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
		const TPoint2D& white, TScalar gamma)
{
	init(red, green, blue, white, gamma);
}



const XYZ RgbSpace::convert(const prim::ColorRGBA& rgb) const
{
	TScalar dummy;
	return convert(rgb, dummy);
}



const XYZ RgbSpace::convert(const prim::ColorRGBA& rgb, TScalar& alpha) const
{
	alpha = rgb.a;
	return r_ * rgb.r + g_ * rgb.g + b_ * rgb.b;
}



const prim::ColorRGBA RgbSpace::convert(const XYZ& xyz) const
{
	return convert(xyz, TScalar(1));
}



const prim::ColorRGBA RgbSpace::convert(const XYZ& xyz, TScalar alpha) const
{
	return prim::ColorRGBA(
		static_cast<RGBA::TValue>(x_.r * xyz.x + y_.r * xyz.y + z_.r * xyz.z),
		static_cast<RGBA::TValue>(x_.g * xyz.x + y_.g * xyz.y + z_.g * xyz.z),
		static_cast<RGBA::TValue>(x_.b * xyz.x + y_.b * xyz.y + z_.b * xyz.z),
		static_cast<RGBA::TValue>(alpha));
}



const XYZ RgbSpace::convertGamma(const prim::ColorRGBA& rgb) const
{
	TScalar dummy;
	return convertGamma(rgb, dummy);
}



const XYZ RgbSpace::convertGamma(const prim::ColorRGBA& rgb, TScalar& alpha) const
{
	alpha = num::clamp<TScalar>(rgb.a, 0, 1);
	return r_ * num::pow(num::clamp<TScalar>(rgb.r, 0, 1), gamma_) 
		+ g_ * num::pow(num::clamp<TScalar>(rgb.g, 0, 1), gamma_) 
		+ b_ * num::pow(num::clamp<TScalar>(rgb.b, 0, 1), gamma_);
}



const prim::ColorRGBA RgbSpace::convertGamma(const XYZ& xyz) const
{
	return convertGamma(xyz, 1);
}



const prim::ColorRGBA RgbSpace::convertGamma(const XYZ& xyz, TScalar alpha) const
{
	return RGBA(
		num::pow(num::clamp<RGBA::TValue>(static_cast<RGBA::TValue>(x_.r * xyz.x + y_.r * xyz.y + z_.r * xyz.z), 0, 1), invGamma_),
		num::pow(num::clamp<RGBA::TValue>(static_cast<RGBA::TValue>(x_.g * xyz.x + y_.g * xyz.y + z_.g * xyz.z), 0, 1), invGamma_),
		num::pow(num::clamp<RGBA::TValue>(static_cast<RGBA::TValue>(x_.b * xyz.x + y_.b * xyz.y + z_.b * xyz.z), 0, 1), invGamma_),
		num::clamp<RGBA::TValue>(static_cast<RGBA::TValue>(alpha), 0, 1));
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



const TPyObjectPtr RgbSpace::reduce() const
{
	return python::makeTuple(
		python::fromNakedToSharedPtrCast<PyObject>(reinterpret_cast<PyObject*>(this->_lassPyGetClassDef()->type())), 
		python::makeTuple(), this->getState());
}



const TPyObjectPtr RgbSpace::getState() const
{
	return python::makeTuple(red_, green_, blue_, white_, gamma_);
}



void RgbSpace::setState(const TPyObjectPtr& state)
{
	TPoint2D red, green, blue, white;
	TScalar gamma;
	LASS_ENFORCE(python::decodeTuple(state, red, green, blue, white, gamma));
	init(red, green, blue, white, gamma);
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



void RgbSpace::init(
		const TPoint2D& red, const TPoint2D& green, const TPoint2D& blue, const TPoint2D& white,
		TScalar gamma)
{
	enforceChromaticity(red, "red");
	enforceChromaticity(green, "green");
	enforceChromaticity(blue, "blue");
	enforceChromaticity(white, "white");
	if (white.y == 0)
	{
		LASS_THROW("y component of white point should not be zero.");
	}
	if (gamma <= 0)
	{
		LASS_THROW("Invalid gamma: " << gamma << ". Must be strictly positive.");
	}

	// see:
	// http://www.babelcolor.com/download/A%20comparison%20of%20four%20multimedia%20RGB%20spaces.pdf
	// http://www.polybytes.com/misc/Meet_CIECAM02.pdf

	const TScalar zero = 0;
	const TScalar z_r = std::max(1 - red.x - red.y, zero);
	const TScalar z_g = std::max(1 - green.x - green.y, zero);
	const TScalar z_b = std::max(1 - blue.x - blue.y, zero);

	TScalar chromaticities[16] =
	{
		red.x, green.x, blue.x, 0,
		red.y, green.y, blue.y, 0,
		z_r, z_g, z_b, 0,
		0, 0, 0, 1
	};
	const TTransformation3D M_xyz(chromaticities, chromaticities + 16);

	const TVector3D w(
		white.x / white.y,
		1,
		(1 - white.x - white.y) / white.y
	);
	const TVector3D s = transform(w, M_xyz.inverse());
	const TTransformation3D M_XYZ = concatenate(TTransformation3D::scaler(s), M_xyz);

	TScalar bradford[16] =
	{
		 0.8951f,  0.2664f, -0.1614f, 0,
		-0.7502f,  1.7135f,  0.0367f, 0,
		 0.0389f, -0.0685f,  1.0296f, 0,
		 0, 0, 0, 1
	};
	const TTransformation3D M_BFD(bradford, bradford + 16);
	const TVector3D cw = transform(w, M_BFD);

	const TTransformation3D M_CAT = concatenate(
		M_BFD.inverse(),
		concatenate(TTransformation3D::scaler(cw.reciprocal()), M_BFD));

	const TTransformation3D M = concatenate(M_CAT, M_XYZ);

	const TScalar* mat = M.matrix();
	r_ = XYZ(mat[0], mat[4], mat[8]);
	g_ = XYZ(mat[1], mat[5], mat[9]);
	b_ = XYZ(mat[2], mat[6], mat[10]);

	const TScalar* invMat = M.inverse().matrix();
	x_ = RGBA(static_cast<RGBA::TValue>(invMat[0]), static_cast<RGBA::TValue>(invMat[4]), static_cast<RGBA::TValue>(invMat[8]));
	y_ = RGBA(static_cast<RGBA::TValue>(invMat[1]), static_cast<RGBA::TValue>(invMat[5]), static_cast<RGBA::TValue>(invMat[9]));
	z_ = RGBA(static_cast<RGBA::TValue>(invMat[2]), static_cast<RGBA::TValue>(invMat[6]), static_cast<RGBA::TValue>(invMat[10]));

	red_ = red;
	green_ = green;
	blue_ = blue;
	white_ = white;
	gamma_ = gamma;
	invGamma_ = static_cast<RGBA::TValue>(num::inv(gamma));
}



void RgbSpace::enforceChromaticity(const TPoint2D& c, const char* name) const
{
	if (c.x < 0 || c.y < 0 || (c.x + c.y) > 1.001)
	{
		LASS_THROW("invalid chromaticity for " << name << ": " << c
			<< ". Must have x >= 0, y >= 0 and x + y <= 1.");
	}
}



// --- free ----------------------------------------------------------------------------------------

XYZ rgb(const prim::ColorRGBA& rgba)
{
	return RgbSpace::defaultSpace()->convert(rgba);
}

XYZ rgb(const prim::ColorRGBA& rgba, const TRgbSpacePtr& rgbSpace)
{
	return rgbSpace->convert(rgba);
}

XYZ rgb(prim::ColorRGBA::TValue red, prim::ColorRGBA::TValue green, prim::ColorRGBA::TValue blue)
{
	return RgbSpace::defaultSpace()->convert(prim::ColorRGBA(red, green, blue));
}

XYZ rgb(prim::ColorRGBA::TValue red, prim::ColorRGBA::TValue green, prim::ColorRGBA::TValue blue, const TRgbSpacePtr& rgbSpace)
{
	return rgbSpace->convert(prim::ColorRGBA(red, green, blue));
}

}
}

// EOF
