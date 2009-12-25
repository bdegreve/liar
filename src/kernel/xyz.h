/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
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

class XYZ
{
public:
	TScalar x;
	TScalar y;
	TScalar z;

	XYZ(): x(0), y(0), z(0) {}
	XYZ(TScalar f): x(f), y(f), z(f) {}
	XYZ(TScalar x, TScalar y, TScalar z): x(x), y(y), z(z) {}

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
	TScalar total() const 
	{
		return x + y + z;
	}
	TScalar absTotal() const
	{
		return num::abs(x) + num::abs(y) + num::abs(z);
	}
	TScalar average() const
	{
		return total() / 3;
	}
	bool isZero() const
	{
		return x == 0 && y == 0 && z == 0;
	}
	bool operator!() const
	{
		return isZero();
	}
	operator num::SafeBool() const
	{
		return isZero() ? num::safeFalse : num::safeTrue;
	}
};

inline TScalar average(const XYZ& a)
{
	return a.x + a.y + a.z / 3;
}

inline TScalar positiveAverage(const XYZ& a)
{
	return std::max<TScalar>(average(a), 0);
}

inline TScalar dot(const XYZ& a, const XYZ& b)
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
inline XYZ pow(const XYZ& a, TScalar b)
{
	return XYZ(num::pow(a.x, b), num::pow(a.y, b), num::pow(a.z, b));
}
inline XYZ clamp(const XYZ& a, TScalar min, TScalar max)
{
	return XYZ(num::clamp(a.x, min, max), num::clamp(a.y, min, max), num::clamp(a.z, min, max));
}
inline XYZ lerp(const XYZ& a, const XYZ& b, TScalar f)
{
	return XYZ(num::lerp(a.x, b.x, f), num::lerp(a.y, b.y, f), num::lerp(a.z, b.z, f));
}
inline bool operator==(const XYZ& a, const XYZ& b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

class LIAR_KERNEL_DLL Observer
{
public:
	typedef std::pair<TFrequency, TFrequency> TFrequencyRange;

	template <typename InputIteratorWavelength, typename InputIteratorXYZ>
	Observer(InputIteratorWavelength firstW, InputIteratorWavelength lastW, InputIteratorXYZ firstXYZ)
	{
		while (firstW != lastW)
		{
			nodes_.push_back(Node(*firstW++, *firstXYZ++));
		}
		init();
	}

	const TFrequencyRange frequencyRange() const;
	const XYZ tristimulus(TFrequency frequency) const;
	const XYZ chromaticity(TFrequency frequency) const;
	TFrequency sample(const XYZ& power, TScalar sample, XYZ& chromaticity, TScalar& pdf) const;

private:
	struct Node
	{
		TScalar wavelength;
		XYZ xyz;
		XYZ dxyz_dw;
		XYZ cdf;
		Node(TScalar w, XYZ xyz): wavelength(w), xyz(xyz) {}
	};
	typedef std::vector<Node> TNodes;

	void init();

	TNodes nodes_;
};

LIAR_KERNEL_DLL const XYZ chromaticity(const XYZ& xyz);

LIAR_KERNEL_DLL const Observer& standardObserver();
LIAR_KERNEL_DLL const XYZ tristimulus(TFrequency frequency);
LIAR_KERNEL_DLL const XYZ chromaticity(TFrequency frequency);
LIAR_KERNEL_DLL TFrequency sampleFrequency(const XYZ& power, TScalar sample, XYZ& chromaticity, TScalar& pdf);

}
}

namespace lass
{
namespace python
{
template <>
struct PyExportTraits<liar::kernel::XYZ>
{
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
