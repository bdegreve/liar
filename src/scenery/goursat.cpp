/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2012  Bram de Greve (bramz@users.sourceforge.net)
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
#include "goursat.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_DOC(Goursat, "implicit distance field thing")
PY_CLASS_CONSTRUCTOR_0(Goursat)
PY_CLASS_CONSTRUCTOR_3(Goursat, TScalar, TScalar, TScalar)

Goursat::Goursat():
    a_(0),
    b_(-5),
    c_(11.8f)
{
}

Goursat::Goursat(TScalar a, TScalar b, TScalar c):
    a_(a),
    b_(b),
    c_(c)
{
}

void Goursat::doIntersect(const Sample&, const BoundedRay& ray, Intersection& result) const
{
    TAabb3D bound = boundingBox();
    TScalar tMin, tMax;
    if (prim::intersect(bound, ray.unboundedRay(), tMin, ray.nearLimit()) == prim::rNone)
    {
        result = Intersection::empty();
        return;
    }
    if (prim::intersect(bound, ray.unboundedRay(), tMax, tMin) == prim::rNone)
    {
        tMax = tMin;
        tMin = ray.nearLimit();
    }
    if (tMin >= ray.farLimit())
    {
        result = Intersection::empty();
        return;
    }
    tMax = std::min(tMax, ray.farLimit());
    LASS_ASSERT(tMin >= ray.nearLimit() && tMax > tMin);

    const TScalar tStep = .1;
    const TScalar epsilon = 2 * liar::tolerance;
    TScalar t1 = tMin;
    TScalar v1 = potential(ray.point(t1));
    while (t1 < tMax)
    {
        TScalar t2 = t1 + tStep;
        TScalar v2 = potential(ray.point(t2));
        if (v1 * v2 <= 0)
        {
            while (num::abs(t2 - t1) > epsilon)
            {
                const TScalar t3 = t2 - v2 * (t2 - t1) / (v2 - v1);
                t1 = t2;
                t2 = t3;
                v1 = v2;
                v2 = potential(ray.point(t2));
            }
            const TScalar t = (t1 + t2) / 2;
            if (t > tMin && t < tMax)
            {
                const SolidEvent event = (t2 - t1) * (v2 - v1) < 0 ? seEntering : seLeaving;
                result = Intersection(this, t, event);
            }
            else
            {
                result = Intersection::empty();
            }
            return;
        }
        t1 = t2;
    }
    result = Intersection::empty();
}




bool Goursat::doIsIntersecting(const Sample& sample, const BoundedRay& ray) const
{
    Intersection temp;
    this->intersect(sample, ray, temp);
    return !temp.isEmpty() && temp.t() > ray.nearLimit();
}



void Goursat::doLocalContext(const Sample&, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const
{
	const TScalar t = intersection.t();
	const TPoint3D point = ray.point(t);
	result.setT(t);
	result.setPoint(point);

    const TVector3D g = gradient(point);
    result.setNormal(g.normal());

    result.setUv(TPoint2D());
	result.setDPoint_dU(TVector3D());
	result.setDPoint_dV(TVector3D());
	result.setDNormal_dU(TVector3D());
	result.setDNormal_dV(TVector3D());
}



bool Goursat::doContains(const Sample&, const TPoint3D& point) const
{
    return potential(point) <= 0;
}



const TAabb3D Goursat::doBoundingBox() const
{
    TScalar s = 2.3;
	return TAabb3D(TPoint3D(-s, -s, -s), TPoint3D(s, s, s));
}



TScalar Goursat::doArea() const
{
	return 0;
}



TScalar Goursat::doArea(const TVector3D&) const
{
	return 0;
}



const TPyObjectPtr Goursat::doGetState() const
{
	return python::makeTuple();
}


void Goursat::doSetState(const TPyObjectPtr& state)
{
	LASS_ENFORCE(python::decodeTuple(state));
}


TScalar Goursat::potential(const TPoint3D& point) const
{
    const TScalar x2 = num::sqr(point.x);
    const TScalar y2 = num::sqr(point.y);
    const TScalar z2 = num::sqr(point.z);
    return num::sqr(x2) + num::sqr(y2) + num::sqr(z2) + a_ * num::sqr(x2 + y2 + z2) + b_ * (x2 + y2 + z2) + c_;
}


TVector3D Goursat::gradient(const TPoint3D& point) const
{
    const TScalar x2 = num::sqr(point.x);
    const TScalar y2 = num::sqr(point.y);
    const TScalar z2 = num::sqr(point.z);
    return TVector3D(
        (2 * x2 + a_ * 2 * (x2 + y2 + z2) + b_) * 2 * point.x,
        (2 * y2 + a_ * 2 * (x2 + y2 + z2) + b_) * 2 * point.y,
        (2 * z2 + a_ * 2 * (x2 + y2 + z2) + b_) * 2 * point.z
        );
}

}
}
