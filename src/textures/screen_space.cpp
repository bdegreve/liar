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
#include "screen_space.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(ScreenSpace, "set UV to screen space coordinates")
PY_CLASS_CONSTRUCTOR_1(ScreenSpace, const TTexturePtr&);

// --- public --------------------------------------------------------------------------------------

ScreenSpace::ScreenSpace(const TTexturePtr& texture):
	UnaryOperator(texture)
{
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const Spectral ScreenSpace::doLookUp(const Sample& sample, const IntersectionContext& context) const
{
	IntersectionContext temp(context);
	temp.setUv(sample.screenCoordinate());
	temp.setDUv_dI(TVector2D(1, 0));
	temp.setDUv_dJ(TVector2D(0, 1));
	temp.setDPoint_dU(context.dPoint_dI());
	temp.setDPoint_dV(context.dPoint_dJ());
	temp.setDNormal_dU(context.dNormal_dI());
	temp.setDNormal_dV(context.dNormal_dJ());
	return texture()->lookUp(sample, temp);
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

