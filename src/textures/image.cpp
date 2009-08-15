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

#include "textures_common.h"
#include "image.h"
#include "../kernel/image_codec.h"
#include <lass/io/file_attribute.h>
#include <lass/stde/extended_string.h>

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(Image, "image file")
PY_CLASS_CONSTRUCTOR_1(Image, const std::string&);
PY_CLASS_CONSTRUCTOR_2(Image, const std::string&, const TRgbSpacePtr&);
PY_CLASS_CONSTRUCTOR_3(Image, const std::string&, const std::string&, const std::string&);
PY_CLASS_CONSTRUCTOR_4(Image, const std::string&, const std::string&, const std::string&, const TRgbSpacePtr&);
PY_CLASS_METHOD_QUALIFIED_1(Image, loadFile, void, const std::string&)
PY_CLASS_METHOD_QUALIFIED_2(Image, loadFile, void, const std::string&, const TRgbSpacePtr&)
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

Image::Image(const std::string& filename):
	antiAliasing_(defaultAntiAliasing_),
	mipMapping_(defaultMipMapping_),
	currentMipMapping_(mmUninitialized)
{
	loadFile(filename);
}



Image::Image(const std::string& filename, const TRgbSpacePtr& rgbSpace):
	antiAliasing_(defaultAntiAliasing_),
	mipMapping_(defaultMipMapping_),
	currentMipMapping_(mmUninitialized)
{
	loadFile(filename, rgbSpace);
}



Image::Image(
		const std::string& filename, const std::string& antiAliasing, 
		const std::string& mipMapping):
	currentMipMapping_(mmUninitialized)
{
	loadFile(filename);
	setAntiAliasing(antiAliasing);
	setMipMapping(mipMapping);
}



Image::Image(
		const std::string& filename, const std::string& antiAliasing, 
		const std::string& mipMapping, const TRgbSpacePtr& rgbSpace):
	currentMipMapping_(mmUninitialized)
{
	loadFile(filename, rgbSpace);
	setAntiAliasing(antiAliasing);
	setMipMapping(mipMapping);
}



void Image::loadFile(const std::string& filename)
{
	loadFile(filename, TRgbSpacePtr());
}



void Image::loadFile(const std::string& filename, const TRgbSpacePtr& rgbSpace)
{
	ImageReader reader(filename, rgbSpace, "");
	TResolution2D resolution = reader.resolution();
	TPixels image(new TPixel[resolution.x * resolution.y]);
	reader.read(TResolution2D(), resolution, image.get(), 0);

	filename_ = filename;
	rgbSpace_ = rgbSpace;
	image_.swap(image);
	resolution_ = resolution;
	currentMipMapping_ = mmUninitialized;
	mipMaps_.clear();
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

const XYZ Image::doLookUp(const Sample& sample, const IntersectionContext& context) const
{
	if (mipMapping_ != currentMipMapping_)
	{
		makeMipMaps(mipMapping_);
	}

	const TPoint2D& uv = context.uv();
	const TVector2D& dUv_dI = context.dUv_dI();
	const TVector2D& dUv_dJ = context.dUv_dJ();

	unsigned levelU0 = 0, levelU1 = 0, levelV0 = 0, levelV1 = 0;
	TScalar dLevelU = TNumTraits::zero, dLevelV = TNumTraits::zero;
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
			bilinear(levelU0, levelV0, uv) * (TNumTraits::one - dLevelU) +
			bilinear(levelU1, levelV0, uv) * dLevelU;
		if (levelV0 != levelV1)
		{
			result *= (TNumTraits::one - dLevelV);
			result += dLevelV * (
				bilinear(levelU0, levelV1, uv) * (TNumTraits::one - dLevelU) +
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
	std::string filename, antiAliasing, mipMapping;
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
void Image::makeMipMaps(MipMapping mode) const
{
	LASS_LOCK(mutex_)
	{
		if (mipMapping_ == currentMipMapping_)
		{
			return;
		}

		TMipMaps mipMaps(1, MipMapLevel(image_, resolution_));
		unsigned numLevelsU = 0, numLevelsV = 0;

		switch (mode)
		{
		case mmNone:
			numLevelsU = 1;
			numLevelsV = 1;
			break;

		case mmIsotropic:
			numLevelsU = static_cast<unsigned>(num::floor(num::log2(
				static_cast<TScalar>(std::max(resolution_.x, resolution_.y))))) + 1;
			numLevelsV = 1;
			for (unsigned i = 1; i < numLevelsU; ++i)
			{
				const TScalar scale = TNumTraits::one / (1 << i);
				const unsigned width = static_cast<unsigned>(num::floor(scale * resolution_.x));
				const unsigned height = static_cast<unsigned>(num::floor(scale * resolution_.x));
				MipMapLevel temp = makeMipMap(mipMaps.back(), 'x', width);
				mipMaps.push_back(makeMipMap(temp, 'y', height));
			}
			break;

		case mmAnisotropic:
			numLevelsU = static_cast<unsigned>(num::floor(num::log2(static_cast<TScalar>(resolution_.x))) + 1);
			numLevelsV = static_cast<unsigned>(num::floor(num::log2(static_cast<TScalar>(resolution_.y))) + 1);
			for (unsigned j = 0; j < numLevelsV; ++j)
			{
				if (j > 0)
				{
					const unsigned height = static_cast<unsigned>(
						num::floor(static_cast<TScalar>(resolution_.y) / (1 << j)));
					mipMaps.push_back(makeMipMap(mipMaps[(j - 1) * numLevelsU], 'y', height));
				}
				for (unsigned i = 1; i < numLevelsU; ++i)
				{
					const unsigned width = static_cast<unsigned>(
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
		currentMipMapping_ = mode;
	}
}




Image::MipMapLevel Image::makeMipMap(
		const MipMapLevel& parent, prim::XY compressionAxis, unsigned newSize) const
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
		const MipMapLevel& parent, prim::XY compressionAxis, unsigned newSize) const
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
		for (unsigned y = 0; y < resolution.y; ++y)
		{
			for (unsigned x = 0; x < resolution.x; ++x)
			{
				level(x, y) = (parent(2 * x, y) + parent(2 * x + 1, y)) / 2;
			}
		}
	}
	else
	{
		LASS_ASSERT(parent.resolution().y == 2 * resolution.y);
		for (unsigned y = 0; y < resolution.y; ++y)
		{
			for (unsigned x = 0; x < resolution.x; ++x)
			{
				level(x, y) = (parent(x, 2 * y) + parent(x, 2 * y + 1)) / 2;
			}
		}
	}

	return level;
}

Image::MipMapLevel Image::makeMipMapOdd(
		const MipMapLevel& parent, prim::XY compressionAxis, unsigned newSize) const
{
	TResolution2D resolution = parent.resolution();
	const unsigned oldSize = resolution[compressionAxis];
	LASS_ASSERT(2 * newSize + 1 == oldSize);
	resolution[compressionAxis] = newSize;

	if (resolution.x == 0 || resolution.y == 0)
	{
		return parent;
	}

	MipMapLevel level(resolution);

	const TScalar scale = num::inv(static_cast<TScalar>(oldSize));
	const bool compressX = compressionAxis == 'x';
	for (unsigned k = 0; k < newSize; ++k)
	{
		const unsigned k0 = 2 * k;
		const unsigned k1 = k0 + 1;
		const unsigned k2 = k0 + 2;
		const TScalar w0 = scale * (newSize - k);
		const TScalar w1 = scale * newSize; 
		const TScalar w2 = scale * (k + 1);

		if (compressX)
		{
			for (unsigned y = 0; y < resolution.y; ++y)
			{
				level(k, y) = w0 * parent(k0, y) + w1 * parent(k1, y) + w2 * parent(k2, y);
			}
		}
		else
		{
			for (unsigned x = 0; x < resolution.x; ++x)
			{
				level(x, k) = w0 * parent(x, k0) + w1 * parent(x, k1) + w2 * parent(x, k2);
			}
		}			
	}

	return level;
}



void Image::mipMapLevel(
		TScalar width, unsigned numLevels, unsigned& level0, unsigned& level1, TScalar& dLevel) const
{
	const TScalar maxLevel = static_cast<TScalar>(numLevels - 1);
	const TScalar level = maxLevel + num::log2(std::max(width, TScalar(1e-9f)));
	if (level < TNumTraits::one)
	{
		level0 = level1 = 0;
		dLevel = TNumTraits::zero;
	}
	else if (level >= maxLevel)
	{
		level0 = level1 = static_cast<unsigned>(maxLevel);
		dLevel = TNumTraits::zero;
	}
	else
	{
		const TScalar floorLevel = num::floor(level);
		level0 = static_cast<unsigned>(floorLevel);
		level1 = level0 + 1;
		dLevel = level - floorLevel;
	}
}




const Image::TPixel Image::nearest(unsigned levelU, unsigned levelV, const TPoint2D& uv) const
{
	LASS_ASSERT(levelU < numLevelsU_ && levelV < numLevelsV_);
	const MipMapLevel& mipMap = mipMaps_[levelV * numLevelsU_ + levelU];
	
	const TResolution2D& res = mipMap.resolution();
	const TScalar x = num::fractional(uv.x) * res.x;
	const TScalar y = num::fractional(1.f - uv.y) * res.y;
	const unsigned x0 = static_cast<unsigned>(num::floor(x));
	const unsigned y0 = static_cast<unsigned>(num::floor(y));
	
	return mipMap(x0, y0);
}




const Image::TPixel Image::bilinear(unsigned levelU, unsigned levelV, const TPoint2D& uv) const
{
	LASS_ASSERT(levelU < numLevelsU_ && levelV < numLevelsV_);
	const MipMapLevel& mipMap = mipMaps_[levelV * numLevelsU_ + levelU];

	const TResolution2D& res = mipMap.resolution();
	const TScalar x = num::mod(uv.x * res.x - .5f, static_cast<TScalar>(res.x));
	const TScalar y = num::mod((1 - uv.y) * res.y - .5f, static_cast<TScalar>(res.y));
	const TScalar x0 = num::floor(x);
	const TScalar y0 = num::floor(y);
	const TScalar dx = x - x0;
	const TScalar dy = y - y0;

	const unsigned i0 = static_cast<unsigned>(x0) % res.x;
	const unsigned j0 = static_cast<unsigned>(y0) % res.y;
	const unsigned i1 = (i0 + 1) % res.x;
	const unsigned j1 = (j0 + 1) % res.y;

	return
		(mipMap(i0, j0) * (TNumTraits::one - dx) + mipMap(i1, j0) * dx) * (TNumTraits::one - dy) +
		(mipMap(i0, j1) * (TNumTraits::one - dx) + mipMap(i1, j1) * dx) * dy;
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

