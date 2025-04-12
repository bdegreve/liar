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

#include "textures_common.h"
#include "checker_board.h"
#include <lass/stde/extended_string.h>

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(CheckerBoard, "mixes two textures in 2D checkerboard pattern")
PY_CLASS_CONSTRUCTOR_2(CheckerBoard, TTextureRef, TTextureRef);
PY_CLASS_MEMBER_RW(CheckerBoard, split, setSplit);
PY_CLASS_MEMBER_RW(CheckerBoard, antiAliasing, setAntiAliasing);
PY_CLASS_STATIC_METHOD(CheckerBoard, setDefaultAntiAliasing);
PY_CLASS_STATIC_CONST(CheckerBoard, "AA_NONE", "none");
PY_CLASS_STATIC_CONST(CheckerBoard, "AA_BILINEAR", "bilinear");

CheckerBoard::TAntiAliasingDictionary CheckerBoard::antiAliasingDictionary_ =
	CheckerBoard::makeAntiAliasingDictionary();
CheckerBoard::AntiAliasing CheckerBoard::defaultAntiAliasing_ = CheckerBoard::aaBilinear;

// --- public --------------------------------------------------------------------------------------

CheckerBoard::CheckerBoard(const TTextureRef& a, const TTextureRef& b):
	BinaryOperator(a, b),
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

const TPyObjectPtr CheckerBoard::doGetState() const
{
	return python::makeTuple(BinaryOperator::doGetState(), split_, antiAliasing());
}



void CheckerBoard::doSetState(const TPyObjectPtr& state)
{
	TPyObjectPtr parentState;
	std::string aa;
	python::decodeTuple(state, parentState, split_, aa);
	setAntiAliasing(aa);
	BinaryOperator::doSetState(parentState);
}



// --- private -------------------------------------------------------------------------------------

const Spectral CheckerBoard::doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const
{
	const TValue wA = weightA(context);
	if (wA >= 1)
	{
		return textureA()->lookUp(sample, context, type);
	}
	else if (wA <= 0)
	{
		return textureB()->lookUp(sample, context, type);
	}
	else
	{
		return Spectral(wA * textureA()->lookUp(sample, context, SpectralType::Illuminant) + (1 - wA) * textureB()->lookUp(sample, context, SpectralType::Illuminant), type);
	}
}


Texture::TValue CheckerBoard::doScalarLookUp(const Sample& sample, const IntersectionContext& context) const
{
	const TValue wA = weightA(context);
	if (wA >= 1)
	{
		return textureA()->scalarLookUp(sample, context);
	}
	else if (wA <= 0)
	{
		return textureB()->scalarLookUp(sample, context);
	}
	else
	{
		return wA * textureA()->scalarLookUp(sample, context) + (1 - wA) * textureB()->scalarLookUp(sample, context);
	}
}


Texture::TValue CheckerBoard::weightA(const IntersectionContext& context) const
{
#pragma LASS_FIXME("points need transform too [Bramz]")
	const TVector2D uv = context.uv().position().transform(num::fractional);

	switch (antiAliasing_)
	{
	case aaNone:
		return (uv.x < split_.x) == (uv.y < split_.y) ? 1.f : 0.f;

	case aaBilinear:
	{
		const TVector2D dUv = prim::pointwiseMax(context.dUv_dI().transform(num::abs), context.dUv_dJ().transform(num::abs));
		const TVector2D uvMin = uv - dUv / 2;
		const TVector2D uvMax = uv + dUv / 2;
		const TValue area = static_cast<TValue>(dUv.x * dUv.y);
		if (area > 0)
		{
			const TValue areaA = integrate(uvMin, uvMax);
			return num::clamp(areaA / area, static_cast<TValue>(0), static_cast<TValue>(1));
		}
		else
		{
			return (uv.x < split_.x) == (uv.y < split_.y) ? 1.f : 0.f;
		}
	}

	default:
		LASS_ASSERT_UNREACHABLE;
	}

	return 1.f;
}



Texture::TValue CheckerBoard::integrate(const TVector2D& min, const TVector2D& max) const
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

	return static_cast<TValue>(result);
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
