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

CheckerBoard::CheckerBoard(const TTexturePtr& a, const TTexturePtr& b):
	Mix2(a, b),
	split_(0.5f, 0.5f),
	antiAliasing_(defaultAntiAliasing_)
{
}



const TVector2D& CheckerBoard::split() const
{
	return split_;
}



void CheckerBoard::setSplit(const TVector2D& split)
{
	split_ = split;
}



const std::string CheckerBoard::antiAliasing() const
{
	return antiAliasingDictionary_.key(antiAliasing_);
}



void CheckerBoard::setAntiAliasing(const std::string& mode)
{
	antiAliasing_ = antiAliasingDictionary_[stde::tolower(mode)];
}



void CheckerBoard::setDefaultAntiAliasing(const std::string& mode)
{
	defaultAntiAliasing_ = antiAliasingDictionary_[stde::tolower(mode)];
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const Spectrum 
CheckerBoard::doLookUp(const Sample& sample, const IntersectionContext& context) const
{
#pragma LASS_FIXME("points need transform too [Bramz]")
	const TVector2D uv = context.uv().position().transform(num::fractional);

	switch (antiAliasing_)
	{
	case aaNone:
		return ((uv.x < split_.x) == (uv.y < split_.y) ? textureA() : textureB())->lookUp(
			sample, context);
		
	case aaBilinear:
		{
			const TVector2D dUv = prim::pointwiseMax(
				context.dUv_dI().transform(num::abs), context.dUv_dJ().transform(num::abs));
            const TVector2D uvMin = uv - dUv / 2;
			const TVector2D uvMax = uv + dUv / 2;
			const TScalar area = dUv.x * dUv.y;
			if (area > 0)
			{
				const TScalar areaA = integrate(uvMin, uvMax);
				const TScalar weightA = num::clamp(areaA / area, TNumTraits::zero, TNumTraits::one);
                return weightA * textureA()->lookUp(sample, context) +
					(1 - weightA) * textureB()->lookUp(sample, context);
			}
			else
			{
				return ((uv.x < split_.x) == (uv.y < split_.y) ? textureA() : textureB())->lookUp(
					sample, context);
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



void CheckerBoard::doSetMixState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, split_);
}



const TScalar CheckerBoard::integrate(const TVector2D& min, const TVector2D& max) const
{
	const TVector2D min0 = min.transform(num::floor);
	const TVector2D max0 = max.transform(num::floor);
	const TVector2D delta0 = max0 - min0;
	const TVector2D dMin = min - min0;
	const TVector2D dMax = max - max0;

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
	result.enableSuggestions(true);
	result.add("none", aaNone);
	result.add("bilinear", aaBilinear);
	return result;
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

