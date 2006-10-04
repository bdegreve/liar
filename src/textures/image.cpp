/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2006  Bram de Greve
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  http://liar.sourceforge.net
 */

#include "textures_common.h"
#include "image.h"
#include <lass/stde/extended_string.h>

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS(Image)
PY_CLASS_CONSTRUCTOR_1(Image, const std::string&);
PY_CLASS_CONSTRUCTOR_3(Image, const std::string&, const std::string&, const std::string&);
PY_CLASS_MEMBER_RW(Image, "antiAliasing", antiAliasing, setAntiAliasing);
PY_CLASS_MEMBER_RW(Image, "mipMapping", mipMapping, setMipMapping);
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
	image_ = TImagePtr(new io::Image(filename));
}



Image::Image(const std::string& filename, 
			 const std::string& antiAliasing, 
			 const std::string& mipMapping):
	currentMipMapping_(mmUninitialized)
{
	setAntiAliasing(antiAliasing);
	setMipMapping(mipMapping);
	image_ = TImagePtr(new io::Image(filename));
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

const Spectrum Image::doLookUp(const Sample& sample, const IntersectionContext& context) const
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

	io::Image::TPixel result;
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

	return Spectrum(TVector3D(result.r, result.g, result.b));
}



const TPyObjectPtr Image::doGetState() const
{
	LASS_THROW("not implemented yet");
	return TPyObjectPtr();
}



void Image::doSetState(const TPyObjectPtr& state)
{
	LASS_THROW("not implemented yet");
}



/** Make a grid of mip maps.
 *  In case of isotropic mip mapping, we'll only fill one row.
 */
void Image::makeMipMaps(MipMapping mode) const
{
	LASS_LOCK(mutex_)
	{
		if (mipMapping_ == currentMipMapping_)
		{
			return;
		}

		TMipMaps mipMaps;
		mipMaps.push_back(THorizontalMipMaps(1, image_));

		switch (mode)
		{
		case mmNone:
			break;

		case mmIsotropic:
			{
				THorizontalMipMaps& level = mipMaps.front(); 
				while (level.back()->rows() > 1 || level.back()->cols() > 1)
				{
					TImagePtr temp = makeMipMap(level.back(), true);
					level.push_back(makeMipMap(temp, false));
				}
			}
			break;

		case mmAnisotropic:
			{	
				// expand vertically first
				//
				while (mipMaps.back().front()->rows() > 1)
				{
					THorizontalMipMaps newLevel;
					newLevel.push_back(makeMipMap(mipMaps.back().front(), true));
					mipMaps.push_back(newLevel);
				}

				// then expand each level horizontally
				//
				for (TMipMaps::iterator level = mipMaps.begin(); level != mipMaps.end(); ++level)
				{
					LASS_ASSERT(!level->empty());
					while (level->back()->cols() > 1)
					{
						level->push_back(makeMipMap(level->back(), false));
					}
					LASS_ASSERT(level->size() == mipMaps.front().size());
				}
			}
			break;

		default:
			LASS_ASSERT_UNREACHABLE;
		}

		mipMaps_.swap(mipMaps);
		currentMipMapping_ = mode;
		numLevelsU_ = mipMaps_.front().size();
		numLevelsV_ = mipMaps_.size();
	}
}




Image::TImagePtr Image::makeMipMap(const TImagePtr iOldImagePtr, prim::XY iCompressionAxis) const
{
	const unsigned oldSize = (iCompressionAxis == 'x') ? iOldImagePtr->cols() : iOldImagePtr->rows();
	if ((oldSize & 0x1) == 0)
	{
		return makeMipMapEven(iOldImagePtr, iCompressionAxis);
	}
	else
	{
		return makeMipMapOdd(iOldImagePtr, iCompressionAxis);
	}
}



Image::TImagePtr Image::makeMipMapEven(const TImagePtr iOldImagePtr, prim::XY iCompressionAxis) const
{
	const io::Image& oldImage = *iOldImagePtr;

	const unsigned newCols = oldImage.cols() >> ((iCompressionAxis == 'x') ? 1 : 0);
	const unsigned newRows = oldImage.rows() >> ((iCompressionAxis == 'y') ? 1 : 0);

	if (newRows == 0 || newCols == 0)
	{
		return iOldImagePtr;
	}

	TImagePtr newImagePtr(new io::Image(newRows, newCols));
	io::Image& newImage = *newImagePtr;

	if (iCompressionAxis == 'y')
	{
		for (unsigned i = 0; i < newRows; ++i)
		{
			for (unsigned j = 0; j < newCols; ++j)
			{
				const unsigned i0 = 2 * i;
				newImage(i, j) = .5f * (oldImage(i0, j) + oldImage(i0 + 1, j));
			}
		}
	}
	else
	{
		for (unsigned i = 0; i < newRows; ++i)
		{
			for (unsigned j = 0; j < newCols; ++j)
			{
				const unsigned j0 = 2 * j;
				newImage(i, j) = .5f * (oldImage(i, j0) + oldImage(i, j0 + 1));
			}
		}
	}

	return newImagePtr;
}

Image::TImagePtr Image::makeMipMapOdd(const TImagePtr iOldImagePtr, prim::XY iCompressionAxis) const
{
	const io::Image& oldImage = *iOldImagePtr;
	const bool compressY = iCompressionAxis == 'y';

	const unsigned oldRows = oldImage.rows();
	const unsigned oldCols = oldImage.cols();
	const unsigned oldN = compressY ? oldRows : oldCols;
	const unsigned newN = oldN / 2 + 1; // rounding up
	LASS_ASSERT(2 * newN - 1 == oldN);
	const unsigned newRows = compressY ? newN : oldRows;
	const unsigned newCols = compressY ? oldCols : newN;

	if (newRows == 0 || newCols == 0)
	{
		return iOldImagePtr;
	}

	TImagePtr newImagePtr(new io::Image(newRows, newCols));
	io::Image& newImage = *newImagePtr;

	const TScalar invOldN = num::inv(static_cast<TScalar>(oldN));
	for (unsigned i = 0; i < newN; ++i)
	{
		const unsigned i1 = 2 * i;
		const unsigned i0 = i1 == 0 ? i1 : (i1 - 1);
		const unsigned i2 = i1 == (oldN - 1) ? (oldN - 1) : (i1 + 1);
		const TScalar w0 = i * invOldN;
		const TScalar w1 = newN * invOldN; 
		const TScalar w2 = (newN - i - 1) * invOldN;

		if (compressY)
		{
			for (unsigned j = 0; j < newCols; ++j)
			{
				newImage(i, j) = w0 * oldImage(i0, j) + w1 * oldImage(i1, j) + w2 * oldImage(i2, j);
			}
		}
		else
		{
			for (unsigned j = 0; j < newRows; ++j)
			{
				newImage(j, i) = w0 * oldImage(j, i0) + w1 * oldImage(j, i1) + w2 * oldImage(j, i2);
			}
		}		
	}

	return newImagePtr;
}



void Image::mipMapLevel(TScalar iWidth, size_t iNumLevels, 
						size_t& oLevel0, size_t& oLevel1, TScalar& oDLevel) const
{
	const TScalar maxLevel = static_cast<TScalar>(iNumLevels - 1);
	const TScalar level = maxLevel + num::log(std::max(iWidth, 1e-8f)) / num::log(TScalar(2));
	if (level < TNumTraits::one)
	{
		oLevel0 = oLevel1 = 0;
		oDLevel = TNumTraits::zero;
	}
	else if (level >= maxLevel)
	{
		oLevel0 = oLevel1 = static_cast<size_t>(maxLevel);
		oDLevel = TNumTraits::zero;
	}
	else
	{
		const TScalar floorLevel = num::floor(level);
		oLevel0 = static_cast<size_t>(floorLevel);
		oLevel1 = oLevel0 + 1;
		oDLevel = level - floorLevel;
	}
}




io::Image::TPixel Image::nearest(size_t iLevelU, size_t iLevelV, const TPoint2D& uv) const
{
	LASS_ASSERT(iLevelU < numLevelsU_ && iLevelV < numLevelsV_);
	const io::Image& mipMap = *mipMaps_[iLevelV][iLevelU];

    const unsigned cols = mipMap.cols();
	const unsigned rows = mipMap.rows();

	const TScalar x = num::fractional(uv.x) * cols;
	const TScalar y = num::fractional(1.f - uv.y) * rows;
	const unsigned i0 = static_cast<unsigned>(num::floor(x));
	const unsigned j0 = static_cast<unsigned>(num::floor(y));

	return mipMap(j0, i0);
}




io::Image::TPixel Image::bilinear(size_t iLevelU, size_t iLevelV, const TPoint2D& uv) const
{
	LASS_ASSERT(iLevelU < numLevelsU_ && iLevelV < numLevelsV_);
	const io::Image& mipMap = *mipMaps_[iLevelV][iLevelU];

    const unsigned cols = mipMap.cols();
	const unsigned rows = mipMap.rows();

	const TScalar x = num::mod(uv.x * cols - .5f, static_cast<TScalar>(cols));
	const TScalar y = num::mod((1.f - uv.y) * rows - .5f, static_cast<TScalar>(rows));
	const TScalar x0 = num::floor(x);
	const TScalar y0 = num::floor(y);
	const TScalar dx = x - x0;
	const TScalar dy = y - y0;

	const unsigned i0 = static_cast<unsigned>(x0);
	const unsigned j0 = static_cast<unsigned>(y0);
	const unsigned i1 = i0 < (cols - 1) ? (i0 + 1) : 0;
	const unsigned j1 = j0 < (rows - 1) ? (j0 + 1) : 0;

	return
		(mipMap(j0, i0) * (TNumTraits::one - dx) + mipMap(j0, i1) * dx) * (TNumTraits::one - dy) +
		(mipMap(j1, i0) * (TNumTraits::one - dx) + mipMap(j1, i1) * dx) * dy;
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

