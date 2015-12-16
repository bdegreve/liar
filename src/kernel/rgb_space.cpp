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
#include "rgb_space.h"
#include "spectrum.h"
#include <lass/num/impl/matrix_solve.h>

#ifdef LIAR_HAVE_LCMS2_H
#	define CMS_DLL
#	include <lcms2.h>
#endif

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
PY_CLASS_METHOD_QUALIFIED_NAME_1(RgbSpace, convert, const XYZ, const RgbSpace::RGBA&, "toXYZ")
PY_CLASS_METHOD_QUALIFIED_NAME_1(RgbSpace, convert, const RgbSpace::RGBA, const XYZ&, "toRGBA")
PY_CLASS_METHOD_QUALIFIED_NAME_1(RgbSpace, linearConvert, const XYZ, const RgbSpace::RGBA&, "toXYZlinear")
PY_CLASS_METHOD_QUALIFIED_NAME_1(RgbSpace, linearConvert, const RgbSpace::RGBA, const XYZ&, "toRGBAlinear")
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

TRgbSpacePtr RgbSpace::defaultSpace_ = sRGB;

#ifdef LIAR_HAVE_LCMS2_H

namespace impl
{

template < typename T, typename R, R (CMSEXPORT *destructor) (T) >
class AutoHandle
{
public:
	AutoHandle(T handle = 0): handle_(handle) {}
	AutoHandle(AutoHandle& other): handle_(other.handle_) { other.handle_ = 0; }
	~AutoHandle() { if (handle_) destructor(handle_); }
	AutoHandle& operator=(AutoHandle other) { swap(other); return *this; } // by copy
	void swap(AutoHandle& other) { std::swap(handle_, other.handle_); }
	T operator*() const { return handle_; }
private:
	T handle_;
};

typedef AutoHandle<cmsHPROFILE, cmsBool, cmsCloseProfile> AutoHPROFILE;
typedef AutoHandle<cmsHTRANSFORM, void, cmsDeleteTransform> AutoHTRANSFORM;
typedef AutoHandle<cmsToneCurve*, void, cmsFreeToneCurve> AutoToneCurve;

template <typename T, cmsUInt32Number flt, cmsUInt32Number dbl> struct SelectFormat;
template <cmsUInt32Number flt, cmsUInt32Number dbl> struct SelectFormat<float, flt, dbl> { enum { value=flt }; };
template <cmsUInt32Number flt, cmsUInt32Number dbl> struct SelectFormat<double, flt, dbl> { enum { value=dbl }; };

class IccSpaceImpl: public util::NonCopyable
{
public:
	IccSpaceImpl(AutoHPROFILE& iccProfile): iccProfile_(iccProfile)
	{
		cmsUInt32Number flags = cmsFLAGS_NOCACHE;
		AutoHPROFILE xyzProfile(cmsCreateXYZProfile());
		toXYZtransform_ = cmsCreateTransform(*iccProfile_, rgbFormat_, *xyzProfile, xyzFormat_, INTENT_PERCEPTUAL, flags);
		fromXYZtransform_ = cmsCreateTransform(*xyzProfile, xyzFormat_, *iccProfile_, rgbFormat_, INTENT_PERCEPTUAL, flags);
	}
	cmsHPROFILE iccProfile() const { return *iccProfile_; }
	cmsHTRANSFORM toXYZtransform() const { return *toXYZtransform_; }
	cmsHTRANSFORM fromXYZtransform() const { return *fromXYZtransform_; }
private:
	enum
	{
		rgbFormat_ = SelectFormat<prim::ColorRGBA::TValue, TYPE_RGB_FLT, TYPE_RGB_DBL>::value,
		xyzFormat_ = SelectFormat<XYZ::TValue, TYPE_XYZ_FLT, TYPE_XYZ_DBL>::value,
	};
	AutoHPROFILE iccProfile_;
	AutoHTRANSFORM toXYZtransform_;
	AutoHTRANSFORM fromXYZtransform_;
};

}

#endif

RgbSpace::RgbSpace(
		const TPoint2D& red, const TPoint2D& green, const TPoint2D& blue,
		const TPoint2D& white, RGBA::TValue gamma)
{
	init(red, green, blue, white, gamma);
}



const XYZ RgbSpace::convert(const prim::ColorRGBA& rgb) const
{
	TScalar dummy;
	return convert(rgb, dummy);
}



const XYZ RgbSpace::convert(const prim::ColorRGBA& rgba, TScalar& alpha) const
{
#ifdef LIAR_HAVE_LCMS2_H
	alpha = rgba.a;
	XYZ xyz;
	cmsDoTransform(icc_->toXYZtransform(), &rgba, &xyz, 1);
	return xyz;
#else
	return linearConvert(toLinear(rgba), alpha);
#endif
}



const prim::ColorRGBA RgbSpace::convert(const XYZ& xyz) const
{
	return convert(xyz, 1);
}



const prim::ColorRGBA RgbSpace::convert(const XYZ& xyz, TScalar alpha) const
{
#ifdef LIAR_HAVE_LCMS2_H
	prim::ColorRGBA rgb;
	cmsDoTransform(icc_->fromXYZtransform(), &xyz, &rgb, 1);
	rgb.a = static_cast<prim::ColorRGBA::TValue>(alpha);
	return rgb;
#else
	return toGamma(linearConvert(xyz, alpha));
#endif
}



const XYZ RgbSpace::linearConvert(const RGBA& rgb) const
{
	TScalar dummy;
	return linearConvert(rgb, dummy);
}



const XYZ RgbSpace::linearConvert(const RGBA& rgb, TScalar& alpha) const
{
	alpha = rgb.a;
	return r_ * rgb.r + g_ * rgb.g + b_ * rgb.b;
}



const RgbSpace::RGBA RgbSpace::linearConvert(const XYZ& xyz) const
{
	return linearConvert(xyz, 1);
}



const RgbSpace::RGBA RgbSpace::linearConvert(const XYZ& xyz, TScalar alpha) const
{
	RGBA temp = x_ * static_cast<RGBA::TValue>(xyz.x) + y_ * static_cast<RGBA::TValue>(xyz.y) + z_ * static_cast<RGBA::TValue>(xyz.z);
	temp.a = static_cast<RGBA::TValue>(alpha);
	return temp;
}



const RgbSpace::RGBA RgbSpace::toGamma(const RGBA& rgba) const
{
	return RGBA(
		num::pow(std::max<RGBA::TValue>(rgba.r, 0), invGamma_),
		num::pow(std::max<RGBA::TValue>(rgba.g, 0), invGamma_),
		num::pow(std::max<RGBA::TValue>(rgba.b, 0), invGamma_),
		rgba.a);
}



const RgbSpace::RGBA RgbSpace::toLinear(const RGBA& rgba) const
{
	return RGBA(
		num::pow(std::max<RGBA::TValue>(rgba.r, 0), gamma_),
		num::pow(std::max<RGBA::TValue>(rgba.g, 0), gamma_),
		num::pow(std::max<RGBA::TValue>(rgba.b, 0), gamma_),
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



const TRgbSpacePtr RgbSpace::linearSpace() const
{
	return withGamma(1);
}



const TRgbSpacePtr RgbSpace::withGamma(RGBA::TValue gamma) const
{
	if (gamma_ == gamma)
	{
		return TRgbSpacePtr(new RgbSpace(*this));
	}
	return TRgbSpacePtr(new RgbSpace(red_, green_, blue_, white_, gamma));
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

#ifdef LIAR_HAVE_LCMS2_H

	cmsCIExyY whitePoint;
	whitePoint.x = white.x;
	whitePoint.y = white.y;
	whitePoint.Y = 1;

	cmsCIExyYTRIPLE primaries;
	primaries.Red.x = red.x;
	primaries.Red.y = red.y;
	primaries.Red.Y = 1;
	primaries.Green.x = green.x;
	primaries.Green.y = green.y;
	primaries.Green.Y = 1;
	primaries.Blue.x = blue.x;
	primaries.Blue.y = blue.y;
	primaries.Blue.Y = 1;
	
	impl::AutoToneCurve transferFunction = cmsBuildGamma(0, gamma);
	cmsToneCurve* transferFunctions[3] = { *transferFunction, *transferFunction, *transferFunction };

	impl::AutoHPROFILE iccProfile = cmsCreateRGBProfile(&whitePoint, &primaries, transferFunctions);

	icc_ = new impl::IccSpaceImpl(iccProfile);

#else

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
	r_ = XYZ(static_cast<XYZ::TValue>(mat[0]), static_cast<XYZ::TValue>(mat[4]), static_cast<XYZ::TValue>(mat[8]));
	g_ = XYZ(static_cast<XYZ::TValue>(mat[1]), static_cast<XYZ::TValue>(mat[5]), static_cast<XYZ::TValue>(mat[9]));
	b_ = XYZ(static_cast<XYZ::TValue>(mat[2]), static_cast<XYZ::TValue>(mat[6]), static_cast<XYZ::TValue>(mat[10]));

	const TScalar* invMat = M.inverseMatrix();
	x_ = RGBA(static_cast<RGBA::TValue>(invMat[0]), static_cast<RGBA::TValue>(invMat[4]), static_cast<RGBA::TValue>(invMat[8]));
	y_ = RGBA(static_cast<RGBA::TValue>(invMat[1]), static_cast<RGBA::TValue>(invMat[5]), static_cast<RGBA::TValue>(invMat[9]));
	z_ = RGBA(static_cast<RGBA::TValue>(invMat[2]), static_cast<RGBA::TValue>(invMat[6]), static_cast<RGBA::TValue>(invMat[10]));

#endif
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

TSpectrumPtr rgb(const RgbSpace::RGBA& rgba)
{
	return Spectrum::make(RgbSpace::defaultSpace()->convert(rgba));
}

TSpectrumPtr rgb(const RgbSpace::RGBA& rgba, const TRgbSpacePtr& rgbSpace)
{
	return Spectrum::make(rgbSpace->convert(rgba));
}

TSpectrumPtr rgb(RgbSpace::RGBA::TValue red, RgbSpace::RGBA::TValue green, RgbSpace::RGBA::TValue blue)
{
	return Spectrum::make(RgbSpace::defaultSpace()->convert(prim::ColorRGBA(red, green, blue)));
}

TSpectrumPtr rgb(RgbSpace::RGBA::TValue red, RgbSpace::RGBA::TValue green, RgbSpace::RGBA::TValue blue, const TRgbSpacePtr& rgbSpace)
{
	return Spectrum::make(rgbSpace->convert(prim::ColorRGBA(red, green, blue)));
}

}
}

// EOF
