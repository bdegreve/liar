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

#include "textures_common.h"
#include "image.h"
#include "../kernel/image_codec.h"
#include <lass/io/file_attribute.h>
#include <lass/stde/extended_string.h>
#include <lass/python/export_traits_filesystem.h>

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
	std::vector<prim::ColorRGBA> line(resolution.x);
	TPixel* pixel = &image[0];
	for (size_t i = 0; i < resolution.y; ++i)
	{
		reader.readLine(&line[0]);
		for (size_t k = 0; k < resolution.x; ++k)
		{
			*pixel++ = rgbSpace_->convert(line[k]);
		}
	}

	image_.swap(image);

	currentMipMapping_ = mmUninitialized;
	mipMaps_.clear();
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
	return Spectral::fromXYZ(lookUp(context), sample, type);
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

	TPixel result;
	switch (antiAliasing_)
	{
	case aaNone:
		result = nearest(levelU0, levelV0, uv);
		break;

	case aaBilinear:
		result = bilinear(levelU0, levelV0, uv);
		break;

	case aaTrilinear:
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
		break;

	default:
		LASS_ASSERT_UNREACHABLE;
	}

	return result;
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

	MipMapLevel level(resolution);
	if (compressionAxis == 'x')
	{
		LASS_ASSERT(parent.resolution().x == 2 * resolution.x);
		for (size_t y = 0; y < resolution.y; ++y)
		{
			for (size_t x = 0; x < resolution.x; ++x)
			{
				level(x, y) = (parent(2 * x, y) + parent(2 * x + 1, y)) / 2;
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
				level(x, y) = (parent(x, 2 * y) + parent(x, 2 * y + 1)) / 2;
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

		if (compressX)
		{
			for (size_t y = 0; y < resolution.y; ++y)
			{
				level(k, y) = w0 * parent(k0, y) + w1 * parent(k1, y) + w2 * parent(k2, y);
			}
		}
		else
		{
			for (size_t x = 0; x < resolution.x; ++x)
			{
				level(x, k) = w0 * parent(x, k0) + w1 * parent(x, k1) + w2 * parent(x, k2);
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




const Image::TPixel Image::nearest(size_t levelU, size_t levelV, const TPoint2D& uv) const
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




const Image::TPixel Image::bilinear(size_t levelU, size_t levelV, const TPoint2D& uv) const
{
	LASS_ASSERT(levelU < numLevelsU_ && levelV < numLevelsV_);
	const MipMapLevel& mipMap = mipMaps_[levelV * numLevelsU_ + levelU];

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
