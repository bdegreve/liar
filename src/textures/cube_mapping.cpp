/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
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
#include "cube_mapping.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(CubeMapping, "converts point to angular coordinates as used in light probes")
PY_CLASS_CONSTRUCTOR_1(CubeMapping, const TTexturePtr&);
PY_CLASS_MEMBER_RW(CubeMapping, texture, setTexture);

// --- public --------------------------------------------------------------------------------------

CubeMapping::CubeMapping(const TTexturePtr& texture):
	texture_(texture)
{
}



const TTexturePtr& CubeMapping::texture() const
{
	return texture_;
}



void CubeMapping::setTexture(const TTexturePtr& texture)
{
	texture_ = texture;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const XYZ CubeMapping::doLookUp(const Sample& sample, const IntersectionContext& context) const
{
	const TVector3D& n = context.normal();
	size_t i, j;
	if (num::abs(n.z) >= num::abs(n.x) && num::abs(n.z) >= num::abs(n.y))
	{
		i = 0; 
		j = 1;
	}
	else if (num::abs(n.y) >= num::abs(n.x))
	{
		i = 0;
		j = 2;
	}
	else
	{
		i = 1;
		j = 2;
	}
	IntersectionContext temp(context);
	temp.setUv(TPoint2D(context.point()[i], context.point()[j]));
	temp.setDUv_dI(TVector2D(context.dPoint_dI()[i], context.dPoint_dI()[j]));
	temp.setDUv_dJ(TVector2D(context.dPoint_dJ()[i], context.dPoint_dJ()[j]));
	return texture_->lookUp(sample, temp);
}



const TPyObjectPtr CubeMapping::doGetState() const
{
	return python::makeTuple(texture_);
}



void CubeMapping::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, texture_);
}


// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

