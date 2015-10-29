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

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_KERNEL_SPECTRAL_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_KERNEL_SPECTRAL_H

#include "kernel_common.h"
#include "rgb_space.h"
#include "bands.h"

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL Spectral
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

	Spectral();
	explicit Spectral(TParam f);
	explicit Spectral(const XYZ& xyz);

	const XYZ xyz() const;
	operator XYZ() const { return xyz(); }

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

	TScalar dot(const Spectral& other) const { return v_.dot(other.v_); }
	TScalar total() const { return v_.total(); }
	TScalar absTotal() const { return v_.absTotal(); }
	TScalar average() const { return total() / numBands; }

	bool isZero() const { return v_.isZero(); }
	bool operator!() const { return isZero(); }
	operator num::SafeBool() const { return isZero() ? num::safeFalse : num::safeTrue; }

	Spectral& inpabs() { v_.inpabs(); return *this; }
	Spectral& inpmax(const Spectral& other) { v_.inpmax(other.v_); return *this; }
	Spectral& inppow(const Spectral& other) { v_.inppow(other.v_); return *this; }
	Spectral& inppow(TParam f) { v_.inppow(f); return *this; }
	Spectral& inpsqrt() { v_.inpsqrt(); return *this; }
	Spectral& inpexp() { v_.inpexp(); return *this; }
	Spectral& inpclamp(TParam min, TParam max) { v_.inpclamp(min, max); return *this; }
	Spectral& inplerp(const Spectral& other, TParam f) { v_.inplerp(other.v_, f); return *this; }

	bool operator==(const Spectral& other) const { return v_ == other.v_; }

private:    
	TBands v_;

#if LIAR_SPECTRAL_BANDS
	static XYZ observer_[numBands];
	static TWavelength bands_[numBands + 1];
	static Spectral yellow_;
	static Spectral magenta_;
	static Spectral cyan_;
	static Spectral red_;
	static Spectral green_;
	static Spectral blue_;
	static TRgbSpacePtr rgbSpace_;

	static TRgbSpacePtr initData();
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

inline Spectral pow(const Spectral& a, TScalar b)
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

inline Spectral clamp(const Spectral& a, TScalar min, TScalar max)
{
	Spectral r(a);
	return r.inpclamp(min, max);
}

inline Spectral lerp(const Spectral& a, const Spectral& b, TScalar f)
{
	Spectral r(a);
	return r.inplerp(b, f);
}

inline TScalar average(const Spectral& a)
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

PY_SHADOW_CLASS(LIAR_KERNEL_DLL, PySpectral, Spectral)

}
}

PY_SHADOW_CASTERS(liar::kernel::PySpectral)

#endif

// EOF
