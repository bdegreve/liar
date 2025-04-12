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
#include "orco.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(OrCo, "use global texture parameters instead of local")
PY_CLASS_CONSTRUCTOR_1(OrCo, const TTextureRef&);

// --- public --------------------------------------------------------------------------------------

OrCo::OrCo(const TTextureRef& texture):
	ContextMapping(texture)
{
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void OrCo::doTransformContext(const Sample&, IntersectionContext& context) const
{
	if (context.bounds().isEmpty())
	{
		return;
	}

	const TPoint3D center = context.bounds().center().affine();
	const TVector3D size = context.bounds().size();
	const TVector3D scale(
		size.x > 0 ? 2 / size.x : 1,
		size.y > 0 ? 2 / size.y : 1,
		size.z > 0 ? 2 / size.z : 1);
	context.setPoint(TPoint3D((context.point() - center) * scale));
	context.setDPoint_dI(context.dPoint_dI() * scale);
	context.setDPoint_dJ(context.dPoint_dJ() * scale);
	context.setDPoint_dU(context.dPoint_dU() * scale);
	context.setDPoint_dV(context.dPoint_dV() * scale);
#pragma LASS_TODO("perhaps we need to transform other Uv dependent quantities as well [Brams]")
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
