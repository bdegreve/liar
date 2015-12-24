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

#include "spectra_common.h"
#include "black_body.h"

namespace liar
{
namespace spectra
{

PY_DECLARE_CLASS_DOC(BlackBody, "Black body radiation")
	PY_CLASS_CONSTRUCTOR_1(BlackBody, BlackBody::TValue)
	PY_CLASS_MEMBER_RW(BlackBody, temperature, setTemperature)
	PY_CLASS_MEMBER_RW(BlackBody, temperatureCelcius, setTemperatureCelcius)


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
	temperature_(temperature)
{
}



TValue BlackBody::temperature() const
{
	return temperature_;
}



void BlackBody::setTemperature(TValue temperature)
{
	temperature_ = temperature;
}



TValue BlackBody::temperatureCelcius() const
{
	return temperature_ - 273.15f;
}



void BlackBody::setTemperatureCelcius(TValue temperature)
{
	temperature_ = temperature + 273.15f;
}


// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const Spectral BlackBody::doEvaluate(const Sample& sample, SpectralType type) const
{
	return Spectral::fromFunc([=](TWavelength w) {
		const TWavelength w5 = num::sqr(num::sqr(w)) * w;
		return static_cast<TValue>(c1 / (w5 * num::expm1(c2 / (w * temperature_))));
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
	return acc / ws.size();
}


const TPyObjectPtr BlackBody::doGetState() const
{
	return python::makeTuple(temperature_);
}



void BlackBody::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, temperature_);
}



// --- free ----------------------------------------------------------------------------------------


}

}

// EOF