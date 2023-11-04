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
#include "transformation_uv.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(TransformationUv, "transform Uv coordinates")
PY_CLASS_CONSTRUCTOR_2(TransformationUv, const TTexturePtr&, const TTransformation2D&);
PY_CLASS_MEMBER_RW(TransformationUv, transformation, setTransformation);

// --- public --------------------------------------------------------------------------------------

TransformationUv::TransformationUv(const TTexturePtr& texture, const TTransformation2D& transformation):
	ContextMapping(texture)
{
	setTransformation(transformation);
}



const TTransformation2D& TransformationUv::transformation() const
{
	return forward_;
}



void TransformationUv::setTransformation(const TTransformation2D& transformation)
{
	forward_ = transformation;
	inverse_ = forward_.inverse();
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void TransformationUv::doTransformContext(const Sample&, IntersectionContext& context) const
{
	context.setUv(prim::transform(context.uv(), forward_));
	context.setDUv_dI(prim::transform(context.dUv_dI(), forward_));
	context.setDUv_dJ(prim::transform(context.dUv_dJ(), forward_));

	// we need derivatives from old (u,v) coordinates to new (s,t) coordinates.
	const TScalar* const invMat = inverse_.matrix();
	const TVector2D dUv_dS(invMat[0], invMat[3]);
	const TVector2D dUv_dT(invMat[1], invMat[4]);

	const TVector3D dPoint_dS = context.dPoint_dU() * dUv_dS.x + context.dPoint_dV() * dUv_dS.y;
	const TVector3D dPoint_dT = context.dPoint_dU() * dUv_dT.x + context.dPoint_dV() * dUv_dT.y;
	context.setDPoint_dU(dPoint_dS);
	context.setDPoint_dV(dPoint_dT);

	const TVector3D dNormal_dS = context.dNormal_dU() * dUv_dS.x + context.dNormal_dV() * dUv_dS.y;
	const TVector3D dNormal_dT = context.dNormal_dU() * dUv_dT.x + context.dNormal_dV() * dUv_dT.y;
	context.setDNormal_dU(dNormal_dS);
	context.setDNormal_dV(dNormal_dT);
}



const TPyObjectPtr TransformationUv::doGetState() const
{
	return python::makeTuple(UnaryOperator::doGetState(), forward_);
}



void TransformationUv::doSetState(const TPyObjectPtr& state)
{
	TPyObjectPtr parentState;
	TTransformation2D transformation;
	python::decodeTuple(state, parentState, transformation);
	UnaryOperator::doSetState(parentState);
	setTransformation(transformation);
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
