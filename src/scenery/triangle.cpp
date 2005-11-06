/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
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
#include "triangle.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(Triangle)
PY_CLASS_CONSTRUCTOR_3(Triangle, TPoint3D, TPoint3D, TPoint3D)



// --- public --------------------------------------------------------------------------------------

Triangle::Triangle(const TPoint3D& iA, const TPoint3D& iB, const TPoint3D& iC):
    SceneObject(&Type),
    triangle_(iA, iB, iC)
{
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void Triangle::doIntersect(const kernel::Sample& iSample, const kernel::BoundedRay& iRay, 
						 kernel::Intersection& oResult) const
{
    TScalar t;
	const prim::Result hit = prim::intersect(triangle_, iRay.unboundedRay(), t, iRay.nearLimit());
	if (hit == prim::rOne && iRay.inRange(t))
	{
		oResult = kernel::Intersection(this, t, seNoEvent);
	}
	else
	{
		oResult = kernel::Intersection::empty();
	}
}



const bool Triangle::doIsIntersecting(const kernel::Sample& iSample, 
									const kernel::BoundedRay& iRay) const
{
    TScalar t;
	const prim::Result hit = prim::intersect(triangle_, iRay.unboundedRay(), t, iRay.nearLimit());
	return hit == prim::rOne && iRay.inRange(t);
}



void Triangle::doLocalContext(const kernel::Sample& iSample, const BoundedRay& iRay,
                            const kernel::Intersection& iIntersection, 
                            kernel::IntersectionContext& oResult) const
{
    TPoint2D uv;
	TScalar t;
	const prim::Result hit = prim::intersect(
		triangle_, iRay.unboundedRay(), uv.x, uv.y, t, iRay.nearLimit());
	LASS_ASSERT(hit == prim::rOne && iRay.inRange(t));
	LASS_ASSERT(t == iIntersection.t());

	oResult.setPoint(iRay.point(iIntersection.t()));
    oResult.setT(iIntersection.t());
	oResult.setUv(uv);
    oResult.setDPoint_dU(triangle_[1] - triangle_[0]);
    oResult.setDPoint_dV(triangle_[2] - triangle_[0]);
    oResult.setNormal(triangle_.plane().normal());
    oResult.setDNormal_dU(TVector3D());
    oResult.setDNormal_dV(TVector3D());
}



const bool Triangle::doContains(const kernel::Sample& iSample, const TPoint3D& iPoint) const
{
	return false;
}



const TAabb3D Triangle::doBoundingBox(const kernel::TimePeriod& iPeriod) const
{
	return prim::aabb(triangle_);
}



// --- free ----------------------------------------------------------------------------------------



// --- python --------------------------------------------------------------------------------------

}

}

// EOF
