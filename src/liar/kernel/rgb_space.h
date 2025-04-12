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

/** @class liar::RgbSpace
 *  @brief transformation from XYZ tristimulus to RGB and vice versa.
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_RGB_SPACE_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_RGB_SPACE_H

#include "kernel_common.h"
#include "xyz.h"
#include "xyza.h"

namespace liar
{
namespace kernel
{

class Spectrum;
typedef PyObjectRef<Spectrum> TSpectrumRef;

class RgbSpace;
typedef python::PyObjectPtr<RgbSpace>::Type TRgbSpacePtr;
typedef PyObjectRef<RgbSpace> TRgbSpaceRef;

class LIAR_KERNEL_DLL RgbSpace: public python::PyObjectPlus
{
	PY_HEADER(python::PyObjectPlus)
public:

	typedef prim::ColorRGBA RGBA;

	RgbSpace(const TPoint2D& red, const TPoint2D& green, const TPoint2D& blue, const TPoint2D& white, RGBA::TValue gamma = 1);


	const XYZA toXYZA(const RGBA& rgba) const;
	const XYZ toXYZ(const RGBA& rgba) const;
	const RGBA toRGBA(const XYZA& xyza) const;
	const RGBA toRGBA(const XYZ& xyz) const;
	const RGBA toRGBA(const XYZ& xyz, XYZ::TValue alpha) const;

	const XYZA toXYZAlinear(const RGBA& rgba) const;
	const XYZ toXYZlinear(const RGBA& rgba) const;
	const RGBA toRGBAlinear(const XYZA& xyza) const;
	const RGBA toRGBAlinear(const XYZ& xyz) const;
	const RGBA toRGBAlinear(const XYZ& xyz, XYZ::TValue alpha) const;

	const RGBA toGamma(const RGBA& rgba) const;
	const RGBA toLinear(const RGBA& rgba) const;

	const TPoint2D& red() const;
	const TPoint2D& green() const;
	const TPoint2D& blue() const;
	const TPoint2D& white() const;
	TScalar gamma() const;

	bool operator==(const RgbSpace& other) const;
	bool operator!=(const RgbSpace& other) const;

	const TRgbSpaceRef linearSpace() const;
	const TRgbSpaceRef withGamma(RGBA::TValue gamma) const;

	const TPyObjectPtr reduce() const;
	const TPyObjectPtr getState() const;
	void setState(const TPyObjectPtr& state);

	static const TRgbSpaceRef& defaultSpace();
	static void setDefaultSpace(const TRgbSpaceRef& defaultSpace);

private:

	std::string doPyRepr() const;

	void init(const TPoint2D& red, const TPoint2D& green, const TPoint2D& blue, const TPoint2D& white, RGBA::TValue gamma);
	void enforceChromaticity(const TPoint2D& c, const char* name) const;

	RGBA x_;
	RGBA y_;
	RGBA z_;
	XYZ r_;
	XYZ g_;
	XYZ b_;
	TPoint2D red_;
	TPoint2D green_;
	TPoint2D blue_;
	TPoint2D white_;
	RGBA::TValue gamma_;
	RGBA::TValue invGamma_;

	static TRgbSpaceRef defaultSpace_;
};


LIAR_KERNEL_DLL TSpectrumRef rgb(const RgbSpace::RGBA& rgb);
LIAR_KERNEL_DLL TSpectrumRef rgb(const RgbSpace::RGBA& rgb, const TRgbSpaceRef& rgbSpace);
LIAR_KERNEL_DLL TSpectrumRef rgb(RgbSpace::RGBA::TValue red, RgbSpace::RGBA::TValue green, RgbSpace::RGBA::TValue blue);
LIAR_KERNEL_DLL TSpectrumRef rgb(RgbSpace::RGBA::TValue red, RgbSpace::RGBA::TValue green, RgbSpace::RGBA::TValue blue, const TRgbSpaceRef& rgbSpace);


// built-in rgb spaces

/** @relates RgbSpace
 */
const TRgbSpaceRef CIEXYZ = TRgbSpaceRef(new RgbSpace(
	TPoint2D(1.0_s, 0.0_s), // red primary
	TPoint2D(0.0_s, 1.0_s), // green primary
	TPoint2D(0.0_s, 0.0_s), // blue primary
	TPoint2D(1.0_s / 3, 1.0_s / 3), // white point
	1.0f)); // gamma

/** @relates RgbSpace
 */
const TRgbSpaceRef sRGB = TRgbSpaceRef(new RgbSpace(
	TPoint2D(0.6400_s, 0.3300_s), // red primary
	TPoint2D(0.3000_s, 0.6000_s), // green primary
	TPoint2D(0.1500_s, 0.0600_s), // blue primary
	TPoint2D(0.3127_s, 0.3290_s), // white point
	2.2f)); // gamma


namespace impl
{
	TTransformation3D chromaticAdaptationMatrix(const TVector3D& from, const TVector3D& to);
}

}

}

#endif

// EOF
