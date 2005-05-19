/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LIAR isn't a raytracer
 *  Copyright (C) 2004-2005  Bram de Greve
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
#include "sphere.h"
#include <lass/meta/type_list.h>

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(Sphere)
PY_CLASS_CONSTRUCTOR_0(Sphere)
PY_CLASS_CONSTRUCTOR_2(Sphere, TPoint3D, TScalar)
PY_CLASS_MEMBER_RW(Sphere, "center", center, setCenter)
PY_CLASS_MEMBER_RW(Sphere, "radius", radius, setRadius)



// --- public --------------------------------------------------------------------------------------

Sphere::Sphere():
	SceneObject(&Type),
	sphere_(TPoint3D(0, 0, 0), 1),
	invRadius_(1)
{
}



Sphere::Sphere(const TPoint3D& iCenter, TScalar iRadius):
    SceneObject(&Type),
    sphere_(iCenter, iRadius),
    invRadius_(num::inv(iRadius))
{
}



const TPoint3D& Sphere::center() const
{
    return sphere_.center();
}



void Sphere::setCenter(const TPoint3D& iCenter)
{
	sphere_.center() = iCenter;
}



const TScalar Sphere::radius() const
{
    return sphere_.radius();
}



void Sphere::setRadius(TScalar iRadius)
{
	sphere_.radius() = iRadius;
	invRadius_ = num::inv(iRadius);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void Sphere::doIntersect(const TRay3D& iRay, kernel::Intersection& oResult) const
{
    TScalar tNear;
    TScalar tFar;
    prim::Result hit = prim::intersect(iRay, sphere_, tNear, tFar);
    if (hit == prim::rNone)
    {
        oResult = kernel::Intersection::empty();
    }
    else
    {
        oResult = kernel::Intersection(this, tNear);
    }
}



const bool Sphere::doIsIntersecting(const TRay3D& iRay, TScalar iMaxT,
									const SceneObject* iExcludeA, const SceneObject* iExcludeB) const
{
    TScalar tNear;
    TScalar tFar;
    prim::Result hit = prim::intersect(iRay, sphere_, tNear, tFar);
	return (hit != prim::rNone && tNear < iMaxT);
}



void Sphere::doLocalContext(const TRay3D& iRay, 
                            const kernel::Intersection& iIntersection, 
                            kernel::IntersectionContext& oResult) const
{
    oResult.setT(iIntersection.t());
    oResult.setPoint(iRay.point(iIntersection.t()));
    
    TVector3D normal = oResult.point() - sphere_.center();
    TVector3D tangentU = TVector3D(-normal.y, normal.x, 0);
    TVector3D tangentV = cross(normal, tangentU);
    normal *= invRadius_;
    tangentU.normalize();
    tangentV.normalize();
    oResult.setNormal(normal);
    oResult.setTangentU(tangentU);
    oResult.setTangentV(tangentV);
    
    LASS_ASSERT(normal.z >= -TNumTraits::one && normal.z <= TNumTraits::one);
    const TScalar theta = num::acos(normal.z);
    const TScalar phi = num::atan2(normal.x, normal.y);
    
    oResult.setU(theta / TNumTraits::pi);
    oResult.setV(phi / (2 * TNumTraits::pi));
}



const TAabb3D Sphere::doBoundingBox() const
{
    TScalar radius = sphere_.radius();
    TVector3D extent = TVector3D(radius, radius, radius);
    return TAabb3D(sphere_.center() - extent, sphere_.center() + extent);
}



// --- free ----------------------------------------------------------------------------------------



// --- python --------------------------------------------------------------------------------------

}

}

// EOF
