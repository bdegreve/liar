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
#include "cauchy.h"

namespace liar
{
namespace spectra
{

PY_DECLARE_CLASS_DOC(Cauchy, "Cauchy's equation")
PY_CLASS_CONSTRUCTOR_2(Cauchy, Cauchy::TParam, Cauchy::TParam)


// --- public --------------------------------------------------------------------------------------

Cauchy::Cauchy(TParam b, TParam c) :
	b_(b),
	c_(c)
{
}


Cauchy::TValue Cauchy::b() const
{
	return b_;
}


void Cauchy::setB(TParam b)
{
	b_ = b;
}


Cauchy::TValue Cauchy::c() const
{
	return c_;
}


void Cauchy::setC(TParam c)
{
	c_ = c;
}


// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const Spectral Cauchy::doEvaluate(const Sample& sample, SpectralType type) const
{
	return Spectral::fromFunc([=](TWavelength w) {
		const TWavelength w_nm = sample.wavelength() * 1e6; // in micrometers
		return static_cast<TValue>(b_ + c_ / num::sqr(w_nm));
	}, sample, type);
}



Cauchy::TValue Cauchy::doLuminance() const
{
	return b_;
}


const TPyObjectPtr Cauchy::doGetState() const
{
	return python::makeTuple(b_, c_);
}



void Cauchy::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, b_, c_);
}



// --- free ----------------------------------------------------------------------------------------


}

}

// EOF
