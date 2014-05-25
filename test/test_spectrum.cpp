#include "kernel/spectrum.h"
#include <iostream>

using namespace liar;
using namespace liar::kernel;
using namespace lass;

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

int test_spectrum(int, char*[])
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
	std::cerr << numTests << " tests\n";

	int errors = 0;
	for (size_t k = 0; k < numTests; ++k)
	{
		if (!test_xyz(tests[k]))
		{
			++errors;
		}
	}

	return errors;
}
