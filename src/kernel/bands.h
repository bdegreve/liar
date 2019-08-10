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

/** @class liar::kernel::Bands
 *  @brief vectorial numerical data
 *  @author Bram de Greve [Bramz]
*/

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_BANDS_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_BANDS_H

#include "kernel_common.h"

namespace liar
{
namespace kernel
{

template <size_t N, typename T = TScalar>
class LASS_SIMD_ALIGN Bands
{
public:

	typedef typename util::CallTraits<T>::TValue TValue;
	typedef typename util::CallTraits<T>::TParam TParam;
	typedef typename util::CallTraits<T>::TReference TReference;
	typedef typename util::CallTraits<T>::TConstReference TConstReference;
	typedef num::NumTraits<T> TNumTraits;

	enum { numBands = N };

	Bands(TParam f = TNumTraits::zero)
	{
		std::fill(v_, v_ + numBands, f);
	}

	void fill(TParam f)
	{
		std::fill(v_, v_ + numBands, f);
	}

	TReference operator[](size_t i) { return v_[i]; }
	TParam operator[](size_t i) const { return v_[i]; }

	Bands& operator+=(const Bands& other)
	{
		for (size_t i = 0; i < N; ++i)
		{
			v_[i] += other.v_[i];
		}
		return *this;
	}
	Bands& operator-=(const Bands& other)
	{
		for (size_t i = 0; i < N; ++i)
		{
			v_[i] -= other.v_[i];
		}
		return *this;
	}
	Bands& operator*=(const Bands& other)
	{
		for (size_t i = 0; i < N; ++i)
		{
			v_[i] *= other.v_[i];
		}
		return *this;
	}
	Bands& operator/=(const Bands& other)
	{
		for (size_t i = 0; i < N; ++i)
		{
			v_[i] /= other.v_[i];
		}
		return *this;
	}

	Bands& operator+=(TParam f)
	{
		for (size_t i = 0; i < N; ++i)
		{
			v_[i] += f;
		}
		return *this;
	}
	Bands& operator-=(TParam f)
	{
		for (size_t i = 0; i < N; ++i)
		{
			v_[i] -= f;
		}
		return *this;
	}
	Bands& operator*=(TParam f)
	{
		for (size_t i = 0; i < N; ++i)
		{
			v_[i] *= f;
		}
		return *this;
	}
	Bands& operator/=(TParam f)
	{
		return (*this *= num::inv(f));
	}

	Bands& fma(const Bands& a, const Bands& b)
	{
		for (size_t i = 0; i < N; ++i)
		{
			v_[i] += a.v_[i] * b.v_[i];
		}
		return *this;
	}
	Bands& fma(TParam a, const Bands& b)
	{
		for (size_t i = 0; i < N; ++i)
		{
			v_[i] += a * b.v_[i];
		}
		return *this;
	}
	Bands& fma(const Bands& a, TParam b)
	{
		for (size_t i = 0; i < N; ++i)
		{
			v_[i] += a.v_[i] * b;
		}
		return *this;
	}

	Bands& inpabs()
	{
		for (size_t i = 0; i < N; ++i)
		{
			v_[i] = num::abs(v_[i]);
		}
		return *this;
	}
	Bands& inpmax(const Bands& other)
	{
		for (size_t i = 0; i < N; ++i)
		{
			v_[i] = std::max(v_[i], other.v_[i]);
		}
		return *this;
	}
	Bands& inpmax(TParam f)
	{
		for (size_t i = 0; i < N; ++i)
		{
			v_[i] = std::max(v_[i], f);
		}
		return *this;
	}
	Bands& inppow(const Bands& other)
	{
		for (size_t i = 0; i < N; ++i)
		{
			v_[i] = num::pow(v_[i], other.v_[i]);
		}
		return *this;
	}
	Bands& inppow(TParam f)
	{
		for (size_t i = 0; i < N; ++i)
		{
			v_[i] = num::pow(v_[i], f);
		}
		return *this;
	}
	Bands& inpsqrt()
	{
		for (size_t i = 0; i < N; ++i)
		{
			v_[i] = num::sqrt(v_[i]);
		}
		return *this;
	}
	Bands& inpexp()
	{
		for (size_t i = 0; i < N; ++i)
		{
			v_[i] = num::exp(v_[i]);
		}
		return *this;
	}
	Bands& inpclamp(TParam min, TParam max)
	{
		for (size_t i = 0; i < N; ++i)
		{
			v_[i] = num::clamp(v_[i], min, max);
		}
		return *this;
	}
	Bands& inplerp(const Bands& other, TParam f)
	{
		for (size_t i = 0; i < N; ++i)
		{
			v_[i] = num::lerp(v_[i], other.v_[i], f);
		}
		return *this;
	}
    Bands& inpsin()
    {
        for (size_t i = 0; i < N; ++i)
        {
            v_[i] = num::sin(v_[i]);
        }
        return *this;
    }

	TValue dot(const Bands& other) const
	{
		TValue sum = TNumTraits::zero;
		for (size_t i = 0; i < N; ++i)
		{
			sum += v_[i] * other.v_[i];
		}
		return sum;
	}

	TValue average() const
	{
		return std::accumulate(v_, v_ + numBands, TNumTraits::zero) / numBands;
	}

	TValue minimum() const
	{
		return *std::min_element(v_, v_ + numBands);
	}

	TValue maximum() const
	{
		return *std::max_element(v_, v_ + numBands);
	}

	bool isZero() const
	{
		for (size_t i = 0; i < N; ++i)
		{
			if (v_[i] != 0)
			{
				return false;
			}
		}
		return true;
	}

	bool operator==(const Bands& other) const
	{
		for (size_t i = 0; i < N; ++i)
		{
			if (v_[i] != other.v_[i]) 
			{
				return false;
			}
		}
		return true;
	}

private:

	TValue v_[N];
};

// --- 1 band ---

template <typename T>
class Bands<1, T>
{
public:

	typedef typename util::CallTraits<T>::TValue TValue;
	typedef typename util::CallTraits<T>::TParam TParam;
	typedef typename util::CallTraits<T>::TReference TReference;
	typedef typename util::CallTraits<T>::TConstReference TConstReference;
	typedef num::NumTraits<T> TNumTraits;

	enum { numBands = 1 };

	Bands(TParam f = TNumTraits::zero): v_(f) {}
	void fill(TParam f) { v_ = f; }

	TReference operator[](size_t) { return v_; }
	TParam operator[](size_t) const { return v_; }

	Bands& operator+=(const Bands& other) { v_ += other.v_; return *this; }
	Bands& operator-=(const Bands& other) { v_ -= other.v_; return *this; }
	Bands& operator*=(const Bands& other) { v_ *= other.v_; return *this; }
	Bands& operator/=(const Bands& other) { v_ /= other.v_; return *this; }

	Bands& operator+=(TParam f) { v_ += f; return *this; }
	Bands& operator-=(TParam f) { v_ -= f; return *this; }
	Bands& operator*=(TParam f) { v_ *= f; return *this; }
	Bands& operator/=(TParam f) { v_ /= f; return *this; }

	Bands& fma(const Bands& a, const Bands& b) { v_ += a.v_ * b.v_; return *this; }
	Bands& fma(TParam a, const Bands& b) { v_ += a * b.v_; return *this; }
	Bands& fma(const Bands& a, TParam b)  { v_ += a.v_ * b; return *this; }

	Bands& inpabs() { v_ = num::abs(v_); return *this; }
	Bands& inpmax(const Bands& other) { v_ = std::max(v_, other.v_); return *this; }
	Bands& inpmax(TParam f) { v_ = std::max(v_, f); return *this; }
	Bands& inppow(const Bands& other) { v_ = num::pow(v_, other.v_); return *this; }
	Bands& inppow(TParam f) { v_ = num::pow(v_, f); return *this; }
	Bands& inpsqrt() { v_ = num::sqrt(v_); return *this; }
	Bands& inpexp() { v_ = num::exp(v_); return *this; }
	Bands& inpclamp(TParam min, TParam max) { v_ = num::clamp(v_, min, max); return *this; }
	Bands& inplerp(const Bands& other, TParam f) { v_ = num::lerp(v_, other.v_, f); return *this; }
    Bands& inpsin() { v_ = num::sin(v_); return *this; }

	TValue dot(const Bands& other) const { return v_ * other.v_; }
	TValue average() const { return v_; }
	TValue minimum() const { return v_; }
	TValue maximum() const { return v_; }

	bool isZero() const { return v_ == 0; }
	bool operator==(const Bands& other) const { return v_ == other.v_; }

private:

	TValue v_;
};


template <typename T>
class LASS_SIMD_ALIGN Bands<3, T>
{
public:

	typedef typename util::CallTraits<T>::TValue TValue;
	typedef typename util::CallTraits<T>::TParam TParam;
	typedef typename util::CallTraits<T>::TReference TReference;
	typedef typename util::CallTraits<T>::TConstReference TConstReference;
	typedef num::NumTraits<T> TNumTraits;

	enum { numBands = 3 };

	Bands(TParam f = TNumTraits::zero)
	{
		v_[0] = v_[1] = v_[2] = f;
	}
	Bands(TParam x, TParam y, TParam z)
	{
		v_[0] = x;
		v_[1] = y;
		v_[2] = z;
	}

	void fill(TParam f)
	{
		v_[0] = v_[1] = v_[2] = f;
	}

	TReference operator[](size_t i) { return v_[i]; }
	TParam operator[](size_t i) const { return v_[i]; }

	Bands& operator+=(const Bands& other)
	{
		v_[0] += other.v_[0];
		v_[1] += other.v_[1];
		v_[2] += other.v_[2];
		return *this;
	}
	Bands& operator-=(const Bands& other)
	{
		v_[0] -= other.v_[0];
		v_[1] -= other.v_[1];
		v_[2] -= other.v_[2];
		return *this;
	}
	Bands& operator*=(const Bands& other)
	{
		v_[0] *= other.v_[0];
		v_[1] *= other.v_[1];
		v_[2] *= other.v_[2];
		return *this;
	}
	Bands& operator/=(const Bands& other)
	{
		v_[0] /= other.v_[0];
		v_[1] /= other.v_[1];
		v_[2] /= other.v_[2];
		return *this;
	}

	Bands& operator+=(TParam f)
	{
		v_[0] += f;
		v_[1] += f;
		v_[2] += f;
		return *this;
	}
	Bands& operator-=(TParam f)
	{
		v_[0] -= f;
		v_[1] -= f;
		v_[2] -= f;
		return *this;
	}
	Bands& operator*=(TParam f)
	{
		v_[0] *= f;
		v_[1] *= f;
		v_[2] *= f;
		return *this;
	}
	Bands& operator/=(TParam f)
	{
		v_[0] /= f;
		v_[1] /= f;
		v_[2] /= f;
		return *this;
	}

	Bands& fma(const Bands& a, const Bands& b)
	{
		v_[0] += a.v_[0] * b.v_[0];
		v_[1] += a.v_[1] * b.v_[1];
		v_[2] += a.v_[2] * b.v_[2];
		return *this;
	}
	Bands& fma(TParam a, const Bands& b)
	{
		v_[0] += a * b.v_[0];
		v_[1] += a * b.v_[1];
		v_[2] += a * b.v_[2];
		return *this;
	}
	Bands& fma(const Bands& a, TParam b)
	{
		v_[0] += a.v_[0] * b;
		v_[1] += a.v_[1] * b;
		v_[2] += a.v_[2] * b;
		return *this;
	}

	Bands& inpabs()
	{
		v_[0] = num::abs(v_[0]);
		v_[1] = num::abs(v_[1]);
		v_[2] = num::abs(v_[2]);
		return *this;
	}
	Bands& inpmax(const Bands& other)
	{
		v_[0] = std::max(v_[0], other.v_[0]);
		v_[1] = std::max(v_[1], other.v_[1]);
		v_[2] = std::max(v_[2], other.v_[2]);
		return *this;
	}
	Bands& inpmax(TParam f)
	{
		v_[0] = std::max(v_[0], f);
		v_[1] = std::max(v_[1], f);
		v_[2] = std::max(v_[2], f);
		return *this;
	}
	Bands& inppow(const Bands& other)
	{
		v_[0] = num::pow(v_[0], other.v_[0]);
		v_[1] = num::pow(v_[1], other.v_[1]);
		v_[2] = num::pow(v_[2], other.v_[2]);
		return *this;
	}
	Bands& inppow(TParam f)
	{
		v_[0] = num::pow(v_[0], f);
		v_[1] = num::pow(v_[1], f);
		v_[2] = num::pow(v_[2], f);
		return *this;
	}
	Bands& inpsqrt()
	{
		v_[0] = num::sqrt(v_[0]);
		v_[1] = num::sqrt(v_[1]);
		v_[2] = num::sqrt(v_[2]);
		return *this;
	}
	Bands& inpexp()
	{
		v_[0] = num::exp(v_[0]);
		v_[1] = num::exp(v_[1]);
		v_[2] = num::exp(v_[2]);
		return *this;
	}
	Bands& inpclamp(TParam min, TParam max)
	{
		v_[0] = num::clamp(v_[0], min, max);
		v_[1] = num::clamp(v_[1], min, max);
		v_[2] = num::clamp(v_[2], min, max);
		return *this;
	}
	Bands& inplerp(const Bands& other, TParam f)
	{
		v_[0] = num::lerp(v_[0], other.v_[0], f);
		v_[1] = num::lerp(v_[1], other.v_[1], f);
		v_[2] = num::lerp(v_[2], other.v_[2], f);
		return *this;
	}
    Bands& inpsin()
    {
        v_[0] = num::sin(v_[0]);
        v_[1] = num::sin(v_[1]);
        v_[2] = num::sin(v_[2]);
        return *this;
    }

	TValue dot(const Bands& other) const
	{
		return v_[0] * other.v_[0] + v_[1] * other.v_[1] + v_[2] * other.v_[2];
	}

	TValue average() const
	{
		return (v_[0] + v_[1] + v_[2]) / 3;
	}

	TValue minimum() const
	{
		return std::min(v_[0], std::min(v_[1], v_[2]));
	}

	TValue maximum() const
	{
		return std::max(v_[0], std::max(v_[1], v_[2]));
	}

	bool isZero() const
	{
		return v_[0] == 0 && v_[1] == 0 && v_[2] == 0;
	}

	bool operator==(const Bands& other) const
	{
		return v_[0] == other.v_[0] && v_[1] == other.v_[1] && v_[2] == other.v_[2];
	}

private:

	TValue v_[3];
};

}

}

#endif

// EOF
