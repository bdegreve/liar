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
#include "cauchy.h"

namespace liar
{
namespace spectra
{

PY_DECLARE_CLASS_DOC(Cauchy,
	"Cauchy's equation for dispersive refractive index:\n"
	"\n"
	"n(w) = a + b / w^2, with w the wavelength in micrometers\n"
	"\n"
	"Cauchy(a, b)\n"
)
PY_CLASS_CONSTRUCTOR_2(Cauchy, Cauchy::TParam, Cauchy::TParam)
PY_CLASS_MEMBER_R(Cauchy, a)
PY_CLASS_MEMBER_R(Cauchy, b)


// --- public --------------------------------------------------------------------------------------

Cauchy::Cauchy(TParam a, TParam b) :
	a_(a),
	b_(b)
{
	update();
}


Cauchy::TValue Cauchy::a() const
{
	return a_;
}


Cauchy::TValue Cauchy::b() const
{
	return b_;
}


// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

Cauchy::TValue Cauchy::doCall(TWavelength wavelength) const
{
	const TWavelength w_um = wavelength * 1e6f; // in micrometers
	return static_cast<TValue>(a_ + b_ / num::sqr(w_um));
}



const Spectral Cauchy::doEvaluate(const Sample& sample, SpectralType type) const
{
	return Spectral::fromFunc([this](TWavelength w) {
		const TWavelength w_um = w * 1e6f; // in micrometers
		return static_cast<TValue>(a_ + b_ / num::sqr(w_um));
	}, sample, type);
}



const TPyObjectPtr Cauchy::doGetState() const
{
	return python::makeTuple(a_, b_);
}



void Cauchy::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, a_, b_);
}



// --- free ----------------------------------------------------------------------------------------


}

}

// EOF
