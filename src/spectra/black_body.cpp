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
#include "black_body.h"

namespace liar
{
namespace spectra
{

PY_DECLARE_CLASS_DOC(BlackBody,
	"Black body radiation\n"
	"\n"
	"BlackBody(temperature, scale=1)"
)
	PY_CLASS_CONSTRUCTOR_1(BlackBody, BlackBody::TValue)
	PY_CLASS_CONSTRUCTOR_2(BlackBody, BlackBody::TValue, BlackBody::TValue)
	PY_CLASS_MEMBER_R_DOC(BlackBody, temperature, "temperature in Kelvin")
	PY_CLASS_MEMBER_R(BlackBody, scale)

namespace
{
	typedef BlackBody::TValue TValue;

	// https://en.wikipedia.org/wiki/Planck's_law#First_and_second_radiation_constants
	const TValue h = 6.626070040e-34f; // planck constant
	const TValue c = 299792458.0f; // speed of light
	const TValue kb = 1.3806488e-23f; // boltzmann constant

	const TValue c1 = 2 * h * num::sqr(c);
	const TValue c2 = h * c / kb;
}


// --- public --------------------------------------------------------------------------------------

BlackBody::BlackBody(TValue temperature) :
	BlackBody(temperature, 1)
{
}



BlackBody::BlackBody(TValue temperature, TValue scale) :
	temperature_(temperature),
	scale_(scale)
{
}



TValue BlackBody::temperature() const
{
	return temperature_;
}



TValue BlackBody::scale() const
{
	return scale_;
}


// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------


BlackBody::TValue BlackBody::doCall(TWavelength wavelength) const
{
	const TWavelength w5 = num::sqr(num::sqr(wavelength)) * wavelength;
	return scale_ * static_cast<TValue>(c1 / (w5 * num::expm1(c2 / (wavelength * temperature_))));
}

const Spectral BlackBody::doEvaluate(const Sample& sample, SpectralType type) const
{
	return Spectral::fromFunc([this](TWavelength w) {
		const TWavelength w5 = num::sqr(num::sqr(w)) * w;
		return scale_ * static_cast<TValue>(c1 / (w5 * num::expm1(c2 / (w * temperature_))));
	}, sample, type);
}



BlackBody::TValue BlackBody::doLuminance() const
{
	// we should be able to do this better
	const Observer::TWavelengths& ws = standardObserver().wavelengths();
	TValue acc = 0;
	for (TWavelength w : ws)
	{
		const TWavelength w5 = num::sqr(num::sqr(w)) * w;
		acc += static_cast<TValue>(c1 / (w5 * num::expm1(c2 / (w * temperature_))));
	}
	return scale_ * acc / static_cast<TValue>(ws.size());
}


const TPyObjectPtr BlackBody::doGetState() const
{
	return python::makeTuple(temperature_, scale_);
}



void BlackBody::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, temperature_, scale_);
}



// --- free ----------------------------------------------------------------------------------------


}

}

// EOF
