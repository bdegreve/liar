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
	PY_CLASS_MEMBER_R(BlackBody, peakWavelength)

namespace
{
	typedef BlackBody::TValue TValue;

	// https://en.wikipedia.org/wiki/Planck's_law#First_and_second_radiation_constants
	constexpr TValue h = 6.626070040e-34f; // planck constant
	constexpr TValue c = 299792458.0f; // speed of light
	constexpr TValue kb = 1.3806488e-23f; // boltzmann constant

	constexpr TValue c1 = 2 * h * c * c;
	constexpr TValue c2 = h * c / kb;

	TValue planck(TValue w, TValue temperature)
	{
		const TValue w5 = num::sqr(num::sqr(w)) * w;
		return c1 / (w5 * num::expm1(c2 / (w * temperature)));
	}
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
	invNorm_ = num::inv(planck(peakWavelength(), temperature));
	update();
}



TValue BlackBody::temperature() const
{
	return temperature_;
}



TValue BlackBody::scale() const
{
	return scale_;
}



TWavelength BlackBody::peakWavelength() const
{
	return 2.897771955e-3f / temperature_;
}


// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------


BlackBody::TValue BlackBody::doCall(TWavelength wavelength) const
{
	return scale_ * invNorm_ * planck(wavelength, temperature_);;
}


const Spectral BlackBody::doEvaluate(const Sample& sample, SpectralType type) const
{
	return Spectral::fromFunc([this](TWavelength w) { return scale_ * invNorm_ * planck(w, temperature_); }, sample, type);
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
