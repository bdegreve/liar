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

/** @class liar::spectra::BlackBody
*  @author Bram de Greve [Bramz]
*/

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SPECTRA_SAMPLED_H
#define LIAR_GUARDIAN_OF_INCLUSION_SPECTRA_SAMPLED_H

#include "spectra_common.h"
#include "../kernel/spectrum.h"

namespace liar
{
namespace spectra
{

class LIAR_SPECTRA_DLL Sampled: public Spectrum
{
	PY_HEADER(Spectrum)
public:

	typedef std::vector<TWavelength> TWavelengths;
	typedef std::vector<TValue> TValues;

	Sampled(const TWavelengths& wavelengths, const TValues& values);

	const TWavelengths& wavelengths() const;
	const TValues& values() const;

	TSpectrumPtr resample(const TWavelengths& wavelengths) const;

private:

	TValue doCall(TWavelength wavelength) const override;
	const Spectral doEvaluate(const Sample& sample, SpectralType type) const override;

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

	TWavelengths wavelengths_;
	TValues values_;
};

}

}

#endif

// EOF
