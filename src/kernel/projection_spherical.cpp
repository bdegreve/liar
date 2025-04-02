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

#include "kernel_common.h"
#include "projection_spherical.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(ProjectionSpherical, "")
	PY_CLASS_CONSTRUCTOR_0(ProjectionSpherical)
	PY_CLASS_CONSTRUCTOR_1(ProjectionSpherical, const TPoint3D&)
	PY_CLASS_MEMBER_RW(ProjectionSpherical, position, setPosition)


// --- public --------------------------------------------------------------------------------------

ProjectionSpherical::ProjectionSpherical():
	Projection(),
	position_(0, 0, 0)
{
}


ProjectionSpherical::ProjectionSpherical(const TPoint3D& position) :
	Projection(),
	position_(position)
{
}




/** return position of camera
*/
const TPoint3D& ProjectionSpherical::position() const
{
	return position_;
}



/** set position of camera
*/
void ProjectionSpherical::setPosition(const TPoint3D& position)
{
	position_ = position;
}



// --- protected -----------------------------------------------------------------------------------

const TRay3D ProjectionSpherical::doRay(const TUv& uv, TScalar& pdf) const
{
	const TScalar phi = -TNumTraits::pi * uv.y; // when unwrapped, phi goes down, but v goes up.
	const TScalar z = num::cos(phi);
	const TScalar r = num::sin(phi);
	const TScalar theta = 2 * TNumTraits::pi * uv.x;
	const TScalar x = r * num::cos(theta);
	const TScalar y = r * num::sin(theta);
	pdf = 1 / (4 * TNumTraits::pi);
	return TRay3D(position_, TVector3D(x, y, z), prim::IsAlreadyNormalized());
}



const Projection::TUv ProjectionSpherical::doUv(const TPoint3D& point, TRay3D& ray, TScalar& t) const
{
	if (squaredDistance(point, position_) < num::sqr(tolerance))
	{
		t = 0;
		return TUv();
	}
	ray = TRay3D(position_, point);
	const TVector3D& dir = ray.direction();
	LASS_ASSERT(dir.z >= -1 && dir.z <= 1);
	const TScalar u = num::atan2(dir.x, dir.y) / 2 * TNumTraits::pi;
	const TScalar v = num::acos(dir.z) / -TNumTraits::pi;
	LASS_ASSERT(v >= 0 && v <= 1);
	t = 1;
	return TUv(u, v);
}



// --- private -------------------------------------------------------------------------------------

const TPyObjectPtr ProjectionSpherical::doGetState() const
{
	return python::makeTuple(position_);
}



void ProjectionSpherical::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, position_);
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
