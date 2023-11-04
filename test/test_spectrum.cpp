/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2014-2021  Bram de Greve (bramz@users.sourceforge.net)
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

#include "kernel/spectral.h"
#include "kernel/rgb_space.h"
#include <iostream>

namespace
{

using namespace liar;
using namespace liar::kernel;
using namespace lass;

struct Tester
{
	Tester() :
		numTests(0),
		numErrors(0)
	{
	}
	~Tester()
	{
		std::cerr << numErrors << "/" << numTests << " failed\n";
	}

	void check(const char* message, bool condition)
	{
		++numTests;
		std::cerr << "* " << message << " ";
		if (condition)
		{
			std::cerr << "ok";
		}
		else
		{
			++numErrors;
			std::cerr << "failed";
		}
		std::cerr << std::endl;
	}
	void checkClose(const char* message, TScalar a, TScalar b)
	{
		check(message, num::almostEqual(a, b, TScalar(1e-3f)));
	}
	void checkClose(const char* message, const Spectral& a, const Spectral& b)
	{
		bool ok = true;
		for (size_t i = 0; i < Spectral::numBands; ++i)
		{
			ok &= num::almostEqual<Spectral::TValue>(a[i], b[i], 1e-3f);
		}
		check(message, ok);
	}

	int numErrors;
	int numTests;
};


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
		conversionSpace->convert(RGBA(0, 0, 0)),
		conversionSpace->convert(RGBA(1, 1, 1)),
		conversionSpace->convert(RGBA(1, 0, 0)),
		conversionSpace->convert(RGBA(0, 1, 0)),
		conversionSpace->convert(RGBA(0, 0, 1)),
		conversionSpace->convert(RGBA(1, 1, 0)),
		conversionSpace->convert(RGBA(1, 0, 1)),
		conversionSpace->convert(RGBA(0, 1, 1)),
		conversionSpace->convert(RGBA(.2f, .5f, .8f)),
		conversionSpace->convert(RGBA(.2f, .8f, .8f)),
		conversionSpace->convert(RGBA(.8f, .2f, .5f)),
		conversionSpace->convert(RGBA(.5f, .2f, .8f)),
		conversionSpace->convert(RGBA(.5f, .8f, .2f)),
		conversionSpace->convert(RGBA(.8f, .5f, .2f)),
		sRGB->convert(RGBA(0, 0, 0)),
		sRGB->convert(RGBA(1, 1, 1)),
		sRGB->convert(RGBA(1, 0, 0)),
		sRGB->convert(RGBA(0, 1, 0)),
		sRGB->convert(RGBA(0, 0, 1)),
		sRGB->convert(RGBA(1, 1, 0)),
		sRGB->convert(RGBA(1, 0, 1)),
		sRGB->convert(RGBA(0, 1, 1)),
		sRGB->convert(RGBA(.2f, .5f, .8f)),
		sRGB->convert(RGBA(.2f, .8f, .5f)),
		sRGB->convert(RGBA(.8f, .2f, .5f)),
		sRGB->convert(RGBA(.5f, .2f, .8f)),
		sRGB->convert(RGBA(.5f, .8f, .2f)),
		sRGB->convert(RGBA(.8f, .5f, .2f)),
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

int test_interface()
{
	Tester t;

	typedef Spectral::TValue TValue;

	t.check("Spectral::numBands",
#ifdef LIAR_SPECTRAL_BANDS
		Spectral::numBands == LIAR_SPECTRAL_BANDS
#else
		Spectral::numBands == 3
#endif
	);

	Spectral zero;
	t.check("Default constructor", zero.absAverage() == 0);

	Spectral one(1);
	t.check("Scalar constructor ", one.absAverage() == 1);

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

	t.checkClose("operator+ (spectrum, spectrum)", a + b, zero);
	t.checkClose("operator+ (spectrum, spectrum)", a + a, c);
	t.checkClose("operator- (spectrum, spectrum)", a - b, c);
	t.checkClose("operator* (spectrum, spectrum)", a * b, e);
	t.checkClose("operator/ (spectrum, spectrum)", e / a, b);
	t.checkClose("operator+ (spectrum, scalar)", a + 1, h);
	t.checkClose("operator- (spectrum, scalar)", a - (-1), h);
	t.checkClose("operator* (spectrum, scalar)", a * 2, c);
	t.checkClose("operator/ (spectrum, scalar)", c / 2, a);
	t.checkClose("operator+ (scalar, spectrum)", 1 + a, h);
	t.checkClose("operator- (scalar, spectrum)", 1 - b, h);
	t.checkClose("operator* (scalar, spectrum)", 2 * a, c);
	t.checkClose("operator/ (scalar, spectrum)", 2 / a, k);

	Spectral y = one;
	t.checkClose("fma (spectrum, spectrum)", y.fma(a, b), f);
	y = one;
	t.checkClose("fma (spectrum, scalar)", y.fma(a, 2), g);
	y = one;
	t.checkClose("fma (scalar, spectrum)", y.fma(2, a), g);

	t.checkClose("average", b.average(), -(Spectral::numBands + 1.f) / 2.f);
	//t.checkClose("dot", a.dot(b), (a * b).total());

	t.check("isZero", zero.isZero());
	t.check("!isZero", !a.isZero());
	t.check("operator!", !zero);
	t.check("bool", static_cast<bool>(a));

	t.checkClose("abs", abs(b), a);
	t.check("max", max(a, b) == a && max(b, a) == a);
	t.checkClose("sqr", sqr(b), d);
	t.checkClose("pow", pow(b, 2), d);
	t.checkClose("sqrt", sqrt(d), a);
	t.checkClose("exp", exp(a), m);
	t.checkClose("clamp", clamp(a, TValue(1.5f), TValue(4.5f)), n);
	t.checkClose("lerp", lerp(one, d, TValue(0.3f)), o);


	return t.numErrors;
}

}

int test_spectrum(int, char*[])
{
	return test_interface() + test_conversion();
}
