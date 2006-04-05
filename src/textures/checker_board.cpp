/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2005  Bram de Greve
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
#include "checker_board.h"
#include <lass/stde/extended_string.h>

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS(CheckerBoard)
PY_CLASS_CONSTRUCTOR_2(CheckerBoard, TTexturePtr, TTexturePtr);
PY_CLASS_MEMBER_RW(CheckerBoard, "split", split, setSplit);
PY_CLASS_MEMBER_RW(CheckerBoard, "antiAliasing", antiAliasing, setAntiAliasing);
PY_CLASS_STATIC_METHOD(CheckerBoard, setDefaultAntiAliasing);
PY_CLASS_STATIC_CONST(CheckerBoard, "AA_NONE", "none");
PY_CLASS_STATIC_CONST(CheckerBoard, "AA_BILINEAR", "bilinear");

CheckerBoard::TAntiAliasingDictionary CheckerBoard::antiAliasingDictionary_ = 
	CheckerBoard::makeAntiAliasingDictionary();
CheckerBoard::AntiAliasing CheckerBoard::defaultAntiAliasing_ = CheckerBoard::aaBilinear;

// --- public --------------------------------------------------------------------------------------

CheckerBoard::CheckerBoard(const TTexturePtr& iA, const TTexturePtr& iB):
	Mix2(&Type, iA, iB),
	split_(0.5f, 0.5f),
	antiAliasing_(defaultAntiAliasing_)
{
}



const TVector2D& CheckerBoard::split() const
{
	return split_;
}



void CheckerBoard::setSplit(const TVector2D& iSplit)
{
	split_ = iSplit;
}



const std::string CheckerBoard::antiAliasing() const
{
	return antiAliasingDictionary_.key(antiAliasing_);
}



void CheckerBoard::setAntiAliasing(const std::string& iMode)
{
	antiAliasing_ = antiAliasingDictionary_[stde::tolower(iMode)];
}



void CheckerBoard::setDefaultAntiAliasing(const std::string& iMode)
{
	defaultAntiAliasing_ = antiAliasingDictionary_[stde::tolower(iMode)];
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const Spectrum 
CheckerBoard::doLookUp(const Sample& iSample, const IntersectionContext& iContext) const
{
#pragma LASS_FIXME("points need transform too [Bramz]")
	const TVector2D uv = iContext.uv().position().transform(num::fractional);

	switch (antiAliasing_)
	{
	case aaNone:
		return ((uv.x < split_.x) == (uv.y < split_.y) ? textureA() : textureB())->lookUp(
			iSample, iContext);
		
	case aaBilinear:
		{
			const TVector2D dUv = prim::pointwiseMax(
				iContext.dUv_dI().transform(num::abs), iContext.dUv_dJ().transform(num::abs));
            const TVector2D uvMin = uv - dUv;
			const TVector2D uvMax = uv + dUv;
			const TScalar area = 4 * dUv.x * dUv.y;
			if (area > 0)
			{
				const TScalar areaA = integrate(uvMin, uvMax);
				const TScalar weightA = num::clamp(areaA / area, TNumTraits::zero, TNumTraits::one);
                return weightA * textureA()->lookUp(iSample, iContext) +
					(1 - weightA) * textureB()->lookUp(iSample, iContext);
			}
			else
			{
				return ((uv.x < split_.x) == (uv.y < split_.y) ? textureA() : textureB())->lookUp(
					iSample, iContext);
			}
		}

	default:
		LASS_ASSERT_UNREACHABLE;
	}

	return Spectrum();
}



const TPyObjectPtr CheckerBoard::doGetMixState() const
{
	return python::makeTuple(split_);
}



void CheckerBoard::doSetMixState(const TPyObjectPtr& iState)
{
	python::decodeTuple(iState, split_);
}



const TScalar CheckerBoard::integrate(const TVector2D& iMin, const TVector2D& iMax) const
{
	const TVector2D min0 = iMin.transform(num::floor);
	const TVector2D max0 = iMax.transform(num::floor);
	const TVector2D delta0 = max0 - min0;
	const TVector2D dMin = iMin - min0;
	const TVector2D dMax = iMax - max0;

	const TVector2D dMinU(std::min(dMin.x, split_.x), std::max(dMin.x - split_.x, TNumTraits::zero));
	const TVector2D dMinV(std::min(dMin.y, split_.y), std::max(dMin.y - split_.y, TNumTraits::zero));
	const TVector2D dMaxU(std::min(dMax.x, split_.x), std::max(dMax.x - split_.x, TNumTraits::zero));
	const TVector2D dMaxV(std::min(dMax.y, split_.y), std::max(dMax.y - split_.y, TNumTraits::zero));
	const TVector2D oneU(split_.x, TNumTraits::one - split_.x);
	const TVector2D oneV(split_.y, TNumTraits::one - split_.y);

	TScalar result = (split_.x * split_.y + (1 - split_.x) * (1 - split_.y)) * delta0.x * delta0.y;
	result += (dot(oneU, dMaxV) - dot(oneU, dMinV)) * delta0.x;
	result += (dot(dMaxU, oneV) - dot(dMinU, oneV)) * delta0.y;
	result += dot(dMaxU, dMaxV) - dot(dMaxU, dMinV);
	result += dot(dMinU, dMinV) - dot(dMinU, dMaxV);

	return result;
}



CheckerBoard::TAntiAliasingDictionary CheckerBoard::makeAntiAliasingDictionary()
{
	TAntiAliasingDictionary result;
	result.add("none", aaNone);
	result.add("bilinear", aaBilinear);
	return result;
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

