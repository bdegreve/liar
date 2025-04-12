/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2014-2025  Bram de Greve (bramz@users.sourceforge.net)
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

#include <liar/kernel/spectrum.h>
#include <liar/kernel/rgb_space.h>
#include <iostream>

#include <gtest/gtest.h>
#include <lass/python/python_api.h>

namespace
{

using namespace liar;
using namespace liar::kernel;
using namespace lass;

bool almostEqual(const Spectral& a, const Spectral& b)
{
	constexpr Spectral::TValue tol = 1e-3f;
	for (size_t i = 0; i < Spectral::numBands; ++i)
	{
		if (!num::almostEqual(a[i], b[i], tol))
		{
			return false;
		}
	}
	return true;
}

bool test_xyz(const XYZ& xyz)
{
	/*const Spectral spectrum(xyz);
	const XYZ test = spectrum.xyz();
	const TScalar error = num::sqrt(sqr(xyz - test).total());
	if (error < 1e-3)
	{
		return true;
	}
	std::cerr << "failed XYZ conversion of " << xyz.x << ", " << xyz.y << ", " << xyz.z << std::endl;
	std::cerr << "returned: " << test.x << ", " << test.y << ", " << test.z << " with error " << error << std::endl;
	return false;
	*/
	return true;
}


int test_conversion()
{
	typedef RgbSpace::RGBA RGBA;

	TRgbSpacePtr conversionSpace(new RgbSpace(sRGB->red(), sRGB->green(), sRGB->blue(), TPoint2D(1.f / 3, 1.f / 3), 1.));

	const XYZ tests[] = {
		XYZ(0, 0, 0),
		XYZ(1, 1, 1),
		XYZ(1, 0, 0),
		XYZ(0, 1, 0),
		XYZ(0, 0, 1),
		XYZ(1, 1, 0),
		XYZ(1, 0, 1),
		XYZ(0, 1, 1),
		XYZ(.2f, .5f, .8f),
		XYZ(.2f, .8f, .5f),
		XYZ(.8f, .2f, .5f),
		XYZ(.5f, .2f, .8f),
		XYZ(.5f, .8f, .2f),
		XYZ(.8f, .5f, .2f),
		conversionSpace->toXYZ(RGBA(0, 0, 0)),
		conversionSpace->toXYZ(RGBA(1, 1, 1)),
		conversionSpace->toXYZ(RGBA(1, 0, 0)),
		conversionSpace->toXYZ(RGBA(0, 1, 0)),
		conversionSpace->toXYZ(RGBA(0, 0, 1)),
		conversionSpace->toXYZ(RGBA(1, 1, 0)),
		conversionSpace->toXYZ(RGBA(1, 0, 1)),
		conversionSpace->toXYZ(RGBA(0, 1, 1)),
		conversionSpace->toXYZ(RGBA(.2f, .5f, .8f)),
		conversionSpace->toXYZ(RGBA(.2f, .8f, .8f)),
		conversionSpace->toXYZ(RGBA(.8f, .2f, .5f)),
		conversionSpace->toXYZ(RGBA(.5f, .2f, .8f)),
		conversionSpace->toXYZ(RGBA(.5f, .8f, .2f)),
		conversionSpace->toXYZ(RGBA(.8f, .5f, .2f)),
		sRGB->toXYZ(RGBA(0, 0, 0)),
		sRGB->toXYZ(RGBA(1, 1, 1)),
		sRGB->toXYZ(RGBA(1, 0, 0)),
		sRGB->toXYZ(RGBA(0, 1, 0)),
		sRGB->toXYZ(RGBA(0, 0, 1)),
		sRGB->toXYZ(RGBA(1, 1, 0)),
		sRGB->toXYZ(RGBA(1, 0, 1)),
		sRGB->toXYZ(RGBA(0, 1, 1)),
		sRGB->toXYZ(RGBA(.2f, .5f, .8f)),
		sRGB->toXYZ(RGBA(.2f, .8f, .5f)),
		sRGB->toXYZ(RGBA(.8f, .2f, .5f)),
		sRGB->toXYZ(RGBA(.5f, .2f, .8f)),
		sRGB->toXYZ(RGBA(.5f, .8f, .2f)),
		sRGB->toXYZ(RGBA(.8f, .5f, .2f)),
	};
	const size_t numTests = sizeof(tests) / sizeof(XYZ);

	int errors = 0;
	for (size_t k = 0; k < numTests; ++k)
	{
		if (!test_xyz(tests[k]))
		{
			++errors;
		}
	}

	std::cerr << errors << "/" << numTests << " failed\n";

	return errors;
}

template <typename R, typename... Args > R callMethod(const python::TPyObjPtr& obj, const char* method, Args&&... args)
{
	using namespace lass::python;

	LockGIL LASS_UNUSED(lock);
	auto tuple = makeTuple(std::forward<Args>(args)...);

	TPyObjPtr callable(PY_ENFORCE_POINTER(PyObject_GetAttrString(obj.get(), method)));
	TPyObjPtr ret(PY_ENFORCE_POINTER(PyObject_CallObject(callable.get(), tuple.get())));
	typedef ArgumentTraits<R> TraitsR;
	typename TraitsR::TStorage temp;
	PY_ENFORCE_ZERO(pyGetSimpleObject(ret.get(), temp));
	return TraitsR::arg(temp);
}

template <typename R, typename T, typename... Args > R callMethod(const kernel::PyObjectRef<T>& obj, const char* method, Args&&... args)
{
	return callMethod<R>(obj.ptr(), std::forward<Args>(args)...);
}

template <typename R> R getAttr(const python::TPyObjPtr& obj, const char* name)
{
	using namespace lass::python;

	LockGIL LASS_UNUSED(lock);
	TPyObjPtr attrObj(PY_ENFORCE_POINTER(PyObject_GetAttrString(obj.get(), name)));
	typedef ArgumentTraits<R> TraitsR;
	typename TraitsR::TStorage temp;
	PY_ENFORCE_ZERO(pyGetSimpleObject(attrObj.get(), temp));
	return TraitsR::arg(temp);
}

template <typename R, typename T> R getAttr(const kernel::PyObjectRef<T>& obj, const char* name)
{
	return getAttr<R>(obj.ptr(), name);
}

}

TEST(Spectrum, NumberOfBands)
{
#if LIAR_SPECTRAL_MODE_BANDED
	EXPECT_EQ(Spectral::numBands, LIAR_SPECTRAL_MODE_BANDED);
#elif LIAR_SPECTRAL_MODE_SINGLE
	EXPECT_EQ(Spectral::numBands, 1);
#else
	EXPECT_EQ(Spectral::numBands, 3);
#endif
}

TEST(Spectrum, Constructors)
{
	Spectral zero;
	EXPECT_EQ(zero.absAverage(), 0);
	Spectral one(1);
	EXPECT_EQ(one.absAverage(), 1);
}

TEST(Spectrum, Operators)
{
	using TValue = Spectral::TValue;

	Spectral zero;
	Spectral one(1);
	Spectral a, b, c, d, e, f, g, h, k, m, n, o;
	for (size_t i = 0; i < Spectral::numBands; ++i)
	{
		const TValue x = static_cast<TValue>(i + 1);
		a[i] = x;
		b[i] = -x;
		c[i] = 2 * x;
		d[i] = x * x;
		e[i] = -x * x;
		f[i] = 1 - x * x;
		g[i] = 1 + 2 * x;
		h[i] = 1 + x;
		k[i] = 2 / x;
		m[i] = num::exp(x);
		n[i] = num::clamp(x, TValue(1.5f), TValue(4.5f));
		o[i] = num::lerp(TValue(1), x * x, TValue(.3f));
	}

	EXPECT_PRED2(almostEqual, a + b, zero);
	EXPECT_PRED2(almostEqual, a + a, c);
	EXPECT_PRED2(almostEqual, a - b, c);
	EXPECT_PRED2(almostEqual, a * b, e);
	EXPECT_PRED2(almostEqual, e / a, b);
	EXPECT_PRED2(almostEqual, a + 1, h);
	EXPECT_PRED2(almostEqual, a - (-1), h);
	EXPECT_PRED2(almostEqual, a * 2, c);
	EXPECT_PRED2(almostEqual, c / 2, a);
	EXPECT_PRED2(almostEqual, 1 + a, h);
	EXPECT_PRED2(almostEqual, 1 - b, h);
	EXPECT_PRED2(almostEqual, 2 * a, c);
	EXPECT_PRED2(almostEqual, 2 / a, k);

	Spectral y = one;
	EXPECT_PRED2(almostEqual, y.fma(a, b), f);
	y = one;
	EXPECT_PRED2(almostEqual, y.fma(a, 2), g);
	y = one;
	EXPECT_PRED2(almostEqual, y.fma(2, a), g);

	EXPECT_FLOAT_EQ(b.average(), -(static_cast<TValue>(Spectral::numBands) + 1.f) / 2.f);
	//t.checkClose("dot", a.dot(b), (a * b).total());

	EXPECT_TRUE(zero.isZero());
	EXPECT_TRUE(!a.isZero());
	EXPECT_TRUE(!zero);
	EXPECT_TRUE(static_cast<bool>(a));

	EXPECT_PRED2(almostEqual, abs(b), a);
	EXPECT_EQ(max(a, b), a);
	EXPECT_EQ(max(b, a), a);
	EXPECT_PRED2(almostEqual, sqr(b), d);
	EXPECT_PRED2(almostEqual, pow(b, 2), d);
	EXPECT_PRED2(almostEqual, sqrt(d), a);
	EXPECT_PRED2(almostEqual, exp(a), m);
	EXPECT_PRED2(almostEqual, clamp(a, TValue(1.5f), TValue(4.5f)), n);
	EXPECT_PRED2(almostEqual, lerp(one, d, TValue(0.3f)), o);
}

TEST(Spectrum, Sampled)
{
	using TWavelengths = std::vector<TWavelength>;
	using TValues = std::vector<Spectrum::TValue>;

	{
		auto sampled = lass::python::evaluate("liar.spectra.Sampled([400e-9, 500e-9, 600e-9], [0.1, 0.2, 0.])");

		TWavelengths ws = { 350e-9f, 375e-9f, 400e-9f, 425e-9f, 450e-9f, 475e-9f, 500e-9f, 525e-9f, 550e-9f, 575e-9f, 600e-9f, 625e-9f, 650e-9f };
		TValues vs =      { 0.f,     0.f,     0.1f,    0.125f,  0.15f,   0.175f,  0.2f,    0.15f,   0.1f,    0.05f,   0.f, 0.f, 0.f };

		auto resampled = callMethod<TSpectrumRef>(sampled, "resample", ws);
		auto ws2 = getAttr<TWavelengths>(resampled, "wavelengths");
		auto vs2 = getAttr<TValues>(resampled, "values");
		EXPECT_EQ(ws2.size(), ws.size());
		EXPECT_EQ(vs2.size(), vs2.size());

		for (size_t i = 0, n = std::min(ws.size(), ws2.size()); i < n; ++i)
			EXPECT_NEAR(ws[i], ws2[i], 1e-6f) << " at index " << i;
		for (size_t i = 0, n = std::min(vs.size(), vs2.size()); i < n; ++i)
			EXPECT_NEAR(vs[i], vs2[i], 1e-6f) << " at index " << i;
	}
}
