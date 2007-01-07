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

/** @class liar::textures::TransformationUv
 *  @brief Applies transformation to UV coordinates.
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_TRANSFORMATION_UV_H
#define LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_TRANSFORMATION_UV_H

#include "textures_common.h"
#include "../kernel/texture.h"
#include <lass/prim/transformation_2d.h>

namespace liar
{
namespace textures
{

typedef prim::Transformation2D<TScalar> TTransformation2D;

class LIAR_TEXTURES_DLL TransformationUv: public Texture
{
	PY_HEADER(Texture)
public:

	TransformationUv(const TTexturePtr& texture, const TTransformation2D& transformation);

	const TTexturePtr& texture() const;
	void setTexture(const TTexturePtr& texture);

	const TTransformation2D& transformation() const;
	void setTransformation(const TTransformation2D& transformation);

private:

	const Spectrum doLookUp(const Sample& sample, 
		const IntersectionContext& context) const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	TTexturePtr texture_;
	TTransformation2D transformation_;
};

}

}

#endif

// EOF
