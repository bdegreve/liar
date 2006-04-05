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

/** @class liar::textures::Mix2
 *  @brief a base class for texture that mixes two input textures
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_MIX_2_H
#define LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_MIX_2_H

#include "textures_common.h"
#include "../kernel/texture.h"

namespace liar
{
namespace textures
{

class LIAR_TEXTURES_DLL Mix2: public Texture
{
	PY_HEADER(Texture)
public:

	const TTexturePtr& textureA() const { return a_; }
	const TTexturePtr& textureB() const { return b_; }
	void setTextureA(const TTexturePtr& iA);
	void setTextureB(const TTexturePtr& iB);

protected:

	Mix2(PyTypeObject* iType, const TTexturePtr& iA, const TTexturePtr& iB);

private:

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& iState);

	virtual const TPyObjectPtr doGetMixState() const = 0;
	virtual void doSetMixState(const TPyObjectPtr& iState) = 0;

	TTexturePtr a_;
	TTexturePtr b_;
};

}

}

#endif

// EOF
