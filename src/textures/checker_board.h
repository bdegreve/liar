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
 * *  http://liar.sourceforge.net
 */

/** @class liar::textures::CheckerBoard
 *  @brief mixes two textures in checker board style
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_CHECKER_BOARD_H
#define LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_CHECKER_BOARD_H

#include "textures_common.h"
#include "mix_2.h"
#include <lass/util/dictionary.h>

namespace liar
{
namespace textures
{

class LIAR_TEXTURES_DLL CheckerBoard: public Mix2
{
	PY_HEADER(Mix2)
public:

	CheckerBoard(const kernel::TTexturePtr& iA, const kernel::TTexturePtr& iB);

	const std::string antiAliasing() const;
	void setAntiAliasing(const std::string& iMode);

	const TVector2D& split() const;
	void setSplit(const TVector2D& iSplit);

	static void setDefaultAntiAliasing(const std::string& iMode);

private:

	enum AntiAliasing
	{
		aaNone = 0,
		aaBilinear,
		numAntiAliasing
	};

	typedef util::Dictionary<std::string, AntiAliasing> TAntiAliasingDictionary;

	kernel::Spectrum doLookUp(const kernel::Sample& iSample, 
		const kernel::IntersectionContext& iContext) const;
	TScalar integrate(const TVector2D& iMin, const TVector2D& iMax) const;

	static TAntiAliasingDictionary makeAntiAliasingDictionary();

	TVector2D split_;
	AntiAliasing antiAliasing_;

	static TAntiAliasingDictionary antiAliasingDictionary_;
	static AntiAliasing defaultAntiAliasing_;
};

}

}

#endif

// EOF
