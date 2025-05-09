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

/** @class liar::Recovery
*  @brief Spectral recovery based on work of Brian Smits. (2000)
*  @author Bram de Greve [Bramz]
*
*  Brian Smits (2000),
*  An RGB to Spectrum Conversion for Reflectances.
*/

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SPECTRA_RECOVERY_SMITS_H
#define LIAR_GUARDIAN_OF_INCLUSION_SPECTRA_RECOVERY_SMITS_H

#include "spectra_common.h"
#include "../kernel/recovery.h"
#include "../kernel/rgb_space.h"
#include "../kernel/spectrum.h"

namespace liar
{
namespace spectra
{

class LIAR_SPECTRA_DLL RecoverySmits : public Recovery
{
	PY_HEADER(Recovery)
public:

	RecoverySmits(const TSpectrumRef& red, const TSpectrumRef& green, const TSpectrumRef& blue,
		const TSpectrumRef& yellow, const TSpectrumRef& magenta, const TSpectrumRef& cyan,
		const TSpectrumRef& white, const TRgbSpaceRef& rgbSpace);

private:

	Spectral doRecover(const XYZ& xyz, const Sample& sample, SpectralType type) const override;
	TValue doRecover(const XYZ& xyz, TWavelength wavelength) const override;

	TSpectrumRef red_;
	TSpectrumRef green_;
	TSpectrumRef blue_;
	TSpectrumRef yellow_;
	TSpectrumRef magenta_;
	TSpectrumRef cyan_;
	TSpectrumRef white_;
	TRgbSpaceRef rgbSpace_;
};

}

}

#endif

// EOF
