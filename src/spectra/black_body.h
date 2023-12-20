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

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SPECTRA_BLACK_BODY_H
#define LIAR_GUARDIAN_OF_INCLUSION_SPECTRA_BLACK_BODY_H

#include "spectra_common.h"
#include "../kernel/spectrum.h"

namespace liar
{
namespace spectra
{

class LIAR_SPECTRA_DLL BlackBody: public Spectrum
{
	PY_HEADER(Spectrum)
public:

	explicit BlackBody(TValue temperature);
	BlackBody(TValue temperature, TValue scale);

	TValue temperature() const;
	TValue scale() const;

private:

	TValue doCall(TWavelength wavelength) const override;
	const Spectral doEvaluate(const Sample& sample, SpectralType type) const override;

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

#if LIAR_SPECTRAL_SAMPLE_INDEPENDENT
	void updateValue();
	Spectral value_;
#endif

	TValue temperature_;
	TValue scale_;
	TValue invNorm_;
};

}

}

#endif

// EOF
