/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2024  Bram de Greve (bramz@users.sourceforge.net)
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

#include "textures_common.h"
#include "image.h"
#include "../kernel/image_codec.h"
#include <lass/io/file_attribute.h>
#include <lass/stde/extended_string.h>
#include <lass/python/export_traits_filesystem.h>
#include <lass/util/stop_watch.h>

#if LIAR_HAVE_AVX
#	include <array>
#endif

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(Image, "image file")
PY_CLASS_CONSTRUCTOR_1(Image, const std::filesystem::path&);
PY_CLASS_CONSTRUCTOR_2(Image, const std::filesystem::path&, const TRgbSpacePtr&);
PY_CLASS_CONSTRUCTOR_3(Image, const std::filesystem::path&, const std::string&, const std::string&);
PY_CLASS_CONSTRUCTOR_4(Image, const std::filesystem::path&, const std::string&, const std::string&, const TRgbSpacePtr&);
PY_CLASS_METHOD_QUALIFIED_1(Image, loadFile, void, const std::filesystem::path&)
PY_CLASS_METHOD_QUALIFIED_2(Image, loadFile, void, const std::filesystem::path&, const TRgbSpacePtr&)
PY_CLASS_MEMBER_R(Image, filename);
PY_CLASS_MEMBER_R(Image, resolution);
PY_CLASS_MEMBER_RW(Image, antiAliasing, setAntiAliasing);
PY_CLASS_MEMBER_RW(Image, mipMapping, setMipMapping);
PY_CLASS_STATIC_METHOD(Image, setDefaultAntiAliasing);
PY_CLASS_STATIC_METHOD(Image, setDefaultMipMapping);
PY_CLASS_STATIC_CONST(Image, "AA_NONE", "none");
PY_CLASS_STATIC_CONST(Image, "AA_BILINEAR", "bilinear");
PY_CLASS_STATIC_CONST(Image, "AA_TRILINEAR", "trilinear");
PY_CLASS_STATIC_CONST(Image, "MM_NONE", "none");
PY_CLASS_STATIC_CONST(Image, "MM_ISOTROPIC", "isotropic");
PY_CLASS_STATIC_CONST(Image, "MM_ANISOTROPIC", "anisotropic");

Image::TAntiAliasingDictionary Image::antiAliasingDictionary_ = Image::makeAntiAliasingDictionary();
Image::TMipMappingDictionary Image::mipMappingDictionary_ = Image::makeMipMappingDictionary();
Image::AntiAliasing Image::defaultAntiAliasing_ = Image::aaTrilinear;
Image::MipMapping Image::defaultMipMapping_ = Image::mmAnisotropic;

// --- public --------------------------------------------------------------------------------------

Image::Image(const std::filesystem::path& filename):
	antiAliasing_(defaultAntiAliasing_),
	mipMapping_(defaultMipMapping_),
	currentMipMapping_(mmUninitialized)
{
	loadFile(filename);
}



Image::Image(const std::filesystem::path& filename, const TRgbSpacePtr& rgbSpace):
	antiAliasing_(defaultAntiAliasing_),
	mipMapping_(defaultMipMapping_),
	currentMipMapping_(mmUninitialized)
{
	loadFile(filename, rgbSpace);
}



Image::Image(
		const std::filesystem::path& filename, const std::string& antiAliasing,
		const std::string& mipMapping):
	currentMipMapping_(mmUninitialized)
{
	loadFile(filename);
	setAntiAliasing(antiAliasing);
	setMipMapping(mipMapping);
}



Image::Image(
		const std::filesystem::path& filename, const std::string& antiAliasing,
		const std::string& mipMapping, const TRgbSpacePtr& rgbSpace):
	currentMipMapping_(mmUninitialized)
{
	loadFile(filename, rgbSpace);
	setAntiAliasing(antiAliasing);
	setMipMapping(mipMapping);
}



void Image::loadFile(const std::filesystem::path& filename)
{
	loadFile(filename, TRgbSpacePtr());
}



void Image::loadFile(const std::filesystem::path& filename, const TRgbSpacePtr& rgbSpace)
{
	util::Clock clock;
	util::StopWatch stopWatch(clock);
	stopWatch.start();
	LASS_CERR << "Loading image " << filename << "... " << std::flush;

	ImageReader reader(filename, rgbSpace, "");
	TResolution2D resolution = reader.resolution();
	if (resolution.x == 0 || resolution.y == 0)
	{
		LASS_THROW("bad image resolution");
	}
	filename_ = filename;
	resolution_ = resolution;
	rgbSpace_ = reader.rgbSpace();

	TPixels image(new TPixel[resolution.x * resolution.y]);
	for (size_t i = 0; i < resolution.y; ++i)
	{
		reader.readLine(&image[i * resolution.x]);
	}

	image_.swap(image);

	currentMipMapping_ = mmUninitialized;
	mipMaps_.clear();

	stopWatch.stop();
	LASS_CERR << stopWatch.time() << "s\n";
}



const std::filesystem::path& Image::filename() const
{
	return filename_;
}



const TResolution2D& Image::resolution() const
{
	return resolution_;
}



const std::string Image::antiAliasing() const
{
	return antiAliasingDictionary_.key(antiAliasing_);
}



const std::string Image::mipMapping() const
{
	return mipMappingDictionary_.key(mipMapping_);
}



void Image::setAntiAliasing(const std::string& mode)
{
	antiAliasing_ = antiAliasingDictionary_[stde::tolower(mode)];
}



void Image::setMipMapping(const std::string& mode)
{
	mipMapping_ = mipMappingDictionary_[stde::tolower(mode)];
}



void Image::setDefaultAntiAliasing(const std::string& mode)
{
	defaultAntiAliasing_ = antiAliasingDictionary_[stde::tolower(mode)];
}



void Image::setDefaultMipMapping(const std::string& mode)
{
	defaultMipMapping_ = mipMappingDictionary_[stde::tolower(mode)];
}


// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const Spectral Image::doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const
{
	return Spectral::fromXYZ(static_cast<XYZ>(lookUp(context)), sample, type);
}


Texture::TValue Image::doScalarLookUp(const Sample&, const IntersectionContext& context) const
{
	return lookUp(context).y;
}


bool Image::doIsChromatic() const
{
	return true;
}

const Image::TPixel Image::lookUp(const IntersectionContext& context) const
{
	if (currentMipMapping_.load(std::memory_order_acquire) != mipMapping_)
	{
		makeMipMaps();
	}

	const TPoint2D& uv = context.uv();
	const TVector2D& dUv_dI = context.dUv_dI();
	const TVector2D& dUv_dJ = context.dUv_dJ();

	size_t levelU0 = 0, levelU1 = 0, levelV0 = 0, levelV1 = 0;
	TPixel::TValue dLevelU = 0, dLevelV = 0;
	switch (mipMapping_)
	{
	case mmNone:
		break;
	case mmIsotropic:
		mipMapLevel(std::max(
				std::max(num::abs(dUv_dI.x), num::abs(dUv_dJ.x)),
				std::max(num::abs(dUv_dI.y), num::abs(dUv_dJ.y))),
			numLevelsU_, levelU0, levelU1, dLevelU);
		break;
	case mmAnisotropic:
		mipMapLevel(std::max(num::abs(dUv_dI.x), num::abs(dUv_dJ.x)),
			numLevelsU_, levelU0, levelU1, dLevelU);
		mipMapLevel(std::max(num::abs(dUv_dI.y), num::abs(dUv_dJ.y)),
			numLevelsV_, levelV0, levelV1, dLevelV);
		break;
	default:
		LASS_ASSERT_UNREACHABLE;
	}

	TPackedPixel result;
	switch (antiAliasing_)
	{
	case aaNone:
		result = nearest(levelU0, levelV0, uv);
		break;

	case aaBilinear:
		result = bilinear(levelU0, levelV0, uv);
		break;

	case aaTrilinear:
#if LIAR_HAVE_AVX
		{
			const __m128 du = _mm_set1_ps(dLevelU);
			const __m128 du1 = _mm_sub_ps(_mm_set1_ps(1.f), du);
			const __m128 p00 = bilinear(levelU0, levelV0, uv);
			const __m128 p10 = bilinear(levelU1, levelV0, uv);
			result = _mm_add_ps(_mm_mul_ps(du1, p00), _mm_mul_ps(du, p10));
			if (levelV0 != levelV1)
			{
				const __m128 dv = _mm_set1_ps(dLevelV);
				const __m128 dv1 = _mm_sub_ps(_mm_set1_ps(1.f), dv);
				const __m128 p01 = bilinear(levelU0, levelV1, uv);
				const __m128 p11 = bilinear(levelU1, levelV1, uv);
				const __m128 p1 = _mm_add_ps(_mm_mul_ps(du1, p01), _mm_mul_ps(du, p11));
				result = _mm_add_ps(_mm_mul_ps(dv1, result), _mm_mul_ps(dv, p1));
			}
		}
#else
		result =
			bilinear(levelU0, levelV0, uv) * (1 - dLevelU) +
			bilinear(levelU1, levelV0, uv) * dLevelU;
		if (levelV0 != levelV1)
		{
			result *= (1 - dLevelV);
			result += dLevelV * (
				bilinear(levelU0, levelV1, uv) * (1 - dLevelU) +
				bilinear(levelU1, levelV1, uv) * dLevelU);
		}
#endif
		break;

	default:
		LASS_ENFORCE_UNREACHABLE;
	}

#if LIAR_HAVE_AVX
	return std::bit_cast<TPixel>(result);
#else
	return result;
#endif
}


const TPyObjectPtr Image::doGetState() const
{
	return python::makeTuple(filename_, rgbSpace_, antiAliasing(), mipMapping());
}



void Image::doSetState(const TPyObjectPtr& state)
{
	std::filesystem::path filename;
	std::string antiAliasing, mipMapping;
	TRgbSpacePtr rgbSpace;
	python::decodeTuple(state, filename, rgbSpace, antiAliasing, mipMapping);

	loadFile(filename, rgbSpace);
	setAntiAliasing(antiAliasing);
	setMipMapping(mipMapping);
}



/** Make a grid of mip maps.
 *  In case of isotropic mip mapping, we'll only fill one row.
 *
 *  S. Guthe, P. Heckbert (2003): nVIDIA tech report: Non-Power-of-Two Mipmap Creation.
 *  http://developer.nvidia.com/object/np2_mipmapping.html
 */
void Image::makeMipMaps() const
{
	LASS_LOCK(mutex_)
	{
		if (currentMipMapping_.load(std::memory_order_relaxed) == mipMapping_)
		{
			return;
		}

		TMipMaps mipMaps(1, MipMapLevel(image_, resolution_));
		size_t numLevelsU = 0, numLevelsV = 0;

		switch (mipMapping_)
		{
		case mmNone:
			numLevelsU = 1;
			numLevelsV = 1;
			break;

		case mmIsotropic:
			numLevelsU = static_cast<size_t>(num::floor(num::log2(
				static_cast<TScalar>(std::max(resolution_.x, resolution_.y))))) + 1;
			numLevelsV = 1;
			for (size_t i = 1; i < numLevelsU; ++i)
			{
				const TScalar scale = num::inv(static_cast<TScalar>(1 << i));
				const size_t width = static_cast<size_t>(num::floor(scale * static_cast<TScalar>(resolution_.x)));
				const size_t height = static_cast<size_t>(num::floor(scale * static_cast<TScalar>(resolution_.y)));
				MipMapLevel temp = makeMipMap(mipMaps.back(), 'x', width);
				mipMaps.push_back(makeMipMap(temp, 'y', height));
			}
			break;

		case mmAnisotropic:
			numLevelsU = static_cast<size_t>(num::floor(num::log2(static_cast<TScalar>(resolution_.x))) + 1);
			numLevelsV = static_cast<size_t>(num::floor(num::log2(static_cast<TScalar>(resolution_.y))) + 1);
			for (size_t j = 0; j < numLevelsV; ++j)
			{
				if (j > 0)
				{
					const size_t height = static_cast<size_t>(
						num::floor(static_cast<TScalar>(resolution_.y) / (1 << j)));
					mipMaps.push_back(makeMipMap(mipMaps[(j - 1) * numLevelsU], 'y', height));
				}
				for (size_t i = 1; i < numLevelsU; ++i)
				{
					const size_t width = static_cast<size_t>(
						num::floor(static_cast<TScalar>(resolution_.x) / (1 << i)));
					mipMaps.push_back(makeMipMap(mipMaps.back(), 'x', width));
				}
			}
			break;

		default:
			LASS_ASSERT_UNREACHABLE;
		}

		mipMaps_.swap(mipMaps);
		numLevelsU_ = numLevelsU;
		numLevelsV_ = numLevelsV;

		currentMipMapping_.store(mipMapping_, std::memory_order_release);
	}
}




Image::MipMapLevel Image::makeMipMap(
		const MipMapLevel& parent, prim::XY compressionAxis, size_t newSize) const
{
	if (2 * newSize == parent.resolution()[compressionAxis])
	{
		return makeMipMapEven(parent, compressionAxis, newSize);
	}
	else
	{
		return makeMipMapOdd(parent, compressionAxis, newSize);
	}
}



Image::MipMapLevel Image::makeMipMapEven(
		const MipMapLevel& parent, prim::XY compressionAxis, size_t newSize) const
{
	TResolution2D resolution = parent.resolution();
	resolution[compressionAxis] = newSize;

	if (resolution.x == 0 || resolution.y == 0)
	{
		return parent;
	}

#if LIAR_HAVE_AVX
	const __m128 half = _mm_set1_ps(0.5f);
#endif
	MipMapLevel level(resolution);
	if (compressionAxis == 'x')
	{
		LASS_ASSERT(parent.resolution().x == 2 * resolution.x);
		for (size_t y = 0; y < resolution.y; ++y)
		{
			for (size_t x = 0; x < resolution.x; ++x)
			{
#if LIAR_HAVE_AVX
				const __m128 p0 = parent(2 * x, y);
				const __m128 p1 = parent(2 * x + 1, y);
				level(x, y) = _mm_mul_ps(_mm_add_ps(p0, p1), half);
#else
				level(x, y) = (parent(2 * x, y) + parent(2 * x + 1, y)) / 2;
#endif
			}
		}
	}
	else
	{
		LASS_ASSERT(parent.resolution().y == 2 * resolution.y);
		for (size_t y = 0; y < resolution.y; ++y)
		{
			for (size_t x = 0; x < resolution.x; ++x)
			{
#if LIAR_HAVE_AVX
				const __m128 p0 = parent(x, 2 * y);
				const __m128 p1 = parent(x, 2 * y + 1);
				level(x, y) = _mm_mul_ps(_mm_add_ps(p0, p1), half);
#else
				level(x, y) = (parent(x, 2 * y) + parent(x, 2 * y + 1)) / 2;
#endif
			}
		}
	}

	return level;
}

Image::MipMapLevel Image::makeMipMapOdd(
		const MipMapLevel& parent, prim::XY compressionAxis, size_t newSize) const
{
	TResolution2D resolution = parent.resolution();
	const size_t oldSize = resolution[compressionAxis];
	LASS_ASSERT(2 * newSize + 1 == oldSize);
	resolution[compressionAxis] = newSize;

	if (resolution.x == 0 || resolution.y == 0)
	{
		return parent;
	}

	MipMapLevel level(resolution);

	const TValue scale = num::inv(static_cast<TValue>(oldSize));
	const bool compressX = compressionAxis == 'x';
	for (size_t k = 0; k < newSize; ++k)
	{
		const size_t k0 = 2 * k;
		const size_t k1 = k0 + 1;
		const size_t k2 = k0 + 2;

		const TValue w0 = scale * static_cast<TValue>(newSize - k);
		const TValue w1 = scale * static_cast<TValue>(newSize);
		const TValue w2 = scale * static_cast<TValue>(k + 1);
#if LIAR_HAVE_AVX
		const __m128 pw0 = _mm_set1_ps(w0);
		const __m128 pw1 = _mm_set1_ps(w1);
		const __m128 pw2 = _mm_set1_ps(w2);
#endif

		if (compressX)
		{
			for (size_t y = 0; y < resolution.y; ++y)
			{
				const auto p0 = parent(k0, y);
				const auto p1 = parent(k1, y);
				const auto p2 = parent(k2, y);
#if LIAR_HAVE_AVX
				level(k, y) = _mm_add_ps(
					_mm_add_ps(_mm_mul_ps(pw0, p0), _mm_mul_ps(p1, p1)),
					_mm_mul_ps(pw2, p2));
#else
				level(k, y) = w0 * p0 + w1 * p1 + w2 * p2;
#endif
			}
		}
		else
		{
			for (size_t x = 0; x < resolution.x; ++x)
			{
				const auto p0 = parent(x, k0);
				const auto p1 = parent(x, k1);
				const auto p2 = parent(x, k2);
#if LIAR_HAVE_AVX
				level(x, k) = _mm_add_ps(
					_mm_add_ps(_mm_mul_ps(pw0, p0), _mm_mul_ps(pw1, p1)),
					_mm_mul_ps(pw2, p2));
#else
				level(x, k) = w0 * p0 + w1 * p1 + w2 * p2;
#endif
			}
		}
	}

	return level;
}



void Image::mipMapLevel(
	TScalar width, size_t numLevels, size_t& level0, size_t& level1, TPixel::TValue& dLevel) const
{
	const TScalar maxLevel = static_cast<TScalar>(numLevels - 1);
	const TScalar level = maxLevel + num::log2(std::max(width, TScalar(1e-9f)));
	if (level < TNumTraits::one)
	{
		level0 = level1 = 0;
		dLevel = 0;
	}
	else if (level >= maxLevel)
	{
		level0 = level1 = static_cast<size_t>(maxLevel);
		dLevel = 0;
	}
	else
	{
		const TScalar floorLevel = num::floor(level);
		level0 = static_cast<size_t>(floorLevel);
		level1 = level0 + 1;
		dLevel = static_cast<TPixel::TValue>(level - floorLevel);
	}
}



Image::TPackedPixel
Image::nearest(size_t levelU, size_t levelV, const TPoint2D& uv) const
{
	LASS_ASSERT(levelU < numLevelsU_ && levelV < numLevelsV_);
	const MipMapLevel& mipMap = mipMaps_[levelV * numLevelsU_ + levelU];

	const TResolution2D& res = mipMap.resolution();
	const TScalar x = num::fractional(uv.x) * static_cast<TScalar>(res.x);
	const TScalar y = num::fractional(1.f - uv.y) * static_cast<TScalar>(res.y);
	const size_t x0 = static_cast<size_t>(num::floor(x));
	const size_t y0 = static_cast<size_t>(num::floor(y));

	return mipMap(x0, y0);
}



Image::TPackedPixel
Image::bilinear(size_t levelU, size_t levelV, const TPoint2D& uv) const
{
	LASS_ASSERT(levelU < numLevelsU_ && levelV < numLevelsV_);
	const MipMapLevel& mipMap = mipMaps_[levelV * numLevelsU_ + levelU];

#if LIAR_HAVE_AVX
	const __m128 one = _mm_set1_ps(1.f);

	// duv = (u, 1 - v), goes from (0, 0) to (1, 1)
	const __m128d uv_ = _mm_set_pd(1 - uv.y, uv.x);
	const __m128d uv0 = _mm_floor_pd(uv_);
	const __m128 duv = _mm_cvtpd_ps(_mm_sub_pd(uv_, uv0));

	// xy goes from (0, 0) to (res.x, res.y)
	const int resX = static_cast<int>(mipMap.resolution().x);
	const int resY = static_cast<int>(mipMap.resolution().y);
	const __m128i res = _mm_set_epi32(0, 0, resY, resX);
	const __m128 resf = _mm_cvtepi32_ps(res);
	const __m128 xy = _mm_sub_ps(_mm_mul_ps(duv, resf), _mm_set1_ps(0.5f));
	const __m128 xy0 = _mm_floor_ps(xy);
	const __m128 dxy = _mm_sub_ps(xy, xy0);

	const __m128i ij0 = _mm_cvtps_epi32(xy0);
	const __m128i ij0mask = _mm_cmplt_epi32(ij0, _mm_setzero_si128());
	const __m128i ij0mod = _mm_add_epi32(ij0, _mm_and_si128(res, ij0mask));
	static_assert(sizeof(ij0mod) == sizeof(int) * 4);
	const auto ij0int = std::bit_cast<std::array<int, 4>>(ij0mod);

	const __m128i ij1 = _mm_add_epi32(ij0, _mm_set1_epi32(1));
	const __m128i ij1mask = _mm_cmplt_epi32(ij1, res);
	const __m128i ij1mod = _mm_and_si128(ij1, ij1mask);
	static_assert(sizeof(ij1mod) == sizeof(int) * 4);
	const auto ij1int = std::bit_cast<std::array<int, 4>>(ij1mod);

	const int i0 = ij0int[0];
	const int j0 = ij0int[1];
	const int i1 = ij1int[0];
	const int j1 = ij1int[1];
	LIAR_ASSERT(i0 >= 0 && i0 < resX, "i0 = " << i0 << ", res.x = " << resX);
	LIAR_ASSERT(j0 >= 0 && j0 < resY, "j0 = " << j0 << ", res.y = " << resY);
	LIAR_ASSERT(i1 >= 0 && i1 < resX, "i1 = " << i1 << ", res.x = " << resX);
	LIAR_ASSERT(j1 >= 0 && j1 < resY, "j1 = " << j1 << ", res.y = " << resY);

	const __m128 dx = _mm_shuffle_ps(dxy, dxy, _MM_SHUFFLE(0, 0, 0, 0));
	const __m128 dx1 = _mm_sub_ps(one, dx);
	const __m128 p00 = mipMap(i0, j0);
	const __m128 p10 = mipMap(i1, j0);
	const __m128 p0 = _mm_add_ps(_mm_mul_ps(dx1, p00), _mm_mul_ps(dx, p10));
	const __m128 p01 = mipMap(i0, j1);
	const __m128 p11 = mipMap(i1, j1);
	const __m128 p1 = _mm_add_ps(_mm_mul_ps(dx1, p01), _mm_mul_ps(dx, p11));

	const __m128 dy = _mm_shuffle_ps(dxy, dxy, _MM_SHUFFLE(1, 1, 1, 1));
	const __m128 dy1 = _mm_sub_ps(one, dy);
	return _mm_add_ps(_mm_mul_ps(dy1, p0), _mm_mul_ps(dy, p1));
#else
	const TResolution2D& res = mipMap.resolution();
	const TScalar nx = static_cast<TScalar>(res.x);
	const TScalar ny = static_cast<TScalar>(res.y);
	const TScalar x = num::mod(uv.x * nx - .5f, nx);
	const TScalar y = num::mod((1 - uv.y) * ny - .5f, ny);
	const TScalar x0 = num::floor(x);
	const TScalar y0 = num::floor(y);
	const TValue dx = static_cast<TValue>(x - x0);
	const TValue dy = static_cast<TValue>(y - y0);

	const size_t i0 = static_cast<size_t>(x0) % res.x;
	const size_t j0 = static_cast<size_t>(y0) % res.y;
	const size_t i1 = (i0 + 1) % res.x;
	const size_t j1 = (j0 + 1) % res.y;

	return
		(mipMap(i0, j0) * (1 - dx) + mipMap(i1, j0) * dx) * (1 - dy) +
		(mipMap(i0, j1) * (1 - dx) + mipMap(i1, j1) * dx) * dy;
#endif
}



Image::TAntiAliasingDictionary Image::makeAntiAliasingDictionary()
{
	TAntiAliasingDictionary result;
	result.enableSuggestions();
	result.add("none", aaNone);
	result.add("bilinear", aaBilinear);
	result.add("trilinear", aaTrilinear);
	return result;
}



Image::TMipMappingDictionary Image::makeMipMappingDictionary()
{
	TMipMappingDictionary result;
	result.enableSuggestions();
	result.add("none", mmNone);
	result.add("isotropic", mmIsotropic);
	result.add("anisotropic", mmAnisotropic);
	return result;
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
