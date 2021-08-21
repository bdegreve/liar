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

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_KERNEL_SPECTRAL_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_KERNEL_SPECTRAL_H

#include "kernel_common.h"
#include "bands.h"
#include "observer.h"
#include "sample.h"

#if LIAR_SPECTRAL_MODE_SINGLE
#	define LIAR_SPECTRAL_SAMPLE_INDEPENDENT 0
#else
#	define LIAR_SPECTRAL_SAMPLE_INDEPENDENT 1
#endif

namespace liar
{
namespace kernel
{

class RgbSpace;
typedef python::PyObjectPtr<RgbSpace>::Type TRgbSpacePtr;

class Sample;


enum class SpectralType
{
	Illuminant = 0,
	Reflectant,
};


class LIAR_KERNEL_DLL Spectral
{
public:

#if LIAR_SPECTRAL_MODE_BANDED
	enum { numBands = LIAR_SPECTRAL_MODE_BANDED };
#elif LIAR_SPECTRAL_MODE_RGB || LIAR_SPECTRAL_MODE_XYZ
	enum { numBands = 3 };
#elif LIAR_SPECTRAL_MODE_SINGLE
	enum { numBands = 1 };
#else
	error Invalid spectral mode
#endif

	typedef Bands<numBands, LIAR_VALUE> TBands;

	typedef TBands::TValue TValue;
	typedef TBands::TParam TParam;
	typedef TBands::TReference TReference;
	typedef TBands::TConstReference TConstReference;
	typedef num::NumTraits<TValue> TNumTraits;
	typedef std::vector<TWavelength> TWavelengths;

	Spectral();
	explicit Spectral(TParam f);
	Spectral(TParam f, SpectralType type);
	Spectral(const Spectral& other, SpectralType type);

	static Spectral fromXYZ(const XYZ& xyz, const Sample& sample, SpectralType type);
	static Spectral fromSampled(const TWavelengths& wavelengths, const std::vector<TValue>& values, const Sample& sample, SpectralType type);
#if LIAR_SPECTRAL_SAMPLE_INDEPENDENT
	static Spectral fromSampled(const TWavelengths& wavelengths, const std::vector<TValue>& values, SpectralType type);
#endif

#if LIAR_SPECTRAL_MODE_BANDED
	template <typename Func> static Spectral fromFunc(Func func, const Sample&, SpectralType type)
	{
		TBands v;
		for (size_t k = 0; k < numBands; ++k)
		{
			v[k] = func((bands_[k] + bands_[k + 1]) / 2);
		}
		return Spectral(v, type);
	}
#elif LIAR_SPECTRAL_MODE_RGB || LIAR_SPECTRAL_MODE_XYZ
	template <typename Func> static Spectral fromFunc(Func func, const Sample& sample, SpectralType type)
	{
		return Spectral::fromXYZ(standardObserver().tristimulusFunc(func), sample, type);
	}
#elif LIAR_SPECTRAL_MODE_SINGLE
	template <typename Func> static Spectral fromFunc(Func func, const Sample& sample, SpectralType type)
	{
		return Spectral(func(sample.wavelength()), type);
	}
#endif

	const XYZ xyz(const Sample& sample) const;
	TValue luminance(const Sample& sample) const { return xyz(sample).y; }

	Spectral& operator=(TParam f) { v_.fill(f); return *this; }
	Spectral& operator=(const Spectral& other) { v_ = other.v_; return *this; }

	TParam operator[](size_t index) const { return v_[index]; }
	TReference operator[](size_t index) { return v_[index]; }

	Spectral& operator+=(const Spectral& other) { v_ += other.v_; return *this; }
	Spectral& operator-=(const Spectral& other) { v_ -= other.v_; return *this; }
	Spectral& operator*=(const Spectral& other) { v_ *= other.v_; return *this; }
	Spectral& operator/=(const Spectral& other) { v_ /= other.v_; return *this; }

	Spectral& operator+=(TParam f) { v_ += f; return *this; }
	Spectral& operator-=(TParam f) { v_ -= f; return *this; }
	Spectral& operator*=(TParam f) { v_ *= f; return *this; }
	Spectral& operator/=(TParam f) { v_ *= num::inv(f); return *this; }

	Spectral& fma(const Spectral& a, const Spectral& b) { v_.fma(a.v_, b.v_); return *this; }
	Spectral& fma(TParam a, const Spectral& b) { v_.fma(a, b.v_); return *this; }
	Spectral& fma(const Spectral& a, TParam b) { v_.fma(a.v_, b); return *this; }

	TValue dot(const Spectral& other) const { return v_.dot(other.v_); }
	TValue average() const { return v_.average(); }
	TValue absAverage() const
	{ 
		TBands av(v_); 
		av.inpabs(); 
		return av.average(); 
	}
	TValue minimum() const { return v_.minimum(); }
	TValue maximum() const { return v_.maximum(); }

	bool isZero() const { return v_.isZero(); }
	bool operator!() const { return isZero(); }
	explicit operator bool() const { return !isZero(); }

	Spectral& inpabs() { v_.inpabs(); return *this; }
	Spectral& inpmax(const Spectral& other) { v_.inpmax(other.v_); return *this; }
	Spectral& inpmax(TParam f) { v_.inpmax(f); return *this; }
	Spectral& inppow(const Spectral& other) { v_.inppow(other.v_); return *this; }
	Spectral& inppow(TParam f) { v_.inppow(f); return *this; }
	Spectral& inpsqrt() { v_.inpsqrt(); return *this; }
	Spectral& inpexp() { v_.inpexp(); return *this; }
	Spectral& inpclamp(TParam min, TParam max) { v_.inpclamp(min, max); return *this; }
	Spectral& inplerp(const Spectral& other, TParam f) { v_.inplerp(other.v_, f); return *this; }
    Spectral& inpsin() { v_.inpsin(); return *this; }

	bool operator==(const Spectral& other) const { return v_ == other.v_; }

private:

	Spectral(const TBands& v, SpectralType type);

	TBands v_;

#if LIAR_SPECTRAL_MODE_BANDED
	friend class Observer;
	static bool initialize();
	static TWavelength bands_[numBands + 1];
	static bool isInitialized_;

	template <typename ValueType>
	static void bandedIntegration(ValueType* result, const TWavelengths& w, const std::vector<ValueType>& v)
	{
		const size_t n = w.size();
		LASS_ASSERT(v.size() == n);
		size_t i = 0, k1 = 0, k2 = 1;
		while (i < numBands && k2 < n)
		{
			if (w[k1] >= bands_[i + 1])
			{
				++i;
				continue;
			}
			const TWavelength dw = w[k2] - w[k1];
			if (w[k2] > bands_[i] && dw > 0)
			{
				const TWavelength t1 = w[k1] < bands_[i]     ? (bands_[i]     - w[k1]) / dw : 0;
				const TWavelength t2 = w[k2] > bands_[i + 1] ? (bands_[i + 1] - w[k1]) / dw : 1;
				const TWavelength c = dw * (t2 - t1) / 2;
				const TValue a = static_cast<TValue>(c * (2 - t1 - t2));
				const TValue b = static_cast<TValue>(c * (t1 + t2));
				result[i] += v[k1] * a + v[k1] * b;
			}
			k1 = k2++;
		}
	}
#endif
};

inline Spectral abs(const Spectral& a)
{
	Spectral r(a);
	return r.inpabs();
}

inline Spectral max(const Spectral& a, const Spectral& b)
{
	Spectral r(a);
	return r.inpmax(b);
}

inline Spectral max(const Spectral& a, Spectral::TParam b)
{
	Spectral r(a);
	return r.inpmax(b);
}

inline Spectral max(Spectral::TParam a, const Spectral& b)
{
	Spectral r(b);
	return r.inpmax(a);
}

inline TScalar dot(const Spectral& a, const Spectral& b)
{
	return a.dot(b);
}

inline Spectral sqr(const Spectral& a)
{
	Spectral r(a);
	r *= a;
	return r;
}

inline Spectral pow(const Spectral& a, const Spectral& b)
{
	Spectral r(a);
	return r.inppow(b);
}

inline Spectral pow(const Spectral& a, Spectral::TParam b)
{
	Spectral r(a);
	return r.inppow(b);
}

inline Spectral sqrt(const Spectral& a)
{
	Spectral r(a);
	return r.inpsqrt();
}

inline Spectral exp(const Spectral& a)
{
	Spectral r(a);
	return r.inpexp();
}

inline Spectral clamp(const Spectral& a, Spectral::TParam min, Spectral::TParam max)
{
	Spectral r(a);
	return r.inpclamp(min, max);
}

inline Spectral lerp(const Spectral& a, const Spectral& b, Spectral::TParam f)
{
	Spectral r(a);
	return r.inplerp(b, f);
}

inline Spectral sin(const Spectral& a)
{
    Spectral r(a);
    return r.inpsin();
}

inline Spectral::TValue average(const Spectral& a)
{
	return a.average();
}

inline Spectral operator+(const Spectral& a, const Spectral& b)
{
	Spectral r(a);
	r += b;
	return r;
}

inline Spectral operator-(const Spectral& a, const Spectral& b)
{
	Spectral r(a);
	r -= b;
	return r;
}

inline Spectral operator*(const Spectral& a, const Spectral& b)
{
	Spectral r(a);
	r *= b;
	return r;
}

inline Spectral operator/(const Spectral& a, const Spectral& b)
{
	Spectral r(a);
	r /= b;
	return r;
}

inline Spectral operator+(const Spectral& a, Spectral::TParam b)
{
	Spectral r(a);
	r += b;
	return r;
}

inline Spectral operator-(const Spectral& a, Spectral::TParam b)
{
	Spectral r(a);
	r -= b;
	return r;
}

inline Spectral operator*(const Spectral& a, Spectral::TParam b)
{
	Spectral r(a);
	r *= b;
	return r;
}

inline Spectral operator/(const Spectral& a, Spectral::TParam b)
{
	Spectral r(a);
	r /= b;
	return r;
}

inline Spectral operator+(Spectral::TParam a, const Spectral& b)
{
	Spectral r(b);
	r += a;
	return r;
}

inline Spectral operator-(Spectral::TParam a, const Spectral& b)
{
	Spectral r(a);
	r -= b;
	return r;
}

inline Spectral operator*(Spectral::TParam a, const Spectral& b)
{
	Spectral r(b);
	r *= a;
	return r;
}

inline Spectral operator/(Spectral::TParam a, const Spectral& b)
{
	Spectral r(a);
	r /= b;
	return r;
}

}
}

#endif

// EOF
