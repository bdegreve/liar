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

/** @class liar::RgbSpace
 *  @brief transformation from XYZ tristimulus to RGB and vice versa.
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_RGB_SPACE_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_RGB_SPACE_H

#include "kernel_common.h"
#include "spectrum.h"
#include "spectrum_format.h"

namespace liar
{
namespace kernel
{

class RgbSpace;
typedef python::PyObjectPtr<RgbSpace>::Type TRgbSpacePtr;

class LIAR_KERNEL_DLL RgbSpace: public python::PyObjectPlus
{
	PY_HEADER(python::PyObjectPlus)
public:

	RgbSpace();
	RgbSpace(const TVector3D& red, const TVector3D& green, const TVector3D& blue);
	
	const TVector3D convert(const prim::ColorRGBA& rgb) const;
	const prim::ColorRGBA convert(const TVector3D& xyz) const;

	const TVector3D& red() const;
	const TVector3D& green() const;
	const TVector3D& blue() const;

	static const TRgbSpacePtr& defaultSpace();
	static void setDefaultSpace(const TRgbSpacePtr& defaultSpace);

private:

	prim::ColorRGBA x_;
	prim::ColorRGBA y_;
	prim::ColorRGBA z_;
	TVector3D red_;
	TVector3D green_;
	TVector3D blue_;

	static TRgbSpacePtr defaultSpace_;
};

Spectrum rgb(const prim::ColorRGBA& rgb);
Spectrum rgb(const prim::ColorRGBA& rgb, const TRgbSpacePtr& rgbSpace);
Spectrum rgb(TScalar red, TScalar green, TScalar blue);
Spectrum rgb(TScalar red, TScalar green, TScalar blue, const TRgbSpacePtr& rgbSpace);

/*
Spectrum rgb(const prim::ColorRGBA& rgb, 
	const TSpectrumFormatPtr& iSpectrumFormat);
Spectrum rgb(const prim::ColorRGBA& rgb, const TRgbSpacePtr& rgbSpace, 
	const TSpectrumFormatPtr& iSpectrumFormat);
Spectrum rgb(TScalar red, TScalar green, TScalar blue, 
	const TSpectrumFormatPtr& iSpectrumFormat);
Spectrum rgb(TScalar red, TScalar green, TScalar blue, const TRgbSpacePtr& rgbSpace, 
	const TSpectrumFormatPtr& iSpectrumFormat);
*/

// built-in rgb spaces

/** @relates RgbSpace
 */
const RgbSpace rgbSpaceIdentity(
	TVector3D(1, 0, 0), 
	TVector3D(0, 1, 0), 
	TVector3D(0, 0, 1));

/** @relates RgbSpace
 */
const RgbSpace rgbSpaceCie(
	TVector3D(0.490, 0.177, 0.000),
	TVector3D(0.310, 0.812, 0.010), 
	TVector3D(0.200, 0.011, 0.990));

}

}

#endif

// EOF
