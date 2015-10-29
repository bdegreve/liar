/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2010  Bram de Greve
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
PY_CLASS_CONSTRUCTOR_2(TransformationLocal, const TTexturePtr&, const TTransformation3D&);
PY_CLASS_CONSTRUCTOR_2(TransformationLocal, const TTexturePtr&, const TPyTransformation3DPtr&);
PY_CLASS_MEMBER_RW(TransformationLocal, transformation, setTransformation);

// --- public --------------------------------------------------------------------------------------

TransformationLocal::TransformationLocal(const TTexturePtr& texture, const TTransformation3D& transformation):
	UnaryOperator(texture),
	transformation_(transformation)
{
}



TransformationLocal::TransformationLocal(const TTexturePtr& texture, const TPyTransformation3DPtr& transformation):
	UnaryOperator(texture),
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

const Spectral TransformationLocal::doLookUp(const Sample& sample, const IntersectionContext& context) const
{
	IntersectionContext temp(context);
	temp.setPoint(prim::transform(context.point(), transformation_));
	temp.setDPoint_dI(prim::transform(context.dPoint_dI(), transformation_));
	temp.setDPoint_dJ(prim::transform(context.dPoint_dJ(), transformation_));
	temp.setDPoint_dU(prim::transform(context.dPoint_dU(), transformation_));
	temp.setDPoint_dV(prim::transform(context.dPoint_dV(), transformation_));
#pragma LASS_TODO("perhaps we need to transform other Uv dependent quantities as well [Brams]")
	return texture()->lookUp(sample, temp);
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

