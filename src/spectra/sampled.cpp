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
#include "sampled.h"

namespace liar
{
namespace spectra
{

PY_DECLARE_CLASS_DOC(Sampled,
	"Sampled spectrum\n"
	"\n"
	"Sampled(wavelengths, values)")
PY_CLASS_CONSTRUCTOR_2(Sampled, const Sampled::TWavelengths&, const Sampled::TValues&)
PY_CLASS_MEMBER_R(Sampled, wavelengths)
PY_CLASS_MEMBER_R(Sampled, values)
PY_CLASS_METHOD(Sampled, resample)

// --- public --------------------------------------------------------------------------------------

Sampled::Sampled(const TWavelengths& wavelengths, const TValues& values):
	wavelengths_(wavelengths),
	values_(values)
{
	update();
}


const Sampled::TWavelengths& Sampled::wavelengths() const
{
	return wavelengths_;
}


const Sampled::TValues& Sampled::values() const
{
	return values_;
}


TSpectrumPtr Sampled::resample(const TWavelengths& wavelengths) const
{
	const size_t n = wavelengths.size();
	TValues values(n, 0);

	LASS_ENFORCE(wavelengths_.size() > 1);

	size_t i = 0;

	{
		const TWavelength w0 = wavelengths_[0];
		while (wavelengths[i] < w0 && i < n)
		{
			++i;
		}
	}

	for (size_t k = 1, m = wavelengths_.size(); k < m && i < n; ++k)
	{
		const TWavelength w1 = wavelengths_[k - 1];
		const TWavelength w2 = wavelengths_[k];
		while (wavelengths[i] < w2 && i < n)
		{
			const TValue t = static_cast<TValue>((wavelengths[i] - w1) / (w2 - w1));
			values[i] = num::lerp(values_[k - 1], values_[k], t);
			++i;
		}
	}

	return TSpectrumPtr(new Sampled(wavelengths, values));
}


// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

Sampled::TValue Sampled::doCall(TWavelength wavelength) const
{
	if (wavelength < wavelengths_.front() || wavelength > wavelengths_.back())
	{
		return 0;
	}
	const auto i = std::upper_bound(wavelengths_.begin(), wavelengths_.end(), wavelength);
	if (i == wavelengths_.begin())
	{
		return values_.front();
	}
	if (i == wavelengths_.end())
	{
		return values_.back();
	}

	const size_t k = static_cast<size_t>(std::distance(wavelengths_.begin(), i));
	LASS_ASSERT(k > 0 && k < wavelengths_.size());
	const TWavelength w1 = wavelengths_[k - 1];
	const TWavelength w2 = wavelengths_[k];
	const TValue t = static_cast<TValue>((wavelength - w1) / (w2 - w1));
	return num::lerp(values_[k - 1], values_[k], t);
}



const Spectral Sampled::doEvaluate(const Sample& sample, SpectralType type) const
{
	return Spectral::fromSampled(wavelengths_, values_, sample, type);
}



const TPyObjectPtr Sampled::doGetState() const
{
	return python::makeTuple(wavelengths_, values_);
}



void Sampled::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, wavelengths_, values_);
}



// --- free ----------------------------------------------------------------------------------------


}

}

// EOF
