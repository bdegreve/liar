/** @file
*  @author Bram de Greve (bramz@users.sourceforge.net)
*
*  LiAR isn't a raytracer
*  Copyright (C) 2023-2025  Bram de Greve (bramz@users.sourceforge.net)
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
#include "r0_conductor.h"

namespace liar
{
namespace spectra
{

PY_DECLARE_CLASS_DOC(R0Conductor,
	"Reflectance at normal incidence of a conductor with complex refractive index n + ik.\n"
	"\n"
	"R0 = (n^2 + k^2 - 2n + 1) / (n^2 + k^2 + 2n + 1)\n"
	"\n"
	"R0Conductor(n: Spectrum, k: Spectrum)\n"
)
PY_CLASS_CONSTRUCTOR_2(R0Conductor, const TSpectrumRef&, const TSpectrumRef&)
PY_CLASS_MEMBER_R(R0Conductor, n)
PY_CLASS_MEMBER_R(R0Conductor, k)

// --- public --------------------------------------------------------------------------------------

R0Conductor::R0Conductor(const TSpectrumRef& n, const TSpectrumRef& k) :
	n_(n),
	k_(k)
{
	update();
}


const TSpectrumRef& R0Conductor::n() const
{
	return n_;
}


const TSpectrumRef& R0Conductor::k() const
{
	return k_;
}


// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

R0Conductor::TValue R0Conductor::doCall(TWavelength wavelength) const
{
	const TValue n = n_->operator()(wavelength);
	const TValue k = k_->operator()(wavelength);

	const TValue n2k2 = n * n + k * k;
	return (n2k2 - 2 * n + 1) / (n2k2 + 2 * n + 1);
}



const Spectral R0Conductor::doEvaluate(const Sample& sample, SpectralType /*type*/) const
{
	const Spectral n = n_->evaluate(sample, SpectralType::Illuminant);
	const Spectral k = k_->evaluate(sample, SpectralType::Illuminant);

	const Spectral n2k2 = n * n + k * k;
	return (n2k2 - 2 * n + 1) / (n2k2 + 2 * n + 1);
}



const TPyObjectPtr R0Conductor::doGetState() const
{
	return python::makeTuple(n_, k_);
}



void R0Conductor::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, n_, k_);
}



// --- free ----------------------------------------------------------------------------------------


}

}

// EOF
