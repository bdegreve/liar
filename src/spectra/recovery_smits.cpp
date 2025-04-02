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

#include "spectra_common.h"
#include "recovery_smits.h"

namespace liar
{
namespace spectra
{

PY_DECLARE_CLASS_DOC(RecoverySmits, "spectral recovery using Smits (2000)")
	PY_CLASS_CONSTRUCTOR_8(RecoverySmits,
		const TSpectrumPtr&, const TSpectrumPtr&, const TSpectrumPtr&,
		const TSpectrumPtr&, const TSpectrumPtr&, const TSpectrumPtr&,
		const TSpectrumPtr&, const TRgbSpacePtr&)


// --- public --------------------------------------------------------------------------------------

RecoverySmits::RecoverySmits(const TSpectrumPtr& red, const TSpectrumPtr& green, const TSpectrumPtr& blue,
		const TSpectrumPtr& yellow, const TSpectrumPtr& magenta, const TSpectrumPtr& cyan,
		const TSpectrumPtr& white,
		const TRgbSpacePtr& rgbSpace) :
	red_(red),
	green_(green),
	blue_(blue),
	yellow_(yellow),
	magenta_(magenta),
	cyan_(cyan),
	white_(white),
	rgbSpace_(rgbSpace)
{
}


// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

Spectral RecoverySmits::doRecover(const XYZ& xyz, const Sample& sample, SpectralType type) const
{
	const prim::ColorRGBA rgb = rgbSpace_->toRGBAlinear(xyz);

	Spectral result = white_->evaluate(sample, type);
	if (rgb.r <= rgb.g)
	{
		if (rgb.r <= rgb.b)
		{
			result *= rgb.r;
			if (rgb.g <= rgb.b)
			{
				LASS_ASSERT(rgb.r <= rgb.g && rgb.r <= rgb.b && rgb.g <= rgb.b);
				result.fma(rgb.g - rgb.r, cyan_->evaluate(sample, type));
				result.fma(rgb.b - rgb.g, blue_->evaluate(sample, type));
			}
			else
			{
				LASS_ASSERT(rgb.r <= rgb.b && rgb.r <= rgb.g && rgb.b <= rgb.g);
				result.fma(rgb.b - rgb.r, cyan_->evaluate(sample, type));
				result.fma(rgb.g - rgb.b, green_->evaluate(sample, type));
			}
		}
		else
		{
			LASS_ASSERT(rgb.b <= rgb.r && rgb.b <= rgb.g && rgb.r <= rgb.g);
			result *= rgb.b;
			result.fma(rgb.r - rgb.b, yellow_->evaluate(sample, type));
			result.fma(rgb.g - rgb.r, green_->evaluate(sample, type));
		}
	}
	else if (rgb.g <= rgb.b)
	{
		result *= rgb.g;
		if (rgb.r <= rgb.b)
		{
			LASS_ASSERT(rgb.g <= rgb.r && rgb.g <= rgb.b && rgb.r <= rgb.b);
			result.fma(rgb.r - rgb.g, magenta_->evaluate(sample, type));
			result.fma(rgb.b - rgb.r, blue_->evaluate(sample, type));
		}
		else
		{
			LASS_ASSERT(rgb.g <= rgb.b && rgb.g <= rgb.r && rgb.b <= rgb.r);
			result.fma(rgb.b - rgb.g, magenta_->evaluate(sample, type));
			result.fma(rgb.r - rgb.b, red_->evaluate(sample, type));
		}
	}
	else
	{
		LASS_ASSERT(rgb.b <= rgb.g && rgb.b <= rgb.r && rgb.g <= rgb.r);
		result *= rgb.b;
		result.fma(rgb.g - rgb.b, yellow_->evaluate(sample, type));
		result.fma(rgb.r - rgb.g, red_->evaluate(sample, type));
	}

	return result;
}



Recovery::TValue RecoverySmits::doRecover(const XYZ& xyz, TWavelength wavelength) const
{
	const prim::ColorRGBA rgb = rgbSpace_->toRGBAlinear(xyz);

	TValue result = (*white_)(wavelength);
	if (rgb.r <= rgb.g)
	{
		if (rgb.r <= rgb.b)
		{
			result *= rgb.r;
			if (rgb.g <= rgb.b)
			{
				LASS_ASSERT(rgb.r <= rgb.g && rgb.r <= rgb.b && rgb.g <= rgb.b);
				result += (rgb.g - rgb.r) * (*cyan_)(wavelength);
				result += (rgb.b - rgb.g) * (*blue_)(wavelength);
			}
			else
			{
				LASS_ASSERT(rgb.r <= rgb.b && rgb.r <= rgb.g && rgb.b <= rgb.g);
				result += (rgb.b - rgb.r) * (*cyan_)(wavelength);
				result += (rgb.g - rgb.b) * (*green_)(wavelength);
			}
		}
		else
		{
			LASS_ASSERT(rgb.b <= rgb.r && rgb.b <= rgb.g && rgb.r <= rgb.g);
			result *= rgb.b;
			result += (rgb.r - rgb.b) * (*yellow_)(wavelength);
			result += (rgb.g - rgb.r) * (*green_)(wavelength);
		}
	}
	else if (rgb.g <= rgb.b)
	{
		result *= rgb.g;
		if (rgb.r <= rgb.b)
		{
			LASS_ASSERT(rgb.g <= rgb.r && rgb.g <= rgb.b && rgb.r <= rgb.b);
			result += (rgb.r - rgb.g) * (*magenta_)(wavelength);
			result += (rgb.b - rgb.r) * (*blue_)(wavelength);
		}
		else
		{
			LASS_ASSERT(rgb.g <= rgb.b && rgb.g <= rgb.r && rgb.b <= rgb.r);
			result += (rgb.b - rgb.g) * (*magenta_)(wavelength);
			result += (rgb.r - rgb.b) * (*red_)(wavelength);
		}
	}
	else
	{
		LASS_ASSERT(rgb.b <= rgb.g && rgb.b <= rgb.r && rgb.g <= rgb.r);
		result *= rgb.b;
		result += (rgb.g - rgb.b) * (*yellow_)(wavelength);
		result += (rgb.r - rgb.g) * (*red_)(wavelength);
	}

	return result;
}

}
}

// EOF
