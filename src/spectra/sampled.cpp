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
#include "sampled.h"

namespace liar
{
namespace spectra
{

PY_DECLARE_CLASS_DOC(Sampled, "sampled spectrum")
PY_CLASS_CONSTRUCTOR_2(Sampled, const Sampled::TWavelengths&, const Sampled::TScalars&)


// --- public --------------------------------------------------------------------------------------

Sampled::Sampled(const TWavelengths& wavelengths, const TScalars& values):
	wavelengths_(wavelengths),
	values_(values)
{
	tristimulus_ = standardObserver().tristimulus(wavelengths, values);
}


TSpectrumPtr Sampled::resample(const TWavelengths& wavelengths) const
{
	const size_t n = wavelengths.size();
	TScalars values(n, 0);

	if (wavelengths_.size() == 1)
	{
		// special case, not much hope here ...
	}

	size_t i = 0;
	for (size_t k = 1, m = wavelengths_.size(); k < m; ++k)
	{
		const TWavelength w1 = wavelengths_[k - 1];
		const TWavelength w2 = wavelengths_[k];
		while (wavelengths[i] < w1 && i < n)
		{
			++i;
		}
		if (i == n)
		{
			break;
		}
		const TWavelength w = wavelengths[i];
		if (w < w2)
		{
			const TScalar t = (w - w1) / (w2 - w1);
			values[i] = num::lerp(values_[k - 1], values_[k], t);
		}
	}

	return TSpectrumPtr(new Sampled(wavelengths, values));
}


// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const Spectral Sampled::doEvaluate(const Sample& sample, SpectralType type) const
{
	return Spectral::fromSampled(wavelengths_, values_, sample, type);
}



TScalar Sampled::doLuminance() const
{
	return tristimulus_.y;
}


const TPyObjectPtr Sampled::doGetState() const
{
	return python::makeTuple(wavelengths_, values_);
}



void Sampled::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, wavelengths_, values_);
	tristimulus_ = standardObserver().tristimulus(wavelengths_, values_);
}



// --- free ----------------------------------------------------------------------------------------


}

}

// EOF
