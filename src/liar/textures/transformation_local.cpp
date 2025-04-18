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

#include "textures_common.h"
#include "transformation_local.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(TransformationLocal, "transform local coordinates")
PY_CLASS_CONSTRUCTOR_2(TransformationLocal, const TTextureRef&, const TTransformation3D&);
PY_CLASS_CONSTRUCTOR_2(TransformationLocal, const TTextureRef&, const TPyTransformation3DRef&);
PY_CLASS_MEMBER_RW(TransformationLocal, transformation, setTransformation);

// --- public --------------------------------------------------------------------------------------

TransformationLocal::TransformationLocal(const TTextureRef& texture, const TTransformation3D& transformation):
	ContextMapping(texture),
	transformation_(transformation)
{
}



TransformationLocal::TransformationLocal(const TTextureRef& texture, const TPyTransformation3DRef& transformation):
	ContextMapping(texture),
	transformation_(transformation->transformation())
{
}



const TTransformation3D& TransformationLocal::transformation() const
{
	return transformation_;
}



void TransformationLocal::setTransformation(const TTransformation3D& transformation)
{
	transformation_ = transformation;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void TransformationLocal::doTransformContext(const Sample&, IntersectionContext& context) const
{
	context.setPoint(prim::transform(context.point(), transformation_));
	context.setDPoint_dI(prim::transform(context.dPoint_dI(), transformation_));
	context.setDPoint_dJ(prim::transform(context.dPoint_dJ(), transformation_));
	context.setDPoint_dU(prim::transform(context.dPoint_dU(), transformation_));
	context.setDPoint_dV(prim::transform(context.dPoint_dV(), transformation_));
#pragma LASS_TODO("perhaps we need to transform other Uv dependent quantities as well [Brams]")
}



const TPyObjectPtr TransformationLocal::doGetState() const
{
	return python::makeTuple(UnaryOperator::doGetState(), transformation_);
}



void TransformationLocal::doSetState(const TPyObjectPtr& state)
{
	TPyObjectPtr parentState;
	python::decodeTuple(state, parentState, transformation_);
	UnaryOperator::doSetState(parentState);
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
