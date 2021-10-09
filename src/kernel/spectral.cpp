/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2021  Bram de Greve (bramz@users.sourceforge.net)
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
#include "recovery.h"
#include "rgb_space.h"
#include "sample.h"
#include "spectral.h"

namespace liar
{
namespace kernel
{

Spectral::Spectral()
{
}


Spectral::Spectral(TValue f):
	v_(f)
{
}


Spectral::Spectral(TValue f, SpectralType type) :
	v_(type == SpectralType::Reflectant ? std::min(f, TNumTraits::one) : f)
{
}


Spectral::Spectral(const Spectral &other, SpectralType type):
	Spectral(other.v_, type)
{
}


Spectral::Spectral(const TBands& v, SpectralType type):
	v_(v)
{
	if (type == SpectralType::Reflectant)
	{
		const TValue max = v_.maximum();
		if (max > 1)
		{
			v_ /= max;
		}
	}
}

#if LIAR_SPECTRAL_MODE_BANDED

namespace
{

const Spectral::TValue bandwidth = ((LIAR_SPECTRAL_MAX_WAVELENGTH)-(LIAR_SPECTRAL_MIN_WAVELENGTH)) * 1e-9f / Spectral::numBands;

}

Spectral Spectral::fromXYZ(const XYZ& xyz, const Sample& sample, SpectralType type)
{
	return standardRecovery().recover(xyz, sample, type);
}

Spectral Spectral::fromSampled(const std::vector<TWavelength>& wavelengths, const std::vector<TValue>& values, const Sample&, SpectralType type)
{
	return fromSampled(wavelengths, values, type);
}

Spectral Spectral::fromSampled(const std::vector<TWavelength>& wavelengths, const std::vector<TValue>& values, SpectralType type)
{
	Spectral result;
	bandedIntegration(&result.v_[0], wavelengths, values);
	result /= bandwidth;

	if (type == SpectralType::Reflectant)
	{
		const TValue max = result.v_.maximum();
		if (max > 1)
		{
			result /= max;
		}
	}

	return result;
}

const XYZ Spectral::xyz(const Sample&) const
{
	return standardObserver().tristimulus(*this);
}


// can we do this with a constexpr somehow?

bool Spectral::initialize()
{
	const TWavelength wMin = bands_[0] = (LIAR_SPECTRAL_MIN_WAVELENGTH)* 1e-9f;
	const TWavelength wMax = bands_[numBands] = (LIAR_SPECTRAL_MAX_WAVELENGTH)* 1e-9f;
	for (size_t k = 1; k < numBands; ++k)
	{
		bands_[k] = wMin + k * bandwidth;
	}
	return true;
}

TWavelength Spectral::bands_[Spectral::numBands + 1] = {};
bool Spectral::isInitialized_ = Spectral::initialize();

#elif LIAR_SPECTRAL_MODE_RGB

Spectral Spectral::fromXYZ(const XYZ& xyz, const Sample&, SpectralType type)
{
	const RgbSpace::RGBA rgb = RgbSpace::defaultSpace()->linearConvert(xyz);
	return Spectral(TBands(rgb.r, rgb.g, rgb.b), type);
}

Spectral Spectral::fromSampled(const std::vector<TWavelength>& wavelengths, const std::vector<TValue>& values, const Sample&, SpectralType type)
{
	return fromSampled(wavelengths, values, type);
}

Spectral Spectral::fromSampled(const std::vector<TWavelength>& wavelengths, const std::vector<TValue>& values, SpectralType type)
{
	const XYZ xyz = standardObserver().tristimulus(wavelengths, values);
	const RgbSpace::RGBA rgb = RgbSpace::defaultSpace()->linearConvert(xyz);
	return Spectral(TBands(rgb.r, rgb.g, rgb.b), type);
}

const XYZ Spectral::xyz(const Sample&) const
{
	return RgbSpace::defaultSpace()->linearConvert(RgbSpace::RGBA(v_[0], v_[1], v_[2]));
}

#elif LIAR_SPECTRAL_MODE_XYZ

Spectral Spectral::fromXYZ(const XYZ& xyz, const Sample&, SpectralType type)
{
	return Spectral(TBands(xyz.x, xyz.y, xyz.z), type);
}

Spectral Spectral::fromSampled(const std::vector<TWavelength>& wavelengths, const std::vector<TValue>& values, const Sample&, SpectralType type)
{
	return fromSampled(wavelengths, values, type);
}

Spectral Spectral::fromSampled(const std::vector<TWavelength>& wavelengths, const std::vector<TValue>& values, SpectralType type)
{
	const XYZ xyz = standardObserver().tristimulus(wavelengths, values);
	return Spectral(TBands(xyz.x, xyz.y, xyz.z), type);
}

const XYZ Spectral::xyz(const Sample&) const
{
	return XYZ(v_[0], v_[1], v_[2]);
}

#elif LIAR_SPECTRAL_MODE_SINGLE

Spectral Spectral::fromXYZ(const XYZ& xyz, const Sample& sample, SpectralType type)
{
	return standardRecovery().recover(xyz, sample, type);
}

Spectral Spectral::fromSampled(const std::vector<TWavelength>& wavelengths, const std::vector<TValue>& values, const Sample& sample, SpectralType type)
{
	LASS_ASSERT(wavelengths.size() > 1 && wavelengths.size() == values.size());
	const auto i = std::upper_bound(wavelengths.begin(), wavelengths.end(), sample.wavelength());
	if (i == wavelengths.begin() || i == wavelengths.end())
	{
		return Spectral(0);
	}
	
	const size_t k = static_cast<size_t>(std::distance(wavelengths.begin(), i));
	LASS_ASSERT(k > 0 && k < wavelengths.size());

	LASS_ASSERT(wavelengths[k] > wavelengths[k - 1]);
	const TValue t = static_cast<TValue>((sample.wavelength() - wavelengths[k - 1]) / (wavelengths[k] - wavelengths[k - 1]));
	const TValue v = num::lerp(values[k - 1], values[k], t);

	if (type == SpectralType::Reflectant)
	{
		const TValue max = *std::max_element(values.begin(), values.end());
		if (max > 1)
		{
			return Spectral(v / max);
		}
	}
	return Spectral(v);
}

const XYZ Spectral::xyz(const Sample& sample) const
{
	TScalar pdf = 0;
	const TWavelength w = sample.wavelength(pdf);
	if (pdf <= 0)
	{
		return XYZ(0);
	}
	return static_cast<TValue>(v_[0] / pdf) * standardObserver().sensitivity(w);
}

#endif

std::ostream& operator<<(std::ostream& s, const Spectral& a)
{
	for (size_t k = 0; k < Spectral::numBands; ++k)
	{
		s << (k == 0 ? "[" : ", ") << a[k];
	}	
	s << "]";
	return s;
}

LIAR_KERNEL_DLL bool isFinite(const Spectral& x)
{
	return isFinite(x.average());
}

LIAR_KERNEL_DLL bool isPositiveAndFinite(const Spectral& x)
{
	return x.minimum() >= 0 && isFinite(x);
}

}
}

// EOF
