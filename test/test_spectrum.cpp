#include "kernel/spectrum.h"
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
	void checkClose(const char* message, const Spectrum& a, const Spectrum& b)
	{
		bool ok = true;
		for (size_t i = 0; i < Spectrum::numBands; ++i)
		{
			ok &= num::almostEqual<Spectrum::TValue>(a[i], b[i], 1e-3f);
		}
		check(message, ok);
	}

	int numErrors;
	int numTests;
};


bool test_xyz(const XYZ& xyz)
{
	const Spectrum spectrum(xyz);
	const XYZ test = spectrum.xyz();
	const TScalar error = num::sqrt(sqr(xyz - test).total());
	if (error < 1e-3)
	{
		return true;
	}
	std::cerr << "failed XYZ conversion of " << xyz.x << ", " << xyz.y << ", " << xyz.z << std::endl;
	std::cerr << "returned: " << test.x << ", " << test.y << ", " << test.z << " with error " << error << std::endl;
	return false;
}


int test_conversion()
{
	TRgbSpacePtr conversionSpace(new RgbSpace(sRGB->red(), sRGB->green(), sRGB->blue(), TPoint2D(1.f/3, 1.f/3), 1.));

	const XYZ tests[] = {
		XYZ(0, 0, 0),
		XYZ(1, 1, 1),
		XYZ(1, 0, 0),
		XYZ(0, 1, 0),
		XYZ(0, 0, 1),
		XYZ(1, 1, 0),
		XYZ(1, 0, 1),
		XYZ(0, 1, 1),
		XYZ(.2, .5, .8),
		XYZ(.2, .8, .5),
		XYZ(.8, .2, .5),
		XYZ(.5, .2, .8),
		XYZ(.5, .8, .2),
		XYZ(.8, .5, .2),
		rgb(0, 0, 0, conversionSpace),
		rgb(1, 1, 1, conversionSpace),
		rgb(1, 0, 0, conversionSpace),
		rgb(0, 1, 0, conversionSpace),
		rgb(0, 0, 1, conversionSpace),
		rgb(1, 1, 0, conversionSpace),
		rgb(1, 0, 1, conversionSpace),
		rgb(0, 1, 1, conversionSpace),
		rgb(.2, .5, .8, conversionSpace),
		rgb(.2, .8, .5, conversionSpace),
		rgb(.8, .2, .5, conversionSpace),
		rgb(.5, .2, .8, conversionSpace),
		rgb(.5, .8, .2, conversionSpace),
		rgb(.8, .5, .2, conversionSpace),	
		rgb(0, 0, 0, sRGB),
		rgb(1, 1, 1, sRGB),
		rgb(1, 0, 0, sRGB),
		rgb(0, 1, 0, sRGB),
		rgb(0, 0, 1, sRGB),
		rgb(1, 1, 0, sRGB),
		rgb(1, 0, 1, sRGB),
		rgb(0, 1, 1, sRGB),
		rgb(.2, .5, .8, sRGB),
		rgb(.2, .8, .5, sRGB),
		rgb(.8, .2, .5, sRGB),
		rgb(.5, .2, .8, sRGB),
		rgb(.5, .8, .2, sRGB),
		rgb(.8, .5, .2, sRGB),	
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

	t.check("Spectrum::numBands",
#ifdef LIAR_SPECTRAL_BANDS
		Spectrum::numBands == LIAR_SPECTRAL_BANDS
#else
		Spectrum::numBands == 3
#endif
	);

	Spectrum zero;
	t.check("Default constructor", zero.total() == 0);

	Spectrum one(1);
	t.check("Scalar constructor ", one.total() == Spectrum::numBands);

	Spectrum a, b, c, d, e, f, g, h, k, m, n, o;
	for (size_t i = 0; i < Spectrum::numBands; ++i)
	{
		const TScalar x = static_cast<TScalar>(i + 1);
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
		n[i] = num::clamp<TScalar>(x, 1.5f, 4.5f);
		o[i] = num::lerp<TScalar>(1, x * x, .3f);
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

	Spectrum y = one;
	t.checkClose("fma (spectrum, spectrum)", y.fma(a, b), f);
	y = one;
	t.checkClose("fma (spectrum, scalar)", y.fma(a, 2), g);
	y = one;
	t.checkClose("fma (scalar, spectrum)", y.fma(2, a), g);

	t.checkClose("total", a.total(), (Spectrum::numBands + 1.f) * Spectrum::numBands / 2.f);
	t.checkClose("absTotal", b.absTotal(), (Spectrum::numBands + 1.f) * Spectrum::numBands / 2.f);
	t.checkClose("average", b.average(), -(Spectrum::numBands + 1.f) / 2.f);
	t.checkClose("dot", a.dot(b), (a * b).total());

	t.check("isZero", zero.isZero());
	t.check("!isZero", !a.isZero());
	t.check("operator!", !zero);
	t.check("bool", a);

	t.checkClose("abs", abs(b), a);
	t.check("max", max(a, b) == a && max(b, a) == a);
	t.checkClose("sqr", sqr(b), d);
	t.checkClose("pow", pow(b, 2), d);
	t.checkClose("sqrt", sqrt(d), a);
	t.checkClose("exp", exp(a), m);
	t.checkClose("clamp", clamp(a, 1.5f, 4.5f), n);
	t.checkClose("lerp", lerp(one, d, 0.3f), o);


	return t.numErrors;
}

}

int test_spectrum(int, char*[])
{
	return test_interface() + test_conversion();
}