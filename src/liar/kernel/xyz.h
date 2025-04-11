/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2025  Bram de Greve (bramz@users.sourceforge.net)
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

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_KERNEL_XYZ_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_KERNEL_XYZ_H

#include "kernel_common.h"

namespace liar
{
namespace kernel
{

class LASS_SIMD_ALIGN XYZ
{
public:
	typedef LIAR_VALUE TValue;
	typedef LIAR_VALUE TParam;
	typedef num::NumTraits<TValue> TNumTraits;

	TValue x;
	TValue y;
	TValue z;

	XYZ(): x(0), y(0), z(0) {}
	XYZ(TParam f) : x(f), y(f), z(f) {}
	XYZ(TParam x, TParam y, TParam z) : x(x), y(y), z(z) {}

	XYZ& operator+=(const XYZ& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}
	XYZ& operator-=(const XYZ& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}
	XYZ& operator*=(const XYZ& other)
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		return *this;
	}
	XYZ& operator/=(const XYZ& other)
	{
		x /= other.x;
		y /= other.y;
		z /= other.z;
		return *this;
	}
	TValue total() const
	{
		return x + y + z;
	}
	TValue absTotal() const
	{
		return num::abs(x) + num::abs(y) + num::abs(z);
	}
	TValue average() const
	{
		return total() / 3;
	}
	TValue maximum() const
	{
		return std::max(x, std::max(y, z));
	}
	bool isZero() const
	{
		return x == 0 && y == 0 && z == 0;
	}
	bool isNaN() const
	{
		return num::isNaN(total());
	}
	bool operator!() const
	{
		return isZero();
	}
	explicit operator bool() const
	{
		return !isZero();
	}
};

inline XYZ abs(const XYZ& a)
{
	return XYZ(num::abs(a.x), num::abs(a.y), num::abs(a.z));
}

inline XYZ max(const XYZ& a, const XYZ& b)
{
	return XYZ(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
}

inline XYZ::TValue average(const XYZ& a)
{
	return (a.x + a.y + a.z) / 3;
}

inline XYZ::TValue dot(const XYZ& a, const XYZ& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline XYZ operator+(const XYZ& a, const XYZ& b)
{
	XYZ r(a);
	r += b;
	return r;
}

inline XYZ operator-(const XYZ& a, const XYZ& b)
{
	XYZ r(a);
	r -= b;
	return r;
}
inline XYZ operator*(const XYZ& a, const XYZ& b)
{
	XYZ r(a);
	r *= b;
	return r;
}
inline XYZ operator/(const XYZ& a, const XYZ& b)
{
	XYZ r(a);
	r /= b;
	return r;
}
inline XYZ sqr(const XYZ& a)
{
	return XYZ(a.x * a.x, a.y * a.y, a.z * a.z);
}
inline XYZ pow(const XYZ& a, XYZ::TParam b)
{
	return XYZ(num::pow(a.x, b), num::pow(a.y, b), num::pow(a.z, b));
}
inline XYZ exp(const XYZ& a)
{
	return XYZ(num::exp(a.x), num::exp(a.y), num::exp(a.z));
}
inline XYZ clamp(const XYZ& a, XYZ::TParam min, XYZ::TParam max)
{
	return XYZ(num::clamp(a.x, min, max), num::clamp(a.y, min, max), num::clamp(a.z, min, max));
}
inline XYZ lerp(const XYZ& a, const XYZ& b, XYZ::TParam f)
{
	return XYZ(num::lerp(a.x, b.x, f), num::lerp(a.y, b.y, f), num::lerp(a.z, b.z, f));
}
inline bool operator==(const XYZ& a, const XYZ& b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

inline const XYZ chromaticity(const XYZ& xyz)
{
	const XYZ::TValue sum = xyz.x + xyz.y + xyz.z;
	return sum == 0 ? 0 : xyz / sum;
}

LIAR_KERNEL_DLL std::ostream& LASS_CALL operator<<(std::ostream& os, const XYZ& xyz);

}
}

namespace lass
{
namespace python
{
template <>
struct PyExportTraits<liar::kernel::XYZ>
{
	constexpr static const char* py_typing = "tuple[float, float, float]";

	static PyObject* build(const liar::kernel::XYZ& v)
	{
		return fromSharedPtrToNakedCast(python::makeTuple(v.x, v.y, v.z));
	}
	static int get(PyObject* obj, liar::kernel::XYZ& v)
	{
		return python::decodeTuple(obj, v.x, v.y, v.z);
	}
};
}
}

#endif

// EOF
