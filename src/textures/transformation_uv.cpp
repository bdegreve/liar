/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2009  Bram de Greve
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

#include "textures_common.h"
#include "transformation_uv.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(TransformationUv, "transform Uv coordinates")
PY_CLASS_CONSTRUCTOR_2(TransformationUv, const TTexturePtr&, const TTransformation2D&);
PY_CLASS_MEMBER_RW(TransformationUv, texture, setTexture);
PY_CLASS_MEMBER_RW(TransformationUv, transformation, setTransformation);

// --- public --------------------------------------------------------------------------------------

TransformationUv::TransformationUv(
		const TTexturePtr& texture, const TTransformation2D& transformation):
	texture_(texture),
	transformation_(transformation)
{
}



const TTexturePtr& TransformationUv::texture() const
{
	return texture_;
}



void TransformationUv::setTexture(const TTexturePtr& texture)
{
	texture_ = texture;
}



const TTransformation2D& TransformationUv::transformation() const
{
	return transformation_;
}



void TransformationUv::setTransformation(const TTransformation2D& transformation)
{
	transformation_ = transformation;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const Spectrum TransformationUv::doLookUp(const Sample& sample, const IntersectionContext& context) const
{
	IntersectionContext temp(context);
	temp.setUv(prim::transform(context.uv(), transformation_));
	temp.setDUv_dI(prim::transform(context.dUv_dI(), transformation_));
	temp.setDUv_dJ(prim::transform(context.dUv_dJ(), transformation_));
#pragma LASS_TODO("perhaps we need to transform other Uv dependent quantities as well [Brams]")
	return texture_->lookUp(sample, temp);
}



const TPyObjectPtr TransformationUv::doGetState() const
{
	return python::makeTuple(texture_, transformation_);
}



void TransformationUv::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, texture_, transformation_);
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

