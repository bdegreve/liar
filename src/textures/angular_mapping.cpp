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
#include "angular_mapping.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(AngularMapping, "converts point to angular coordinates as used in light probes")
PY_CLASS_CONSTRUCTOR_1(AngularMapping, const TTexturePtr&);
PY_CLASS_CONSTRUCTOR_2(AngularMapping, const TTexturePtr&, const TPoint3D&);
PY_CLASS_MEMBER_RW(AngularMapping, center, setCenter);

// --- public --------------------------------------------------------------------------------------

AngularMapping::AngularMapping(const TTexturePtr& texture):
	UnaryOperator(texture),
	center_(0, 0, 0)
{
}



AngularMapping::AngularMapping(const TTexturePtr& texture, const TPoint3D& center):
	UnaryOperator(texture),
	center_(center)
{
}



const TPoint3D& AngularMapping::center() const
{
	return center_;
}



void AngularMapping::setCenter(const TPoint3D& center)
{
	center_ = center;
}



// --- protected -----------------------------------------------------------------------------------

const TPyObjectPtr AngularMapping::doGetState() const
{
	return python::makeTuple(UnaryOperator::doGetState(), center_);
}



void AngularMapping::doSetState(const TPyObjectPtr& state)
{
	TPyObjectPtr parentState;
	python::decodeTuple(state, parentState, center_);
	UnaryOperator::doSetState(parentState);
}



// --- private -------------------------------------------------------------------------------------

const XYZ AngularMapping::doLookUp(const Sample& sample, const IntersectionContext& context) const
{
	IntersectionContext temp(context);
	const TVector3D dir0 = context.point() - center_;
	const TPoint2D uv0 = uv(dir0);
	const TPoint2D uvI = uv(dir0 + context.dPoint_dI());
	const TPoint2D uvJ = uv(dir0 + context.dPoint_dJ());
	temp.setUv(uv0);
	temp.setDUv_dI(uvI - uv0);
	temp.setDUv_dJ(uvJ - uv0);
#pragma LASS_TODO("perhaps we need to transform other Uv dependent quantities as well [Brams]")
	return texture()->lookUp(sample, temp);
}



inline const TPoint2D AngularMapping::uv(const TVector3D& dir) const
{
	const TVector3D d = dir.normal();
	const TScalar r = num::acos(d.y) / (TNumTraits::pi * num::sqrt(num::sqr(d.x) + num::sqr(d.z)));
	const TScalar u = (dir.x * r + 1) / 2;
	const TScalar v = (dir.z * r + 1) / 2;
	return TPoint2D(u, v);
}


// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

