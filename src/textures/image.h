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

/** @class liar::textures::Image
 *  @brief texture using image file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_IMAGE_H
#define LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_IMAGE_H

#include "textures_common.h"
#include "../kernel/texture.h"
#include "../kernel/rgb_space.h"
#include "../kernel/xyza.h"
#include <lass/prim/xy.h>
#include <lass/util/dictionary.h>
#include <lass/util/thread.h>
#include <filesystem>

#if LIAR_HAVE_AVX
#	include <immintrin.h>
#	include <bit>
#endif

namespace liar
{
namespace textures
{

class LIAR_TEXTURES_DLL Image: public Texture
{
	PY_HEADER(Texture)
public:

	typedef kernel::XYZA TPixel;

	enum class AntiAliasing
	{
		none = 0,
		bilinear,
		trilinear,
	};

	enum class MipMapping
	{
		none = 0,
		isotropic,
		anisotropic,
	};

	explicit Image(const std::filesystem::path& filename);
	Image(const std::filesystem::path& filename, const TRgbSpacePtr& rgbSpace);
	Image(const std::filesystem::path& filename, AntiAliasing antiAliasing,
		MipMapping mipMapping);
	Image(const std::filesystem::path& filename, AntiAliasing antiAliasing,
		MipMapping mipMapping, const TRgbSpacePtr& rgbSpace);

	void loadFile(const std::filesystem::path& filename);
	void loadFile(const std::filesystem::path& filename, const TRgbSpacePtr& rgbSpace);

	const std::filesystem::path& filename() const;
	const TResolution2D& resolution() const;

	AntiAliasing antiAliasing() const;
	MipMapping mipMapping() const;
	void setAntiAliasing(AntiAliasing antiAliasing);
	void setMipMapping(MipMapping mipMapping);

	const TPixel lookUp(const IntersectionContext& context) const;

	static void setDefaultAntiAliasing(AntiAliasing antiAliasing);
	static void setDefaultMipMapping(MipMapping mipMapping);

private:

	typedef util::SharedPtr<TPixel, util::ArrayStorage> TPixels;

#if LIAR_HAVE_AVX
	using TPackedPixel = __m128;
#else
	using TPackedPixel = TPixel;
#endif

	class MipMapLevel
	{
	public:

		static_assert(sizeof(TPackedPixel) == sizeof(TPixel));
		static_assert(std::is_same_v<TPixel::TValue, float>);

		MipMapLevel(const TPixels& pixels, const TResolution2D& resolution):
			MipMapLevel(resolution)
		{
			for (size_t y = 0; y < resolution.y; ++y)
			{
				for (size_t x = 0; x < resolution.x; ++x)
				{
#if LIAR_HAVE_AVX
					(*this)(x, y) = std::bit_cast<TPackedPixel>(pixels[y * resolution.x + x]);
#else
					(*this)(x, y) = pixels[y * resolution.x + x];
#endif
				}
			}
		}
		MipMapLevel(const TResolution2D& resolution) :
			numBlocksX_(block(roundUp(resolution.x))),
			numBlocksY_(block(roundUp(resolution.y))),
			resolution_(resolution.x, resolution.y)
		{
			blocks_.resize(numBlocksX_ * numBlocksY_);
		}
		TPackedPixel& operator()(size_t x, size_t y)
		{
			return blocks_[block(x, y)].pixels[blockOffset(y)][blockOffset(x)];
		}
		const TPackedPixel& operator()(size_t x, size_t y) const
		{
			return blocks_[block(x, y)].pixels[blockOffset(y)][blockOffset(x)];
		}
		const TResolution2D& resolution() const { return resolution_; }
	private:
		constexpr static size_t logBlockSize = 2;
		constexpr static size_t blockSize = 1 << logBlockSize;
#ifdef __cpp_lib_hardware_interference_size
		constexpr static size_t blockAlignment = std::hardware_constructive_interference_size;
#else
		constexpr static size_t blockAlignment = 64;
#endif

		struct alignas(blockAlignment) Block
		{
			TPackedPixel pixels[blockSize][blockSize];
		};
		static_assert(alignof(Block) % alignof(TPackedPixel) == 0, "Assume a whole number of TPackedPixels fit on one cacheline");

		using TBlocks = std::vector<Block>;

		LIAR_FORCE_INLINE static size_t block(size_t i)
		{
			return i >> logBlockSize;
		}
		LIAR_FORCE_INLINE size_t block(size_t x, size_t y) const
		{
			return block(x) * numBlocksY_ + block(y); // column major blocks, with row major pixels in blocks
		}
		LIAR_FORCE_INLINE static size_t blockOffset(size_t i)
		{
			return i & (blockSize - 1);
		}
		static size_t roundUp(size_t size)
		{
			return (size + blockSize - 1) & ~(blockSize - 1);
		}
		size_t address(size_t x, size_t y) const
		{
			const size_t bx = block(x);
			const size_t by = block(y);
			const size_t ox = blockOffset(x);
			const size_t oy = blockOffset(y);
			return (by * numBlocksX_ + bx) * blockSize * blockSize + oy * blockSize + ox;
		}

		TBlocks blocks_;
		size_t numBlocksX_;
		size_t numBlocksY_;
		TResolution2D resolution_;
	};
	typedef std::vector<MipMapLevel> TMipMaps;

	const Spectral doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const override;
	TValue doScalarLookUp(const Sample& sample, const IntersectionContext& context) const override;
	bool doIsChromatic() const override;

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

	void makeMipMaps() const;
	MipMapLevel makeMipMap(const MipMapLevel& parent, prim::XY compressionAxis, size_t newSize) const;
	MipMapLevel makeMipMapEven(const MipMapLevel& parent, prim::XY compressionAxis, size_t newSize) const;
	MipMapLevel makeMipMapOdd(const MipMapLevel& parent, prim::XY compressionAxis, size_t newSize) const;

	void mipMapLevel(TScalar width, size_t numLevels,
		size_t& level0, size_t& level1, TPixel::TValue& dLevel) const;

	TPackedPixel nearest(size_t levelU, size_t levelV, const TPoint2D& uv) const;
	TPackedPixel bilinear(size_t levelU, size_t levelV, const TPoint2D& uv) const;

	std::filesystem::path filename_;
	TRgbSpacePtr rgbSpace_;
	TPixels image_;
	TResolution2D resolution_;
	AntiAliasing antiAliasing_;
	MipMapping mipMapping_;
	mutable std::atomic<MipMapping> currentMipMapping_;
	mutable TMipMaps mipMaps_;
	mutable size_t numLevelsU_;
	mutable size_t numLevelsV_;
	mutable util::Semaphore mutex_;

	static AntiAliasing defaultAntiAliasing_;
	static MipMapping defaultMipMapping_;
};

typedef python::PyObjectPtr<Image>::Type TImagePtr;

}

}

PY_SHADOW_STR_ENUM(LASS_DLL_EXPORT, liar::textures::Image::AntiAliasing)
PY_SHADOW_STR_ENUM(LASS_DLL_EXPORT, liar::textures::Image::MipMapping)

#endif

// EOF
