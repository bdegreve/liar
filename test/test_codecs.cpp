/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2023  Bram de Greve (bramz@users.sourceforge.net)
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

#include <gtest/gtest.h>

#include "kernel/image_codec.h"

using namespace liar;
using namespace liar::kernel;

namespace
{

using TRaster = std::vector<XYZ>;

/*

The colors of the test image are in sRGB space:

main colors:
(255,  0,  0), (255,255,  0), (  0,255,  0), (  0,255,255), (  0,  0,255), (255,  0,255),

main colors, watered down:
(255,128,128), (255,255,128), (128,255,128), (128,255,255), (128,128,255), (255,128,255),

from https://en.wikipedia.org/wiki/ColorChecker#Colors
orange       , dark skin    , light skin   , foliage      , yellow green , bluish green ,
(214,126, 44), (115, 82, 68), (194,150,130), ( 87,108, 67), (157,188, 64), (103,189,170),

L=0          , L=20         , L=40         , L=60         , L=80         , L=100        ,
(  0,  0,  0), ( 48, 48, 48), ( 94, 94, 94), (145,145,145), (198,198,198), (255,255,255),

When these colors (especially the ColorChecker ones) are also specified as CIE XYZ,
they usually give the XYZ values under the same Illuminant D65.
However, liar converts them to XYZ values under the Illuminant E.


To test we're doing all conversions right, the XYZ values below are computed with
and external tool: coloraide

Converting from sRGB (r, g, b) to CIE XYZ (x, y, z) was done as following:

from coloraide import cat, Color
D65 = cat.WHITES['2deg']["D65"]
E = cat.WHITES['2deg']["E"]
xyz_d65 = Color('srgb', [r / 255.0, g / 255.0, b / 255.0]).coords()
x, y, z = Color.chromatic_adaptation(D65, E, xyz_d65, method="bradford")

*/

const XYZ colors[4][6] =
{
	{ XYZ(0.4384f, 0.2228f, 0.0173f), XYZ(0.8306f, 0.9315f, 0.1278f), XYZ(0.3922f, 0.7087f, 0.1104f), XYZ(0.5616f, 0.7772f, 0.9827f), XYZ(0.1694f, 0.0685f, 0.8722f), XYZ(0.6078f, 0.2913f, 0.8896f) },
	{ XYZ(0.5597f, 0.3906f, 0.2294f), XYZ(0.8672f, 0.9463f, 0.316f ), XYZ(0.5234f, 0.7716f, 0.3025f), XYZ(0.6562f, 0.8253f, 0.9864f), XYZ(0.3487f, 0.2696f, 0.8998f), XYZ(0.6925f, 0.4443f, 0.9134f) },
	{ XYZ(0.3809f, 0.2994f, 0.0567f), XYZ(0.118f , 0.102f , 0.0627f), XYZ(0.394f , 0.3516f, 0.2377f), XYZ(0.1101f, 0.1314f, 0.0672f), XYZ(0.3537f, 0.4350f, 0.1061f), XYZ(0.3271f, 0.4184f, 0.4092f) },
	{ XYZ(0.0f   , 0.0f   , 0.0f   ), XYZ(0.0296f, 0.0296f, 0.0296f), XYZ(0.1119f, 0.1119f, 0.1119f), XYZ(0.2831f, 0.2831f, 0.2831f), XYZ(0.5647f, 0.5647f, 0.5647f), XYZ(1.0f   , 1.0f   , 1.0f   ) },
};

const std::filesystem::path test_src_dir(LASS_STRINGIFY(TEST_SOURCE_DIR));

TRaster read(ImageReader&& reader)
{
	const size_t nx = reader.resolution().x;
	const size_t ny = reader.resolution().y;
	if (nx == 0 || ny == 0)
	{
		return TRaster();
	}
	TRaster raster(nx * ny);
	reader.readFull(&raster[0]);
	return raster;
}

bool equalChromaticity(const TPoint2D& a, const TPoint2D& b)
{
	constexpr TPoint2D::TValue tol = 1e-3f;
	return num::almostEqual(a.x, b.x, tol) && num::almostEqual(a.y, b.y, tol);
}

bool equalDouble(double a, double b)
{
	return num::almostEqual(a, b, 1e-3);
}

bool equalXYZ(const XYZ& a, const XYZ& b)
{
	constexpr XYZ::TValue tol = 1.2e-2f;
	return num::abs(a.x - b.x) < tol && num::abs(a.y - b.y) < tol && num::abs(a.z - b.z) < tol;
}

#define EXPECT_CHROMATICITY(a, x, y) EXPECT_PRED2(equalChromaticity, a, TPoint2D(x, y))
#define EXPECT_DOUBLE(a, b) EXPECT_PRED2(equalDouble, a, b)

void testImage(ImageReader&& reader)
{
	const auto resolution = reader.resolution();
	EXPECT_EQ(resolution.x, 600);
	EXPECT_EQ(resolution.y, 400);

	const auto raster = read(std::move(reader));
	for (size_t y = 0; y < 4; ++y)
	{
		const size_t k0 = (100 * y + 50) * 600;
		for (size_t x = 0; x < 6; ++x)
		{
			const size_t k = k0 + (100 * x + 50);
			if (!equalXYZ(raster[k], colors[y][x]))
			{
				ADD_FAILURE() << "[" << y << "," << x << "]: color "
					<< raster[k] << " is not as expected " << colors[y][x];
			}
		}
	}
}

}

TEST(Codecs, lodepng_srgb)
{
	ImageReader reader(test_src_dir / "colors-srgb.png");

	const auto rgbSpace = reader.rgbSpace(); // use actual source space
	EXPECT_TRUE(rgbSpace);
	EXPECT_CHROMATICITY(rgbSpace->red(),   0.6400, 0.3300);
	EXPECT_CHROMATICITY(rgbSpace->green(), 0.3000, 0.6000);
	EXPECT_CHROMATICITY(rgbSpace->blue(),  0.1500, 0.0600);
	EXPECT_CHROMATICITY(rgbSpace->white(), 0.3127, 0.3290);
	EXPECT_DOUBLE(rgbSpace->gamma(), 2.2);

	testImage(std::move(reader));
}

TEST(Codecs, lodepng_adobergb)
{
	ImageReader reader(test_src_dir / "colors-adobergb.png");

	const auto rgbSpace = reader.rgbSpace(); // use actual source space
	EXPECT_TRUE(rgbSpace);
	EXPECT_CHROMATICITY(rgbSpace->red(), 0.6400, 0.3300);
	EXPECT_CHROMATICITY(rgbSpace->green(), 0.2100, 0.7100);
	EXPECT_CHROMATICITY(rgbSpace->blue(), 0.1500, 0.0600);
	EXPECT_CHROMATICITY(rgbSpace->white(), 0.3127, 0.3290);
	EXPECT_DOUBLE(rgbSpace->gamma(), 2.2);

	testImage(std::move(reader));
}

TEST(Codecs, jpeglib_srgb)
{
	ImageReader reader(test_src_dir / "colors-srgb.jpg");

	const auto rgbSpace = reader.rgbSpace(); // use actual source space
	EXPECT_TRUE(rgbSpace);
	EXPECT_CHROMATICITY(rgbSpace->red(), 0.6400, 0.3300);
	EXPECT_CHROMATICITY(rgbSpace->green(), 0.3000, 0.6000);
	EXPECT_CHROMATICITY(rgbSpace->blue(), 0.1500, 0.0600);
	EXPECT_CHROMATICITY(rgbSpace->white(), 0.3127, 0.3290);
	EXPECT_DOUBLE(rgbSpace->gamma(), 2.2);

	testImage(std::move(reader));
}

#if LIAR_HAVE_LCMS2_H

TEST(Codecs, jpeglib_adodbergb)
{
	ImageReader reader(test_src_dir / "colors-adobergb.jpg");
	/*
	const auto rgbSpace = reader.rgbSpace(); // use actual source space
	EXPECT_TRUE(rgbSpace);
	EXPECT_CHROMATICITY(rgbSpace->red(), 0.6400, 0.3300);
	EXPECT_CHROMATICITY(rgbSpace->green(), 0.2100, 0.7100);
	EXPECT_CHROMATICITY(rgbSpace->blue(), 0.1500, 0.0600);
	EXPECT_CHROMATICITY(rgbSpace->white(), 0.3127, 0.3290);
	EXPECT_DOUBLE(rgbSpace->gamma(), 2.2);
	*/

	testImage(std::move(reader));
}

#endif
