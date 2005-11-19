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

/** @class liar::textures::LinearInterpolator
 *  @brief interpolates between textures based on gray value of control texture
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_LINEAR_INTERPOLATOR_H
#define LIAR_GUARDIAN_OF_INCLUSION_LINEAR_INTERPOLATOR_H

#include "textures_common.h"
#include "../kernel/texture.h"

namespace liar
{
namespace textures
{

class LIAR_TEXTURES_DLL LinearInterpolator: public Texture
{
	PY_HEADER(Texture)
public:

	typedef std::pair<TScalar, TTexturePtr> TKeyTexture;
	typedef std::vector<TKeyTexture> TKeyTextures;

	LinearInterpolator();
	LinearInterpolator(const TKeyTextures& iKeyTextures, 
		const TTexturePtr& iControlTexture);

	const TKeyTextures& keys() const;
	const TTexturePtr& control() const;

	void setKeys(const TKeyTextures& iKeyTextures);
	void setControl(const TTexturePtr& iControlTexture);

	void addKey(TScalar iKeyValue, const TTexturePtr& iKeyTexture);

private:

	struct LesserKey
	{
		bool operator()(const TKeyTexture& iA, const TKeyTexture& iB) const { return iA.first < iB.first; }
	};

	Spectrum doLookUp(const Sample& iSample, 
		const IntersectionContext& iContext) const;

	TKeyTextures keys_;
	TTexturePtr control_;
};

}

}

#endif

// EOF
