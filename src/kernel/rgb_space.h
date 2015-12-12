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

/** @class liar::RgbSpace
 *  @brief transformation from XYZ tristimulus to RGB and vice versa.
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_RGB_SPACE_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_RGB_SPACE_H

#include "kernel_common.h"
#include "xyz.h"


namespace liar
{
namespace kernel
{

class Spectrum;
typedef python::PyObjectPtr<Spectrum>::Type TSpectrumPtr;

class RgbSpace;
typedef python::PyObjectPtr<RgbSpace>::Type TRgbSpacePtr;

#ifdef LIAR_HAVE_LCMS2_H
namespace impl
{
	class IccSpaceImpl;
}
#endif

class LIAR_KERNEL_DLL RgbSpace: public python::PyObjectPlus
{
	PY_HEADER(python::PyObjectPlus)
public:

	typedef prim::ColorRGBA RGBA;

	RgbSpace(const TPoint2D& red, const TPoint2D& green, const TPoint2D& blue, const TPoint2D& white, RGBA::TValue gamma = 1);
	
	const XYZ convert(const RGBA& rgb) const;
	const XYZ convert(const RGBA& rgb, TScalar& alpha) const;
	const RGBA convert(const XYZ& xyz) const;
	const RGBA convert(const XYZ& xyz, TScalar alpha) const;

	const XYZ linearConvert(const RGBA& rgba) const;
	const XYZ linearConvert(const RGBA& rgba, TScalar& alpha) const;
	const RGBA linearConvert(const XYZ& xyz) const;
	const RGBA linearConvert(const XYZ& xyz, TScalar alpha) const;
	const RGBA toGamma(const RGBA& rgba) const;
	const RGBA toLinear(const RGBA& rgba) const;

	const TPoint2D& red() const;
	const TPoint2D& green() const;
	const TPoint2D& blue() const;
	const TPoint2D& white() const;
	TScalar gamma() const;

	bool operator==(const RgbSpace& other) const;
	bool operator!=(const RgbSpace& other) const;

	const TRgbSpacePtr linearSpace() const;
	const TRgbSpacePtr withGamma(RGBA::TValue gamma) const;

	const TPyObjectPtr reduce() const;
	const TPyObjectPtr getState() const;
	void setState(const TPyObjectPtr& state);

	static const TRgbSpacePtr& defaultSpace();
	static void setDefaultSpace(const TRgbSpacePtr& defaultSpace);

private:

	std::string doPyRepr() const;

	void init(const TPoint2D& red, const TPoint2D& green, const TPoint2D& blue, const TPoint2D& white, RGBA::TValue gamma);
	void enforceChromaticity(const TPoint2D& c, const char* name) const;

#ifdef LIAR_HAVE_LCMS2_H
	impl::IccSpaceImpl* icc_;
#else
	RGBA x_;
	RGBA y_;
	RGBA z_;
	XYZ r_;
	XYZ g_;
	XYZ b_;
#endif
	TPoint2D red_;
	TPoint2D green_;
	TPoint2D blue_;
	TPoint2D white_;
	RGBA::TValue gamma_;
	RGBA::TValue invGamma_;

	static TRgbSpacePtr defaultSpace_;
};


LIAR_KERNEL_DLL TSpectrumPtr rgb(const RgbSpace::RGBA& rgb);
LIAR_KERNEL_DLL TSpectrumPtr rgb(const RgbSpace::RGBA& rgb, const TRgbSpacePtr& rgbSpace);
LIAR_KERNEL_DLL TSpectrumPtr rgb(RgbSpace::RGBA::TValue red, RgbSpace::RGBA::TValue green, RgbSpace::RGBA::TValue blue);
LIAR_KERNEL_DLL TSpectrumPtr rgb(RgbSpace::RGBA::TValue red, RgbSpace::RGBA::TValue green, RgbSpace::RGBA::TValue blue, const TRgbSpacePtr& rgbSpace);


// built-in rgb spaces

/** @relates RgbSpace
 */
const TRgbSpacePtr CIEXYZ = TRgbSpacePtr(new RgbSpace(
	TPoint2D(1, 0), // red primary 
	TPoint2D(0, 1), // green primary
	TPoint2D(0, 0), // blue primary
	TPoint2D(TScalar(1) / 3, TScalar(1) / 3), // white point
	1)); // gamma

/** @relates RgbSpace
 */
const TRgbSpacePtr sRGB = TRgbSpacePtr(new RgbSpace(
	TPoint2D(0.6400f, 0.3300f), // red primary 
	TPoint2D(0.3000f, 0.6000f), // green primary
	TPoint2D(0.1500f, 0.0600f), // blue primary
	TPoint2D(0.3127f, 0.3290f), // white point
	2.2f)); // gamma

}

}

#endif

// EOF
