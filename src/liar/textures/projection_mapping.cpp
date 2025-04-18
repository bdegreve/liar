/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2025  Bram de Greve (bramz@users.sourceforge.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  http://liar.bramz.net/
 */

#include "textures_common.h"
#include "projection_mapping.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(ProjectionMapping, "computes UV coordinates using a Projection")
PY_CLASS_CONSTRUCTOR_2(ProjectionMapping, const TTextureRef&, const TProjectionRef&);
PY_CLASS_MEMBER_RW(ProjectionMapping, projection, setProjection);

// --- public --------------------------------------------------------------------------------------

ProjectionMapping::ProjectionMapping(const TTextureRef& texture, const TProjectionRef& projection):
	ContextMapping(texture),
	projection_(projection)
{
}



const TProjectionRef& ProjectionMapping::projection() const
{
	return projection_;
}



void ProjectionMapping::setProjection(const TProjectionRef& projection)
{
	projection_ = projection;
}



// --- protected -----------------------------------------------------------------------------------

const TPyObjectPtr ProjectionMapping::doGetState() const
{
	return python::makeTuple(UnaryOperator::doGetState(), projection_);
}



void ProjectionMapping::doSetState(const TPyObjectPtr& state)
{
	TPyObjectPtr parentState;
	python::decodeTuple(state, parentState, projection_);
	UnaryOperator::doSetState(parentState);
}



// --- private -------------------------------------------------------------------------------------

void ProjectionMapping::doTransformContext(const Sample&, IntersectionContext& context) const
{
	TRay3D ray;
	TScalar t;
	const TPoint2D uv0 = projection_->uv(context.point(), ray, t);
	const TPoint2D uvI = projection_->uv(context.point() + context.dPoint_dI(), ray, t);
	const TPoint2D uvJ = projection_->uv(context.point() + context.dPoint_dJ(), ray, t);
	context.setUv(uv0);
	context.setDUv_dI(uvI - uv0);
	context.setDUv_dJ(uvJ - uv0);
#pragma LASS_TODO("perhaps we need to transform other Uv dependent quantities as well [Brams]")
}


// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
