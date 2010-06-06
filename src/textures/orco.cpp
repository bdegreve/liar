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
#include "orco.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(OrCo, "use global texture parameters instead of local")
PY_CLASS_CONSTRUCTOR_1(OrCo, const TTexturePtr&);
PY_CLASS_MEMBER_RW(OrCo, texture, setTexture);

// --- public --------------------------------------------------------------------------------------

OrCo::OrCo(const TTexturePtr& texture):
	texture_(texture)
{
}



const TTexturePtr& OrCo::texture() const
{
	return texture_;
}



void OrCo::setTexture(const TTexturePtr& texture)
{
	texture_ = texture;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const XYZ OrCo::doLookUp(const Sample& sample, const IntersectionContext& context) const
{
	if (context.bounds().isEmpty())
	{
		return texture_->lookUp(sample, context);
	}
	const TPoint3D center = context.bounds().center().affine();
	const TVector3D size = context.bounds().size();
	const TVector3D scale(
		size.x ? 2 / size.x : 1,
		size.y ? 2 / size.y : 1,
		size.z ? 2 / size.z : 1);
	IntersectionContext temp(context);
	temp.setPoint(TPoint3D((context.point() - center) * scale));
	temp.setDPoint_dI(context.dPoint_dI() * scale);
	temp.setDPoint_dJ(context.dPoint_dJ() * scale);
	temp.setDPoint_dU(context.dPoint_dU() * scale);
	temp.setDPoint_dV(context.dPoint_dV() * scale);
#pragma LASS_TODO("perhaps we need to transform other Uv dependent quantities as well [Brams]")
	return texture_->lookUp(sample, temp);
}



const TPyObjectPtr OrCo::doGetState() const
{
	return python::makeTuple(texture_);
}



void OrCo::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, texture_);
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

