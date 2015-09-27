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

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_KERNEL_SPECTRUM_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_KERNEL_SPECTRUM_H

#include "kernel_common.h"
#include "rgb_space.h"
#include "bands.h"

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL Spectrum
{
private:

#if LIAR_SPECTRAL_BANDS
	typedef Bands< LIAR_SPECTRAL_BANDS> TBands;
#else
	typedef Bands<3> TBands;
#endif

public:
	typedef TBands::TValue TValue;
	typedef TBands::TParam TParam;
	typedef TBands::TReference TReference;
	typedef TBands::TConstReference TConstReference;

	enum { numBands = TBands::numBands };

	Spectrum();
	explicit Spectrum(TParam f);
	explicit Spectrum(const XYZ& xyz);

	const XYZ xyz() const;
	operator XYZ() const { return xyz(); }

	TParam operator[](size_t index) const { return v_[index]; }
	TReference operator[](size_t index) { return v_[index]; }

	Spectrum& operator+=(const Spectrum& other) { v_ += other.v_; return *this; }
	Spectrum& operator-=(const Spectrum& other) { v_ -= other.v_; return *this; }
	Spectrum& operator*=(const Spectrum& other) { v_ *= other.v_; return *this; }
	Spectrum& operator/=(const Spectrum& other) { v_ /= other.v_; return *this; }

	Spectrum& operator+=(TParam f) { v_ += f; return *this; }
	Spectrum& operator-=(TParam f) { v_ -= f; return *this; }
	Spectrum& operator*=(TParam f) { v_ *= f; return *this; }
	Spectrum& operator/=(TParam f) { v_ *= num::inv(f); return *this; }

	Spectrum& fma(const Spectrum& a, const Spectrum& b) { v_.fma(a.v_, b.v_); return *this; }
	Spectrum& fma(TParam a, const Spectrum& b) { v_.fma(a, b.v_); return *this; }
	Spectrum& fma(const Spectrum& a, TParam b) { v_.fma(a.v_, b); return *this; }

	TScalar dot(const Spectrum& other) const { return v_.dot(other.v_); }
	TScalar total() const { return v_.total(); }
	TScalar absTotal() const { return v_.absTotal(); }
	TScalar average() const { return total() / numBands; }

	bool isZero() const { return v_.isZero(); }
	bool operator!() const { return isZero(); }
	operator num::SafeBool() const { return isZero() ? num::safeFalse : num::safeTrue; }

	Spectrum& inpabs() { v_.inpabs(); return *this; }
	Spectrum& inpmax(const Spectrum& other) { v_.inpmax(other.v_); return *this; }
	Spectrum& inppow(TParam f) { v_.inppow(f); return *this; }
	Spectrum& inpsqrt() { v_.inpsqrt(); return *this; }
	Spectrum& inpexp() { v_.inpexp(); return *this; }
	Spectrum& inpclamp(TParam min, TParam max) { v_.inpclamp(min, max); return *this; }
	Spectrum& inplerp(const Spectrum& other, TParam f) { v_.inplerp(other.v_, f); return *this; }

	bool operator==(const Spectrum& other) const { return v_ == other.v_; }

private:    
	TBands v_;

#if LIAR_SPECTRAL_BANDS
	static XYZ observer_[numBands];
	static TWavelength bands_[numBands + 1];
	static Spectrum yellow_;
	static Spectrum magenta_;
	static Spectrum cyan_;
	static Spectrum red_;
	static Spectrum green_;
	static Spectrum blue_;
	static TRgbSpacePtr rgbSpace_;

	static TRgbSpacePtr initData();
#endif
};

inline Spectrum abs(const Spectrum& a)
{
	Spectrum r(a);
	return r.inpabs();
}

inline Spectrum max(const Spectrum& a, const Spectrum& b)
{
	Spectrum r(a);
	return r.inpmax(b);
}

inline TScalar dot(const Spectrum& a, const Spectrum& b)
{
	return a.dot(b);
}

inline Spectrum sqr(const Spectrum& a)
{
	Spectrum r(a);
	r *= a;
	return r;
}

inline Spectrum pow(const Spectrum& a, TScalar b)
{
	Spectrum r(a);
	return r.inppow(b);
}

inline Spectrum sqrt(const Spectrum& a)
{
	Spectrum r(a);
	return r.inpsqrt();
}

inline Spectrum exp(const Spectrum& a)
{
	Spectrum r(a);
	return r.inpexp();
}

inline Spectrum clamp(const Spectrum& a, TScalar min, TScalar max)
{
	Spectrum r(a);
	return r.inpclamp(min, max);
}

inline Spectrum lerp(const Spectrum& a, const Spectrum& b, TScalar f)
{
	Spectrum r(a);
	return r.inplerp(b, f);
}

inline TScalar average(const Spectrum& a)
{
	return a.average();
}

inline Spectrum operator+(const Spectrum& a, const Spectrum& b)
{
	Spectrum r(a);
	r += b;
	return r;
}

inline Spectrum operator-(const Spectrum& a, const Spectrum& b)
{
	Spectrum r(a);
	r -= b;
	return r;
}

inline Spectrum operator*(const Spectrum& a, const Spectrum& b)
{
	Spectrum r(a);
	r *= b;
	return r;
}

inline Spectrum operator/(const Spectrum& a, const Spectrum& b)
{
	Spectrum r(a);
	r /= b;
	return r;
}

inline Spectrum operator+(const Spectrum& a, Spectrum::TParam b)
{
	Spectrum r(a);
	r += b;
	return r;
}

inline Spectrum operator-(const Spectrum& a, Spectrum::TParam b)
{
	Spectrum r(a);
	r -= b;
	return r;
}

inline Spectrum operator*(const Spectrum& a, Spectrum::TParam b)
{
	Spectrum r(a);
	r *= b;
	return r;
}

inline Spectrum operator/(const Spectrum& a, Spectrum::TParam b)
{
	Spectrum r(a);
	r /= b;
	return r;
}

inline Spectrum operator+(Spectrum::TParam a, const Spectrum& b)
{
	Spectrum r(b);
	r += a;
	return r;
}

inline Spectrum operator-(Spectrum::TParam a, const Spectrum& b)
{
	Spectrum r(a);
	r -= b;
	return r;
}

inline Spectrum operator*(Spectrum::TParam a, const Spectrum& b)
{
	Spectrum r(b);
	r *= a;
	return r;
}

inline Spectrum operator/(Spectrum::TParam a, const Spectrum& b)
{
	Spectrum r(a);
	r /= b;
	return r;
}

PY_SHADOW_CLASS(LIAR_KERNEL_DLL, PySpectrum, Spectrum)

}
}

PY_SHADOW_CASTERS(liar::kernel::PySpectrum)

#endif

// EOF
