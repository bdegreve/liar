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
#include "icc_space.h"

#ifdef LIAR_HAVE_LCMS2_H
#define CMS_DLL
#include <lcms2.h>

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS(IccSpace)
PY_CLASS_CONSTRUCTOR_5(IccSpace, const TPoint2D&, const TPoint2D&, const TPoint2D&, const TPoint2D&, TScalar);

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
		AutoHPROFILE xyzProfile(cmsCreateXYZProfile());
		toXYZtransform_ = cmsCreateTransform(*iccProfile_, rgbFormat_, *xyzProfile, xyzFormat_, INTENT_PERCEPTUAL, 0);
		fromXYZtransform_ = cmsCreateTransform(*xyzProfile, xyzFormat_, *iccProfile_, rgbFormat_, INTENT_PERCEPTUAL, 0);
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

IccSpace::IccSpace(const std::string& iccProfileString)
{
	cmsUInt32Number size = num::numCast<cmsUInt32Number>(iccProfileString.size());
	impl::AutoHPROFILE iccProfile(cmsOpenProfileFromMem(&iccProfileString[0], size));
	pimpl_ = new impl::IccSpaceImpl(iccProfile);
}


IccSpace::IccSpace(
		const TPoint2D& red, const TPoint2D& green, const TPoint2D& blue,
		const TPoint2D& white, TScalar gamma)
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

	pimpl_ = new impl::IccSpaceImpl(iccProfile);
}


IccSpace::~IccSpace()
{
	delete pimpl_;
}



const XYZ IccSpace::convert(const prim::ColorRGBA& rgb) const
{
	TScalar dummy;
	return convert(rgb, dummy);
}



const XYZ IccSpace::convert(const prim::ColorRGBA& rgb, TScalar& alpha) const
{
	alpha = rgb.a;
	XYZ xyz;
	cmsDoTransform(pimpl_->toXYZtransform(), &rgb, &xyz, 1);
	return xyz;
}



const prim::ColorRGBA IccSpace::convert(const XYZ& xyz) const
{
	return convert(xyz, TScalar(1));
}



const prim::ColorRGBA IccSpace::convert(const XYZ& xyz, TScalar alpha) const
{
	prim::ColorRGBA rgb;
	cmsDoTransform(pimpl_->fromXYZtransform(), &xyz, &rgb, 1);
	rgb.a = static_cast<prim::ColorRGBA::TValue>(alpha);
	return rgb;
}



void IccSpace::swap(IccSpace& other)
{
	std::swap(pimpl_, other.pimpl_);
}



TIccSpacePtr IccSpace::withGamma(TScalar gamma) const
{
}



const TPyObjectPtr IccSpace::reduce() const
{
	return python::makeTuple(
		python::fromNakedToSharedPtrCast<PyObject>(reinterpret_cast<PyObject*>(this->_lassPyGetClassDef()->type())),
		python::makeTuple(), this->getState());
}



const TPyObjectPtr IccSpace::getState() const
{
	LASS_ASSERT(pimpl_->iccProfile());
	cmsUInt32Number size = 0;
	cmsSaveProfileToMem(pimpl_->iccProfile(), 0, &size);
	LASS_ASSERT(size > 0);
	std::string buffer(size, '\0');
	cmsSaveProfileToMem(pimpl_->iccProfile(), &buffer[0], &size);
	return python::makeTuple(buffer);
}



void IccSpace::setState(const TPyObjectPtr& state)
{
	std::string buffer;
	LASS_ENFORCE(python::decodeTuple(state, buffer));
	IccSpace temp(buffer);
	swap(temp);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void IccSpace::enforceChromaticity(const TPoint2D& c, const char* name) const
{
	if (c.x < 0 || c.y < 0 || (c.x + c.y) > 1.001)
	{
		LASS_THROW("invalid chromaticity for " << name << ": " << c
			<< ". Must have x >= 0, y >= 0 and x + y <= 1.");
	}
}



// --- free ----------------------------------------------------------------------------------------

}
}

#endif

// EOF
