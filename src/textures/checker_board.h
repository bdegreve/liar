/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LIAR isn't a raytracer
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

/** @class liar::textures::CheckerBoard
 *  @brief mixes two textures in checker board style
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_CHECKER_BOARD_H
#define LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_CHECKER_BOARD_H

#include "textures_common.h"
#include "../kernel/texture.h"

namespace liar
{
namespace textures
{

class LIAR_TEXTURES_DLL CheckerBoard: public kernel::Texture
{
	PY_HEADER(kernel::Texture)
public:

	CheckerBoard(const kernel::TTexturePtr& iA, const kernel::TTexturePtr& iB);

	const kernel::TTexturePtr& textureA() const;
	const kernel::TTexturePtr& textureB() const;
	void setTextureA(const kernel::TTexturePtr& iA);
	void setTextureB(const kernel::TTexturePtr& iB);

private:

	TValue doLookUp(const kernel::IntersectionContext& iContext) const;

	kernel::TTexturePtr a_;
	kernel::TTexturePtr b_;
};

}

}

#endif

// EOF
