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

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_KERNEL_XYZA_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_KERNEL_XYZA_H

#include "kernel_common.h"
#include "xyz.h"

namespace liar
{
namespace kernel
{

class LASS_SIMD_ALIGN XYZA
{
public:
	typedef LIAR_VALUE TValue;
	typedef LIAR_VALUE TParam;
	typedef num::NumTraits<TValue> TNumTraits;

	TValue x;
	TValue y;
	TValue z;
	TValue a;

	XYZA(): x(0), y(0), z(0), a(0) {}
	explicit XYZA(TParam f, TParam a = 1) : x(f), y(f), z(f), a(a) {}
	explicit XYZA(const XYZ& xyz, TParam a = 1) : x(xyz.x), y(xyz.y), z(xyz.z), a(a) {}
	XYZA(TParam x, TParam y, TParam z, TParam a = 1) : x(x), y(y), z(z), a(a) {}

	XYZA& operator+=(const XYZA& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		a += other.a;
		return *this;
	}
	XYZA& operator-=(const XYZA& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		a -= other.a;
		return *this;
	}
	XYZA& operator*=(TParam f)
	{
		x *= f;
		y *= f;
		z *= f;
		a *= f;
		return *this;
	}
	XYZA& operator/=(TParam f)
	{
		x /= f;
		y /= f;
		z /= f;
		a /= f;
		return *this;
	}

	explicit operator XYZ() const
	{
		return XYZ(x, y, z);
	}
};

inline XYZA operator+(const XYZA& a, const XYZA& b)
{
	XYZA result(a);
	result += b;
	return result;
}

inline XYZA operator-(const XYZA& a, const XYZA& b)
{
	XYZA result(a);
	result -= b;
	return result;
}

inline XYZA operator*(const XYZA& a, XYZA::TParam b)
{
	XYZA result(a);
	result *= b;
	return result;
}

inline XYZA operator/(const XYZA& a, XYZA::TParam b)
{
	XYZA result(a);
	result /= b;
	return result;
}

inline XYZA operator*(XYZA::TParam a, const XYZA& b)
{
	XYZA result(b);
	result *= a;
	return result;
}

}
}

#endif

// EOF
