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
class Bands
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
	Bands& inplerp(const Bands& other, TScalar f)
	{
		for (size_t i = 0; i < N; ++i)
		{
			v_[i] = num::lerp(v_[i], other.v_[i], f);
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

	TValue total() const
	{
		return std::accumulate(v_, v_ + numBands, TNumTraits::zero);
	}
	TValue absTotal() const
	{
		TValue sum = TNumTraits::zero;
		for (size_t i = 0; i < N; ++i)
		{
			sum += num::abs(v_[i]);
		}
		return sum;
	}

	bool isZero() const
	{
		for (size_t i = 0; i < N; ++i)
		{
			if (v_[i])
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

// --- 3 bands ---

template <typename T>
class Bands<3, T>
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
	Bands& inppow(const Bands& other)
	{
		v_[0] = num::pow(v_[0], other.v_[0]);
		v_[1] = num::pow(v_[1], other.v_[1]);
		v_[2] = num::pow(v_[2], other.v_[2]);
		return *this;
	}
	Bands& inppow(TScalar f)
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
	Bands& inplerp(const Bands& other, TScalar f)
	{
		v_[0] = num::lerp(v_[0], other.v_[0], f);
		v_[1] = num::lerp(v_[1], other.v_[1], f);
		v_[2] = num::lerp(v_[2], other.v_[2], f);
		return *this;
	}

	TValue dot(const Bands& other) const
	{
		return v_[0] * other.v_[0] + v_[1] * other.v_[1] + v_[2] * other.v_[2];
	}

	TValue total() const
	{
		return v_[0] + v_[1] + v_[2];
	}
	TValue absTotal() const
	{
		return num::abs(v_[0]) + num::abs(v_[1]) + num::abs(v_[2]);
	}

	bool isZero() const
	{
		return !(v_[0] || v_[1] || v_[2]);
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
