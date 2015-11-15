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

#include "scenery_common.h"
#include "box.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_DOC(Box, "a 3D axis aligned box")
PY_CLASS_CONSTRUCTOR_0(Box)
PY_CLASS_CONSTRUCTOR_1(Box, const TAabb3D&)
PY_CLASS_CONSTRUCTOR_2(Box, const TPoint3D&, const TPoint3D&)
PY_CLASS_MEMBER_RW(Box, min, setMin)
PY_CLASS_MEMBER_RW(Box, max, setMax)
PY_CLASS_MEMBER_RW(Box, bounds, setBounds)



// --- public --------------------------------------------------------------------------------------

Box::Box():
	bounds_(TPoint3D(-1, -1, -1), TPoint3D(1, 1, 1))
{
}



Box::Box(const TPoint3D& min, const TPoint3D& max):
	bounds_(min, max)
{
}



Box::Box(const TAabb3D& bounds):
	bounds_(bounds)
{
}



const TPoint3D& Box::min() const
{
	return bounds_.min();
}



void Box::setMin(const TPoint3D& min)
{
	bounds_.setMin(min);
}



const TPoint3D& Box::max() const
{
	return bounds_.max();
}



void Box::setMax(const TPoint3D& max)
{
	bounds_.setMax(max);
}




const TAabb3D& Box::bounds() const
{
	return bounds_;
}



void Box::setBounds(const TAabb3D& bounds)
{
	bounds_ = bounds;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void Box::doIntersect(const Sample&, const BoundedRay& ray, Intersection& result) const
{
	TScalar t;
	const prim::Result hit = prim::intersect(bounds_, ray.unboundedRay(), t, ray.nearLimit());
	if (hit == prim::rOne && ray.inRange(t))
	{
		result = Intersection(this, t, seNoEvent);
	}
	else
	{
		result = Intersection::empty();
	}
}



bool Box::doIsIntersecting(const Sample&, const BoundedRay& ray) const
{
	TScalar t;
	const prim::Result hit = prim::intersect(bounds_, ray.unboundedRay(), t, ray.nearLimit());
	return hit == prim::rOne && ray.inRange(t);
}



namespace impl
{

bool intersectSlab(TScalar min, TScalar max, TScalar support, TScalar direction, prim::XYZ axis, TScalar& tNear, TScalar& tFar, prim::XYZ& axisFar, prim::XYZ& axisNear)
{
	if (direction == 0)
	{
		return support >= min && support <= max;
	}
	
	const TScalar tMin = (min - support) / direction;
	const TScalar tMax = (max - support) / direction;
	if (direction > 0)
	{
		if (tMin > tNear)
		{
			axisNear = axis;
			tNear = tMin;
		}
		if (tMax < tFar)
		{
			axisFar = axis;
			tFar = tMax;
		}
	}
	else
	{
		if (tMax > tNear)
		{
			axisNear = axis;
			tNear = tMax;
		}
		if (tMin < tFar)
		{
			axisFar = axis;
			tFar = tMin;
		}
	}
	return tNear <= tFar;
}


bool intersect(const TAabb3D& box, const BoundedRay& ray, TScalar& t, prim::XYZ& axis)
{
	const TPoint3D& min = box.min();
	const TPoint3D& max = box.max();
	const TPoint3D& support = ray.support();
	const TVector3D& direction = ray.direction();

	TScalar tNear = ray.nearLimit();
	TScalar tFar = TNumTraits::infinity;
	prim::XYZ axisNear, axisFar;

	if (!intersectSlab(min[0], max[0], support[0], direction[0], 0, tNear, tFar, axisNear, axisFar)) return false;
	if (!intersectSlab(min[1], max[1], support[1], direction[1], 1, tNear, tFar, axisNear, axisFar)) return false;
	if (!intersectSlab(min[2], max[2], support[2], direction[2], 2, tNear, tFar, axisNear, axisFar)) return false;

	if (tNear > ray.nearLimit())
	{
		t = tNear;
		axis = axisNear;
		return true;
	}
	if (tFar > ray.nearLimit())
	{
		t = tFar;
		axis = axisFar;
		return true;
	}
	return false;
}

}



void Box::doLocalContext(const Sample&, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const
{
	const TPoint3D center = bounds_.center().affine();
	const TVector3D size = bounds_.size();

	TScalar t = 0;
	prim::XYZ axis;
	const bool LASS_UNUSED(hit) = impl::intersect(bounds_, ray, t, axis);
	LASS_ASSERT(hit && ray.inRange(t) && int(axis) >= 0);
	LASS_ASSERT(t == intersection.t());
	const TPoint3D point = ray.point(intersection.t());

	LASS_ASSERT(center[axis] != point[axis]);
	TVector3D normal = TVector3D(0, 0, 0);
	normal[axis] = point[axis] > center[axis] ? 1 : -1;

	prim::XYZ i = axis + 1;
	prim::XYZ j = axis + 2;
	TVector3D dp_du(0, 0, 0);
	dp_du[i] = 1;
	const TVector3D dp_dv = prim::cross(normal, dp_du);

	TPoint2D uv;
	uv.x = (point[i] - bounds_.min()[i]) / size[i];
	uv.y = (point[j] - bounds_.min()[j]) / size[j];
	if (dp_dv[j] < 0)
	{
		uv.y = 1 - uv.y;
	}

	result.setPoint(point);
	result.setT(t);
	result.setUv(uv);
	result.setDPoint_dU(dp_du);
	result.setDPoint_dV(dp_dv);
	result.setGeometricNormal(normal);
	result.setNormal(normal);
	result.setDNormal_dU(TVector3D());
	result.setDNormal_dV(TVector3D());
}



bool Box::doContains(const Sample&, const TPoint3D& point) const
{
	return bounds_.contains(point);
}




const TAabb3D Box::doBoundingBox() const
{
	return bounds_;
}



TScalar Box::doArea() const
{
	return bounds_.area();
}



TScalar Box::doArea(const TVector3D& normal) const
{
	const TVector3D size = bounds_.size();
	return size.x * size.y * num::abs(normal.z)
		+ size.y * size.z * num::abs(normal.x)
		+ size.z * size.x * num::abs(normal.y);
}



const TPyObjectPtr Box::doGetState() const
{
	return python::makeTuple(bounds_.min(), bounds_.max());
}



void Box::doSetState(const TPyObjectPtr& state)
{
	TPoint3D min;
	TPoint3D max;
	LASS_ENFORCE(python::decodeTuple(state, min, max));
	bounds_ = TAabb3D(min, max);
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
