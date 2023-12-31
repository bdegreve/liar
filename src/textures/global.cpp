/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
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
#include "global.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(Global, "use global texture parameters instead of local")
PY_CLASS_CONSTRUCTOR_1(Global, const TTexturePtr&);

// --- public --------------------------------------------------------------------------------------

Global::Global(const TTexturePtr& texture):
	ContextMapping(texture)
{
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void Global::doTransformContext(const Sample&, IntersectionContext& context) const
{
	const TTransformation3D& localToWorld = context.localToWorld();
	context.setPoint(prim::transform(context.point(), localToWorld));
	context.setDPoint_dI(prim::transform(context.dPoint_dI(), localToWorld));
	context.setDPoint_dJ(prim::transform(context.dPoint_dJ(), localToWorld));
	context.setDPoint_dU(prim::transform(context.dPoint_dU(), localToWorld));
	context.setDPoint_dV(prim::transform(context.dPoint_dV(), localToWorld));
#pragma LASS_TODO("perhaps we need to transform other Uv dependent quantities as well [Brams]")
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
