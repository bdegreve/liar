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

#include "kernel_common.h"

#include "observer.h"
#if LIAR_SPECTRAL_MODE_BANDED
#	include "spectral.h"
#endif

#include <lass/stde/extended_iterator.h>
#include <lass/stde/access_iterator.h>


namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(Observer, "Conversion of spectral data to tristimulus")
	PY_CLASS_CONSTRUCTOR_2(Observer, const Observer::TWavelengths&, const Observer::TXYZs&)
	PY_CLASS_MEMBER_R(Observer, wavelengths)
	PY_CLASS_MEMBER_R(Observer, sensitivities)
	PY_CLASS_MEMBER_R(Observer, minWavelength)
	PY_CLASS_MEMBER_R(Observer, maxWavelength)
	PY_CLASS_METHOD(Observer, sensitivity)
	PY_CLASS_METHOD_QUALIFIED_1(Observer, tristimulus, const XYZ, const Observer::TValues&)
	PY_CLASS_STATIC_METHOD_DOC(Observer, standard, "standard() -> Observer");
	PY_CLASS_STATIC_METHOD_DOC(Observer, setStandard, "setStandard(Observer) -> None");

TObserverPtr Observer::standard_(0);


// --- public ----------------------------------------------------------------

Observer::Observer(const TWavelengths& wavelengths, const TXYZs& sensitivities):
	w_(wavelengths),
	xyz_(sensitivities)
{
	const size_t n = wavelengths.size();
	if (n < 2)
	{
		LASS_THROW("Requires at least two wavelengths.");
	}
	if (sensitivities.size() != n)
	{
		LASS_THROW("Requires as many sensitivities as wavelengths.");
	}

	dxyz_dw_.reserve(n);
	dXYZ_.reserve(n);
	cdf_.reserve(n);

	TWavelength w_prev = w_[0];
	cdf_.push_back(0);
	for (size_t k = 0;  k < (n - 1); ++k)
	{
		const TWavelength w = w_[k];
		const XYZ& xyz = xyz_[k];

		const TWavelength w_next = w_[k + 1];
		const XYZ& xyz_next = xyz_[k + 1];

		dxyz_dw_.push_back((xyz_next - xyz) / static_cast<TValue>(w_next - w));
		dXYZ_.push_back(xyz * static_cast<TValue>((w_next - w_prev) / 2));

		w_prev = w;
		cdf_.push_back(cdf_.back() + xyz.absTotal());
	}

	{
		const TWavelength w = w_.back();
		const XYZ& xyz = xyz_.back();

		dxyz_dw_.push_back(XYZ(0, 0, 0));
		dXYZ_.push_back(xyz * static_cast<TValue>((w - w_prev) / 2));
		cdf_.push_back(cdf_.back() + xyz.absTotal());
	}

	// rescale so that tristimulus of `1' spectrum gives Y == 1, and that cdf_.back() == 1.
	TValue Y = 0;
	for (size_t k = 0; k < n; ++k)
	{
		Y += dXYZ_[k].y;
	}
	LASS_ENFORCE(Y > 0);
	
	const TValue invY = num::inv(Y);
	const TScalar invCdf = num::inv(cdf_.back());

	for (size_t k = 0; k < n; ++k)
	{
		xyz_[k] *= invY;
		dxyz_dw_[k] *= invY;
		dXYZ_[k] *= invY;
		cdf_[k] *= invCdf;
	}
	cdf_.back() = 1;

#if LIAR_SPECTRAL_MODE_BANDED
	std::fill(xyzBands_, xyzBands_ + Spectral::numBands, XYZ(0));
	Spectral::bandedIntegration(xyzBands_, w_, xyz_);
#endif
}


const Observer::TWavelengths& Observer::wavelengths() const
{
	return w_;
}


const Observer::TXYZs& Observer::sensitivities() const
{
	return xyz_;
}


TWavelength Observer::minWavelength() const
{
	return w_.front();
}


TWavelength Observer::maxWavelength() const
{
	return w_.back();
}


const XYZ Observer::sensitivity(TWavelength wavelength) const
{
	const TWavelengths::const_iterator i = std::upper_bound(w_.begin(), w_.end(), wavelength);
	if (i == w_.begin() || i == w_.end())
	{
		return XYZ(0);
	}
	
	const size_t k = static_cast<size_t>(std::distance(w_.begin(), i) - 1);
	LASS_ASSERT(k < w_.size() - 1);

	const TWavelength dw = wavelength - w_[k];
	LASS_ASSERT(dw >= 0);

	return xyz_[k] + dxyz_dw_[k] * static_cast<TValue>(dw);
}


/** Calculate tristimulus for spectrum where you have a sample for each wavelength in the observer 
 */
const XYZ Observer::tristimulus(const TValues& spectrum) const
{
	if (spectrum.size() != dXYZ_.size())
	{
		LASS_THROW("Requires a spectrum sample for each wavelength in observer data.");
	}

	XYZ acc;
	for (size_t k = 0, n = w_.size(); k < n; ++k)
	{
		acc += dXYZ_[k] * spectrum[k];
	}
	return acc;
}


/** Calculate tristimulus for spectrum where you have values at different sampled wavelengths
 */
const XYZ Observer::tristimulus(const TWavelengths& wavelengths, const TValues& spectrum) const
{
	const size_t n = wavelengths.size();
	LASS_ASSERT(n == spectrum.size());
	LASS_ASSERT(n > 1);

	// Triangular integration.
	XYZ acc = sensitivity(wavelengths[0]) * spectrum[0] * static_cast<TValue>((wavelengths[1] - wavelengths[0]) / 2);
	for (size_t k = 1; k < n - 1; ++k)
	{
		acc += sensitivity(wavelengths[k]) * spectrum[k] * static_cast<TValue>((wavelengths[k + 1] - wavelengths[k - 1]) / 2);
	}
	acc += sensitivity(wavelengths[n - 1]) * spectrum[n - 1] * static_cast<TValue>((wavelengths[n - 1] - wavelengths[n - 2]) / 2);

	return acc;
}


/** Calculate luminance (Y component of tristimulus) for spectrum where you have a sample for each wavelength in the observer
*/
Observer::TValue Observer::luminance(const TValues& spectrum) const
{
	if (spectrum.size() != dXYZ_.size())
	{
		LASS_THROW("Requires a spectrum sample for each wavelength in observer data.");
	}

	TValue y = 0;
	for (size_t k = 0, n = w_.size(); k < n; ++k)
	{
		y += dXYZ_[k].y * spectrum[k];
	}
	return y;
}


/** Calculate luminance (Y component of tristimulus) for spectrum where you have values at different sampled wavelengths
*/
Observer::TValue Observer::luminance(const TWavelengths& wavelengths, const TValues& spectrum) const
{
	// i'm too lazy.
	return tristimulus(wavelengths, spectrum).y;
}



TWavelength Observer::sample(TScalar sample, TScalar& pdf) const
{
	LASS_ASSERT(sample >= 0 && sample < 1);
	const auto i = std::upper_bound(cdf_.begin(), cdf_.end(), sample);

	if (i == cdf_.begin())
	{
		LASS_ASSERT(false); // should never happen since *i > sample >= 0 and cdf_[0] = 0
		pdf = 0;
		return w_.front();
	}
	if (i == cdf_.end())
	{
		LASS_ASSERT(false); // should never happen since cdf_.back() == 1 > sample
		pdf = 0;
		return w_.back();
	}
	
	const TScalar cdf1 = *stde::prev(i);
	const TScalar cdf2 = *i;
	const TScalar dcdf = cdf2 - cdf1;
	LASS_ASSERT(dcdf > 0);
	const TWavelength x = static_cast<TWavelength>((sample - cdf1) / dcdf);
	
	const size_t k = static_cast<size_t>(std::distance(cdf_.begin(), i));
	LASS_ASSERT(k > 0 && k < cdf_.size());
	const TWavelength w1 = w_[k - 1];
	const TWavelength w2 = w_[k];
	const TWavelength dw = w2 - w1;
	LASS_ASSERT(dw > 0);

	pdf = dcdf / dw;
	return num::lerp(w1, w2, x);
}

const TObserverPtr& Observer::standard()
{
	return standard_;
}


void Observer::setStandard(const TObserverPtr& standard)
{
	standard_ = standard;
}


// --- private ----------------------------------------------------------------

#if LIAR_SPECTRAL_MODE_BANDED

const XYZ Observer::tristimulus(const Spectral& spectrum) const
{
	XYZ sum = 0;
	for (size_t k = 0; k < Spectral::numBands; ++k)
	{
		sum += xyzBands_[k] * spectrum[k];
	}
	return sum;

}

#endif


// --- free --------------------------------------------------------------------

/** Convenience function that checks if standard observer is initialized
 */
const Observer& standardObserver()
{
	const TObserverPtr& standard = Observer::standard();
	if (!standard)
	{
		LASS_THROW("You must first load a standard observer using liar.Observer.setStandard(observer)");
	}
	return *standard;
}

}
}

// EOF
