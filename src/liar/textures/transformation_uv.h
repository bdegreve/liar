/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2025  Bram de Greve
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
 *  http://liar.bramz.net/
 */

/** @class liar::textures::TransformationUv
 *  @brief Applies transformation to UV coordinates.
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_TRANSFORMATION_UV_H
#define LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_TRANSFORMATION_UV_H

#include "textures_common.h"
#include "context_mapping.h"
#include <lass/prim/transformation_2d.h>

namespace liar
{
namespace textures
{

typedef prim::Transformation2D<TScalar> TTransformation2D;

class LIAR_TEXTURES_DLL TransformationUv: public ContextMapping
{
	PY_HEADER(ContextMapping)
public:

	TransformationUv(const TTextureRef& texture, const TTransformation2D& transformation);

	const TTransformation2D& transformation() const;
	void setTransformation(const TTransformation2D& transformation);

protected:
	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

private:
	void doTransformContext(const Sample& sample, IntersectionContext& context) const override;

	TTransformation2D forward_;
	TTransformation2D inverse_;
};

}

}

#endif

// EOF
