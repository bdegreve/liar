/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2006  Bram de Greve
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
 *  http://liar.sourceforge.net
 */

#include "scenery_common.h"
#include "plane.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(Plane)
PY_CLASS_CONSTRUCTOR_0(Plane)
PY_CLASS_CONSTRUCTOR_2(Plane, TVector3D, TScalar)
PY_CLASS_MEMBER_RW(Plane, "normal", normal, setNormal)
PY_CLASS_MEMBER_RW(Plane, "d", d, setD)

// --- public --------------------------------------------------------------------------------------

Plane::Plane():
	plane_(TVector3D(0, 0, 1), 0)
{
}



Plane::Plane(const TVector3D& i_normal, TScalar i_d):
    plane_(i_normal, i_d)
{
}



const TVector3D& Plane::normal() const
{
    return plane_.normal();
}



void Plane::setNormal(const TVector3D& normal)
{
	plane_ = TPlane3D(normal, plane_.d());
}



const TScalar Plane::d() const
{
    return plane_.d();
}



void Plane::setD(TScalar iD)
{
	plane_ = TPlane3D(plane_.normal(), iD);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void Plane::doIntersect(const Sample& sample, const BoundedRay& ray, 
						Intersection& result) const
{
    TScalar t;
    const prim::Result hit = prim::intersect(plane_, ray.unboundedRay(), t, ray.nearLimit());
	if (hit == prim::rOne && ray.inRange(t)) 
    {
		result = Intersection(this, t, seNoEvent);
    }
    else
    {
        result = Intersection::empty();
    }
}



const bool Plane::doIsIntersecting(const Sample& sample, 
								   const BoundedRay& ray) const
{
    TScalar t;
	const prim::Result hit = prim::intersect(plane_, ray.unboundedRay(), t, ray.nearLimit());
	return hit == prim::rOne && ray.inRange(t);
}


void Plane::doLocalContext(const Sample& sample, const BoundedRay& ray,
                           const Intersection& intersection, 
                           IntersectionContext& result) const
{
    result.setPoint(ray.point(intersection.t()));
    TVector3D dPoint_dU;
    TVector3D dPoint_dV;
    plane_.getDirections(dPoint_dU, dPoint_dV);
    result.setDPoint_dU(dPoint_dU);
    result.setDPoint_dV(dPoint_dV);

    result.setNormal(plane_.normal());
    result.setDNormal_dU(TVector3D());
    result.setDNormal_dV(TVector3D());
	
	result.setGeometricNormal(result.normal());

	result.setUv(plane_.uv(result.point()));
    result.setT(intersection.t());
}



const bool Plane::doContains(const Sample& sample, const TPoint3D& point) const
{
	return false;
}



const TAabb3D Plane::doBoundingBox() const
{
    const TVector3D normal = plane_.normal();
    TVector3D extent;
    if (normal.x != 0)
    {
        extent.y = TNumTraits::infinity;
        extent.z = TNumTraits::infinity;
    }
    if (normal.y != 0)
    {
        extent.z = TNumTraits::infinity;
        extent.x = TNumTraits::infinity;
    }
    if (normal.z != 0)
    {
        extent.x = TNumTraits::infinity;
        extent.y = TNumTraits::infinity;
    }
    return TAabb3D(TPoint3D(-extent), TPoint3D(+extent));
}



const TScalar Plane::doArea() const
{
	return TNumTraits::infinity;
}



const TPyObjectPtr Plane::doGetState() const
{
	return python::makeTuple(plane_.normal(), plane_.d());
}



void Plane::doSetState(const TPyObjectPtr& state)
{
	TVector3D normal;
	TScalar d;
	LASS_ENFORCE(python::decodeTuple(state, normal, d));
	plane_ = TPlane3D(normal, d);
}
    


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
