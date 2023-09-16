/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2023  Bram de Greve (bramz@users.sourceforge.net)
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
#include <lcms2.h>

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS(IccSpace)
PY_CLASS_CONSTRUCTOR_5(IccSpace, const TPoint2D&, const TPoint2D&, const TPoint2D&, const TPoint2D&, TScalar);

namespace
{

template < typename T, typename R, R(CMSEXPORT* destructor) (T) >
class AutoHandle : public util::NonCopyable
{
public:
	explicit AutoHandle(T handle = 0) : handle_(handle) {}
	AutoHandle(AutoHandle&& other) noexcept: handle_(other.handle_) { other.handle_ = 0; }
	~AutoHandle()
	{
		if (handle_)
			destructor(handle_);
	}
	AutoHandle& operator=(AutoHandle&& other) noexcept
	{
		swap(other);
		return *this;
	}
	void swap(AutoHandle& other)
	{
		std::swap(handle_, other.handle_);
	}
	T operator*() const { return handle_; }
	explicit operator bool() { return !!handle_; }
private:
	T handle_;
};

typedef AutoHandle<cmsHPROFILE, cmsBool, cmsCloseProfile> AutoHPROFILE;
typedef AutoHandle<cmsHTRANSFORM, void, cmsDeleteTransform> AutoHTRANSFORM;
typedef AutoHandle<cmsToneCurve*, void, cmsFreeToneCurve> AutoToneCurve;
typedef AutoHandle<cmsContext, void, cmsDeleteContext> AutoContext;

class Context
{
public:
	cmsContext operator*() const
	{
		return *context_;
	}
	static Context& instance()
	{
		static Context context;
		return context;
	}
private:
	Context()
	{
		context_ = AutoContext(cmsCreateContext(0, this));
		cmsSetLogErrorHandlerTHR(*context_, &Context::errorHandler);
	}
	static void errorHandler(cmsContext LASS_UNUSED(contextID), cmsUInt32Number errorCode, const char* text)
	{
		LASS_ASSERT(contextID == *instance().context_);
		LASS_CERR << "LCMS error " << errorCode << ": " << text << std::endl;
	}
	AutoContext context_;
};

template <typename T, cmsUInt32Number flt, cmsUInt32Number dbl> struct SelectFormat;
template <cmsUInt32Number flt, cmsUInt32Number dbl> struct SelectFormat<float, flt, dbl> { enum { value = flt }; };
template <cmsUInt32Number flt, cmsUInt32Number dbl> struct SelectFormat<double, flt, dbl> { enum { value = dbl }; };

void enforceChromaticity(const TPoint2D& chromaticity, const char* name)
{
	if (chromaticity.x < 0 || chromaticity.x > 1 || chromaticity.y < 0 || chromaticity.y > 1)
	{
		LASS_THROW("Invalid " << name << " chromaticity: " << chromaticity << ". Must be in [0,1]x[0,1].");
	}
}

AutoHPROFILE createRGBProfile(const TPoint2D& red, const TPoint2D& green, const TPoint2D& blue, const TPoint2D& white, TScalar gamma)
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

	cmsContext context = *Context::instance();
	AutoToneCurve transferFunction(cmsBuildGamma(context, gamma));
	cmsToneCurve* transferFunctions[3] = { *transferFunction, *transferFunction, *transferFunction };

	return AutoHPROFILE(cmsCreateRGBProfileTHR(context, &whitePoint, &primaries, transferFunctions));
}

}

namespace impl
{

class IccSpaceImpl: public util::NonCopyable
{
public:
	IccSpaceImpl(AutoHPROFILE&& iccProfile): iccProfile_(std::move(iccProfile))
	{
		cmsContext context = *Context::instance();
		const TPoint2D whitepointE(1 / 3.0, 1 / 3.0);
		AutoHPROFILE xyzProfile = createRGBProfile(TPoint2D(1, 0), TPoint2D(0, 1), TPoint2D(0, 0), whitepointE, 1);
		toXYZtransform_ = AutoHTRANSFORM(cmsCreateTransformTHR(context, *iccProfile_, rgbFormat, *xyzProfile, xyzFormat, INTENT_ABSOLUTE_COLORIMETRIC, cmsFLAGS_NOOPTIMIZE));
		fromXYZtransform_ = AutoHTRANSFORM(cmsCreateTransformTHR(context, *xyzProfile, xyzFormat, *iccProfile_, rgbFormat, INTENT_ABSOLUTE_COLORIMETRIC, cmsFLAGS_NOOPTIMIZE));
		if (!toXYZtransform_ || !fromXYZtransform_)
		{
			LASS_THROW("Not a valid RGB ICC profile");
		}
	}
	cmsHPROFILE iccProfile() const { return *iccProfile_; }
	cmsHTRANSFORM toXYZtransform() const { return *toXYZtransform_; }
	cmsHTRANSFORM fromXYZtransform() const { return *fromXYZtransform_; }
private:
	static constexpr cmsUInt32Number rgbFormat = SelectFormat<prim::ColorRGBA::TValue, TYPE_RGB_FLT, TYPE_RGB_DBL>::value;
	static constexpr cmsUInt32Number xyzFormat = SelectFormat<XYZ::TValue, TYPE_RGB_FLT, TYPE_RGB_DBL>::value;

	AutoHPROFILE iccProfile_;
	AutoHTRANSFORM toXYZtransform_;
	AutoHTRANSFORM fromXYZtransform_;
};


void IccSpaceImplDeleter::operator()(IccSpaceImpl* pimpl) const
{
	delete pimpl;
}

}

IccSpace::IccSpace(const void* iccProfileData, num::Tuint32 length)
{
	cmsContext context = *Context::instance();
	AutoHPROFILE iccProfile(cmsOpenProfileFromMemTHR(context, iccProfileData, length));
	pimpl_.reset(new impl::IccSpaceImpl(std::move(iccProfile)));
}


IccSpace::IccSpace(const std::string& iccProfileString):
	IccSpace(&iccProfileString[0], num::numCast<num::Tuint32>(iccProfileString.size()))
{
}


IccSpace::IccSpace(
		const TPoint2D& red, const TPoint2D& green, const TPoint2D& blue,
		const TPoint2D& white, TScalar gamma)
{
	pimpl_.reset(new impl::IccSpaceImpl(createRGBProfile(red, green, blue, white, gamma)));
}


IccSpace::IccSpace(const TRgbSpacePtr& rgbSpace) :
	IccSpace(rgbSpace->red(), rgbSpace->green(), rgbSpace->blue(), rgbSpace->white(), rgbSpace->gamma())
{
}


const XYZ IccSpace::convert(const prim::ColorRGBA& rgb) const
{
	XYZ xyz;
	cmsDoTransform(pimpl_->toXYZtransform(), &rgb, &xyz, 1);
	return xyz;
}



const XYZ IccSpace::convert(const prim::ColorRGBA& rgb, XYZ::TValue& alpha) const
{
	alpha = rgb.a;
	return convert(rgb);
}



const prim::ColorRGBA IccSpace::convert(const XYZ& xyz) const
{
	prim::ColorRGBA rgb;
	cmsDoTransform(pimpl_->fromXYZtransform(), &xyz, &rgb, 1);
	return rgb;
}



const prim::ColorRGBA IccSpace::convert(const XYZ& xyz, XYZ::TValue alpha) const
{
	prim::ColorRGBA rgb = convert(xyz);
	rgb.a = static_cast<prim::ColorRGBA::TValue>(alpha);
	return rgb;
}



void IccSpace::swap(IccSpace& other)
{
	std::swap(pimpl_, other.pimpl_);
}



std::string IccSpace::iccProfile() const
{
	LASS_ASSERT(pimpl_->iccProfile());
	cmsUInt32Number size = 0;
	cmsSaveProfileToMem(pimpl_->iccProfile(), 0, &size);
	LASS_ASSERT(size > 0);
	std::string buffer(size, '\0');
	cmsSaveProfileToMem(pimpl_->iccProfile(), &buffer[0], &size);
	return buffer;
}



namespace
{

TPoint2D chromaticity(TScalar x, TScalar y, TScalar z)
{
	const TScalar sum = x + y + z;
	return sum == 0
		? TPoint2D()
		: TPoint2D(x / sum, y / sum);
}

}


TRgbSpacePtr IccSpace::rgbSpace() const
{
	cmsHPROFILE profile = pimpl_->iccProfile();
	LASS_ASSERT(profile);
	const cmsCIEXYZ* red = static_cast<cmsCIEXYZ *>(cmsReadTag(profile, cmsSigRedColorantTag));
	const cmsCIEXYZ* green = static_cast<cmsCIEXYZ *>(cmsReadTag(profile, cmsSigGreenColorantTag));
	const cmsCIEXYZ* blue = static_cast<cmsCIEXYZ *>(cmsReadTag(profile, cmsSigBlueColorantTag));
	const cmsCIEXYZ* white = static_cast<cmsCIEXYZ *>(cmsReadTag(profile, cmsSigMediaWhitePointTag));
	const auto gamma = static_cast<prim::ColorRGBA::TValue>(cmsDetectRGBProfileGamma(pimpl_->iccProfile(), 0.1));

	if (!red || !green || !blue || !white || gamma < 0)
	{
		LASS_THROW("Not a valid RGB ICC profile");
	}

	const TVector3D W(white->X, white->Y, white->Z);
	const TVector3D D50(cmsD50X, cmsD50Y, cmsD50Z);
	const auto M_CAT_inv = impl::chromaticAdaptationMatrix(D50, W);

	const TTransformation3D M{
		red->X,   green->X,   blue->X,   0,
		red->Y,   green->Y,   blue->Y,   0,
		red->Z,   green->Z,   blue->Z,   0,
		0,        0,          0,         1
	};

	const TTransformation3D M_XYZ = concatenate(M, M_CAT_inv);

	const auto mat = M_XYZ.matrix();
	const TPoint2D r = chromaticity(mat[0], mat[4], mat[8]);
	const TPoint2D g = chromaticity(mat[1], mat[5], mat[9]);
	const TPoint2D b = chromaticity(mat[2], mat[6], mat[10]);
	const TPoint2D w = chromaticity(white->X, white->Y, white->Z);

	return TRgbSpacePtr(new RgbSpace(r, g, b, w, gamma));
}


const TPyObjectPtr IccSpace::reduce() const
{
	return python::makeTuple(
		python::fromNakedToSharedPtrCast<PyObject>(reinterpret_cast<PyObject*>(this->_lassPyGetClassDef()->type())),
		python::makeTuple(), this->getState());
}



const TPyObjectPtr IccSpace::getState() const
{
	return python::makeTuple(this->iccProfile());
}



void IccSpace::setState(const TPyObjectPtr& state)
{
	std::string buffer;
	LASS_ENFORCE(python::decodeTuple(state, buffer));
	IccSpace temp(std::move(buffer));
	swap(temp);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------



// --- free ----------------------------------------------------------------------------------------

}
}

#endif

// EOF
