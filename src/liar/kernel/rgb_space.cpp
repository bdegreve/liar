/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2025  Bram de Greve (bramz@users.sourceforge.net)
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
#include "spectrum.h"
#include <lass/num/impl/matrix_solve.h>

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(RgbSpace,
	"XYZ-RGB convertor\n"
	"RgbSpace((x_red, y_red), (x_green, y_green), (x_blue, y_blue), (x_white, y_white), gamma)"
	);
PY_CLASS_CONSTRUCTOR_5(RgbSpace, const TPoint2D&, const TPoint2D&, const TPoint2D&, const TPoint2D&, RgbSpace::RGBA::TValue);
PY_CLASS_MEMBER_R_DOC(RgbSpace, red, "(x_red, y_red)");
PY_CLASS_MEMBER_R_DOC(RgbSpace, green, "(x_green, y_green)");
PY_CLASS_MEMBER_R_DOC(RgbSpace, blue, "(x_blue, y_blue)");
PY_CLASS_MEMBER_R_DOC(RgbSpace, white, "(x_white, y_white)");
PY_CLASS_MEMBER_R_DOC(RgbSpace, gamma, "float");
PY_CLASS_METHOD_QUALIFIED_NAME_1(RgbSpace, toXYZ, const XYZ, const RgbSpace::RGBA&, "toXYZ")
PY_CLASS_METHOD_QUALIFIED_NAME_1(RgbSpace, toRGBA, const RgbSpace::RGBA, const XYZ&, "toRGBA")
PY_CLASS_METHOD_QUALIFIED_NAME_1(RgbSpace, toXYZlinear, const XYZ, const RgbSpace::RGBA&, "toXYZlinear")
PY_CLASS_METHOD_QUALIFIED_NAME_1(RgbSpace, toRGBAlinear, const RgbSpace::RGBA, const XYZ&, "toRGBAlinear")
PY_CLASS_METHOD_DOC(RgbSpace, linearSpace, "return RGB space with same chromaticities, but with gamma=1");
PY_CLASS_METHOD_DOC(RgbSpace, withGamma, "return RGB space with same chromaticities, but with another gamma");
PY_CLASS_METHOD_NAME(RgbSpace, operator==, python::methods::_eq_);
PY_CLASS_METHOD_NAME(RgbSpace, operator!=, python::methods::_ne_);
PY_CLASS_METHOD_NAME(RgbSpace, reduce, "__reduce__")
PY_CLASS_METHOD_NAME(RgbSpace, getState, "__getstate__")
PY_CLASS_METHOD_NAME(RgbSpace, setState, "__setstate__")
PY_CLASS_STATIC_METHOD_DOC(RgbSpace, defaultSpace, "defaultSpace() -> RgbSpace");
PY_CLASS_STATIC_METHOD_DOC(RgbSpace, setDefaultSpace, "setDefaultSpace(RgbSpace) -> None");

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

TRgbSpaceRef RgbSpace::defaultSpace_ = sRGB;

RgbSpace::RgbSpace(
		const TPoint2D& red, const TPoint2D& green, const TPoint2D& blue,
		const TPoint2D& white, RGBA::TValue gamma)
{
	init(red, green, blue, white, gamma);
}



const XYZA RgbSpace::toXYZA(const prim::ColorRGBA& rgba) const
{
	return toXYZAlinear(toLinear(rgba));
}



const XYZ RgbSpace::toXYZ(const prim::ColorRGBA& rgba) const
{
	return toXYZlinear(toLinear(rgba));
}



const prim::ColorRGBA RgbSpace::toRGBA(const XYZA& xyza) const
{
	return toGamma(toRGBAlinear(xyza));
}



const prim::ColorRGBA RgbSpace::toRGBA(const XYZ& xyz) const
{
	return toGamma(toRGBAlinear(xyz, 1));
}



const prim::ColorRGBA RgbSpace::toRGBA(const XYZ& xyz, XYZ::TValue alpha) const
{
	return toGamma(toRGBAlinear(xyz, alpha));
}



const XYZA RgbSpace::toXYZAlinear(const RGBA& rgba) const
{
	return XYZA(toXYZlinear(rgba), rgba.a);
}



const XYZ RgbSpace::toXYZlinear(const RGBA& rgba) const
{
	return r_ * rgba.r + g_ * rgba.g + b_ * rgba.b;
}



const RgbSpace::RGBA RgbSpace::toRGBAlinear(const XYZA& xyza) const
{
	RGBA temp = x_ * static_cast<RGBA::TValue>(xyza.x) + y_ * static_cast<RGBA::TValue>(xyza.y) + z_ * static_cast<RGBA::TValue>(xyza.z);
	temp.a = static_cast<RGBA::TValue>(xyza.a);
	return temp;
}



const RgbSpace::RGBA RgbSpace::toRGBAlinear(const XYZ& xyz) const
{
	return toRGBAlinear(XYZA(xyz, 1));
}



const RgbSpace::RGBA RgbSpace::toRGBAlinear(const XYZ& xyz, XYZ::TValue alpha) const
{
	return toRGBAlinear(XYZA(xyz, alpha));
}


namespace
{

template <typename T>
T tonecurve(T x, T gamma)
{
	return x < 0
		? -num::pow(-x, gamma)
		: num::pow(x, gamma);
}

}


const RgbSpace::RGBA RgbSpace::toGamma(const RGBA& rgba) const
{
	if (gamma_ == 1)
		return rgba;
	return RGBA(
		tonecurve(rgba.r, invGamma_),
		tonecurve(rgba.g, invGamma_),
		tonecurve(rgba.b, invGamma_),
		rgba.a);
}



const RgbSpace::RGBA RgbSpace::toLinear(const RGBA& rgba) const
{
	if (gamma_ == 1)
		return rgba;
	return RGBA(
		tonecurve(rgba.r, gamma_),
		tonecurve(rgba.g, gamma_),
		tonecurve(rgba.b, gamma_),
		rgba.a);
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



TScalar RgbSpace::gamma() const
{
	return gamma_;
}



bool RgbSpace::operator==(const RgbSpace& other) const
{
	return red_ == other.red_ && green_ == other.green_ && blue_ == other.blue_ && white_ == other.white_ && gamma_ == other.gamma_;
}



bool RgbSpace::operator!=(const RgbSpace& other) const
{
	return !(*this == other);
}



const TRgbSpaceRef RgbSpace::linearSpace() const
{
	return withGamma(1);
}



const TRgbSpaceRef RgbSpace::withGamma(RGBA::TValue gamma) const
{
	if (gamma_ == gamma)
	{
		return TRgbSpaceRef(new RgbSpace(*this));
	}
	return TRgbSpaceRef(new RgbSpace(red_, green_, blue_, white_, gamma));
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
	RGBA::TValue gamma;
	LASS_ENFORCE(python::decodeTuple(state, red, green, blue, white, gamma));
	init(red, green, blue, white, gamma);
}



const TRgbSpaceRef& RgbSpace::defaultSpace()
{
	return defaultSpace_;
}


void RgbSpace::setDefaultSpace(const TRgbSpaceRef& iDefault)
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
		RGBA::TValue gamma)
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

	red_ = red;
	green_ = green;
	blue_ = blue;
	white_ = white;
	gamma_ = gamma;
	invGamma_ = static_cast<RGBA::TValue>(num::inv(gamma));

	// see:
	// http://www.babelcolor.com/download/A%20comparison%20of%20four%20multimedia%20RGB%20spaces.pdf
	// http://www.polybytes.com/misc/Meet_CIECAM02.pdf

	// first, make matrix M_XYZ that will transform from linear (R,G,B) to (X,Y,Z)

	const TScalar z_r = 1 - red.x - red.y;
	const TScalar z_g = 1 - green.x - green.y;
	const TScalar z_b = 1 - blue.x - blue.y;

	TScalar chromaticities[16] =
	{
		red.x, green.x, blue.x, 0,
		red.y, green.y, blue.y, 0,
		z_r,   z_g,     z_b,    0,
		0,     0,       0,      1
	};
	const TTransformation3D M_xyz(chromaticities, chromaticities + 16);

	const TVector3D W(
		white.x / white.y,
		1,
		(1 - white.x - white.y) / white.y
	);

	const TVector3D S = transform(W, M_xyz.inverse());
	const TTransformation3D M_S = TTransformation3D::scaler(S);
	const TTransformation3D M_XYZ = concatenate(M_S, M_xyz); // = M_xyz * M_S. M_S is applied first

	// then, make matrix that does chromatic adaptation from whitepoint W to whitepoint E

	const TVector3D E(1, 1, 1);
	const TTransformation3D M_CAT = impl::chromaticAdaptationMatrix(W, E);

	// M converts colors from RGB (with whitepoint W) to XYZ (CIE E)
	const TTransformation3D M = concatenate(M_XYZ, M_CAT);

	const TScalar* mat = M.matrix();
	r_ = XYZ(static_cast<XYZ::TValue>(mat[0]), static_cast<XYZ::TValue>(mat[4]), static_cast<XYZ::TValue>(mat[8]));
	g_ = XYZ(static_cast<XYZ::TValue>(mat[1]), static_cast<XYZ::TValue>(mat[5]), static_cast<XYZ::TValue>(mat[9]));
	b_ = XYZ(static_cast<XYZ::TValue>(mat[2]), static_cast<XYZ::TValue>(mat[6]), static_cast<XYZ::TValue>(mat[10]));

	const TScalar* invMat = M.inverseMatrix();
	x_ = RGBA(static_cast<RGBA::TValue>(invMat[0]), static_cast<RGBA::TValue>(invMat[4]), static_cast<RGBA::TValue>(invMat[8]));
	y_ = RGBA(static_cast<RGBA::TValue>(invMat[1]), static_cast<RGBA::TValue>(invMat[5]), static_cast<RGBA::TValue>(invMat[9]));
	z_ = RGBA(static_cast<RGBA::TValue>(invMat[2]), static_cast<RGBA::TValue>(invMat[6]), static_cast<RGBA::TValue>(invMat[10]));
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

TSpectrumRef rgb(const RgbSpace::RGBA& rgba)
{
	return Spectrum::make(RgbSpace::defaultSpace()->toXYZ(rgba));
}

TSpectrumRef rgb(const RgbSpace::RGBA& rgba, const TRgbSpaceRef& rgbSpace)
{
	return Spectrum::make(rgbSpace->toXYZ(rgba));
}

TSpectrumRef rgb(RgbSpace::RGBA::TValue red, RgbSpace::RGBA::TValue green, RgbSpace::RGBA::TValue blue)
{
	return Spectrum::make(RgbSpace::defaultSpace()->toXYZ(prim::ColorRGBA(red, green, blue)));
}

TSpectrumRef rgb(RgbSpace::RGBA::TValue red, RgbSpace::RGBA::TValue green, RgbSpace::RGBA::TValue blue, const TRgbSpaceRef& rgbSpace)
{
	return Spectrum::make(rgbSpace->toXYZ(prim::ColorRGBA(red, green, blue)));
}

namespace impl
{

TTransformation3D chromaticAdaptationMatrix(const TVector3D& from, const TVector3D& to)
{
	// bradford matrix
	const static TTransformation3D M_BFD {
		 0.8951_s,  0.2664_s, -0.1614_s, 0._s,
		-0.7502_s,  1.7135_s,  0.0367_s, 0._s,
		 0.0389_s, -0.0685_s,  1.0296_s, 0._s,
		 0._s,      0._s,      0._s,     1._s
	};

	const TVector3D C_s = transform(from, M_BFD);
	const TVector3D C_d = transform(to, M_BFD);
	const TTransformation3D M_c = TTransformation3D::scaler(C_d / C_s);

	// M_CAT = M_BFD^-1 * M_c * M_BFD, thus M_BFD is applied first, then M_c and finally M_BFD^-1
	const TTransformation3D M_CAT = concatenate(concatenate(M_BFD, M_c), M_BFD.inverse());

	return M_CAT;
}

}

}
}

// EOF
