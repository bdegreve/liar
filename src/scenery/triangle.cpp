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
#include "triangle.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(Triangle)
PY_CLASS_CONSTRUCTOR_3(Triangle, TPoint3D, TPoint3D, TPoint3D)



// --- public --------------------------------------------------------------------------------------

Triangle::Triangle(const TPoint3D& a, const TPoint3D& b, const TPoint3D& iC):
    triangle_(a, b, iC)
{
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void Triangle::doIntersect(const kernel::Sample& sample, const kernel::BoundedRay& ray, 
						 kernel::Intersection& result) const
{
    TScalar t;
	const prim::Result hit = prim::intersect(triangle_, ray.unboundedRay(), t, ray.nearLimit());
	if (hit == prim::rOne && ray.inRange(t))
	{
		result = kernel::Intersection(this, t, seNoEvent);
	}
	else
	{
		result = kernel::Intersection::empty();
	}
}



const bool Triangle::doIsIntersecting(const kernel::Sample& sample, 
									const kernel::BoundedRay& ray) const
{
    TScalar t;
	const prim::Result hit = prim::intersect(triangle_, ray.unboundedRay(), t, ray.nearLimit());
	return hit == prim::rOne && ray.inRange(t);
}



void Triangle::doLocalContext(const kernel::Sample& sample, const BoundedRay& ray,
                            const kernel::Intersection& intersection, 
                            kernel::IntersectionContext& result) const
{
    TPoint2D uv;
	TScalar t;
	const prim::Result hit = prim::intersect(
		triangle_, ray.unboundedRay(), uv.x, uv.y, t, ray.nearLimit());
	LASS_ASSERT(hit == prim::rOne && ray.inRange(t));
	LASS_ASSERT(t == intersection.t());

	result.setPoint(ray.point(intersection.t()));
    result.setT(intersection.t());
	result.setUv(uv);
    result.setDPoint_dU(triangle_[1] - triangle_[0]);
    result.setDPoint_dV(triangle_[2] - triangle_[0]);
    result.setNormal(triangle_.plane().normal());
    result.setDNormal_dU(TVector3D());
    result.setDNormal_dV(TVector3D());
}



const bool Triangle::doContains(const kernel::Sample& sample, const TPoint3D& point) const
{
	return false;
}



const TAabb3D Triangle::doBoundingBox() const
{
	return prim::aabb(triangle_);
}



const TScalar Triangle::doArea() const
{
	return triangle_.area();
}



const TPyObjectPtr Triangle::doGetState() const
{
	return python::makeTuple(triangle_[0], triangle_[1], triangle_[2]);
}



void Triangle::doSetState(const TPyObjectPtr& state)
{
	TPoint3D a;
	TPoint3D b;
	TPoint3D c;
	LASS_ENFORCE(python::decodeTuple(state, a, b, c));
	triangle_ = TTriangle3D(a, b, c);
}



// --- free ----------------------------------------------------------------------------------------



// --- python --------------------------------------------------------------------------------------

}

}

// EOF
