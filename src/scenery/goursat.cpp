/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2024  Bram de Greve (bramz@users.sourceforge.net)
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
PY_CLASS_MEMBER_RW(Goursat, a, setA)
PY_CLASS_MEMBER_RW(Goursat, b, setB)
PY_CLASS_MEMBER_RW(Goursat, c, setC)
PY_CLASS_MEMBER_RW(Goursat, parameters, setParameters)
PY_CLASS_MEMBER_RW(Goursat, parameterSpeed, setParameterSpeed)
PY_CLASS_MEMBER_RW(Goursat, timeOffset, setTimeOffset)
PY_CLASS_MEMBER_RW(Goursat, useSphereTracing, setSphereTracing)

Goursat::Goursat():
    abc0_(0, -5, 11.8f),
    dAbc_(0, 0, 0),
    t0_(0),
    useSphereTracing_(true)
{
}


Goursat::Goursat(TScalar a, TScalar b, TScalar c):
    abc0_(a, b, c),
    dAbc_(0, 0, 0),
    t0_(0),
    useSphereTracing_(true)
{
}


const TPoint3D& Goursat::parameters() const
{
    return abc0_;
}


void Goursat::setParameters(const TPoint3D& abc0)
{
    abc0_ = abc0;
}


const TVector3D& Goursat::parameterSpeed() const
{
    return dAbc_;
}


void Goursat::setParameterSpeed(const TVector3D& dAbc)
{
    dAbc_ = dAbc;
}


TTime Goursat::timeOffset() const
{
    return t0_;
}


void Goursat::setTimeOffset(TTime timeOffset)
{
    t0_ = timeOffset;
}


TScalar Goursat::a() const
{
    return abc0_.x;
}


TScalar Goursat::b() const
{
    return abc0_.y;
}


TScalar Goursat::c() const
{
    return abc0_.z;
}


void Goursat::setA(TScalar a)
{
    abc0_.x = a;
}


void Goursat::setB(TScalar b)
{
    abc0_.y = b;
}


void Goursat::setC(TScalar c)
{
    abc0_.z = c;
}


bool Goursat::useSphereTracing() const
{
    return useSphereTracing_;
}


void Goursat::setSphereTracing(bool enabled)
{
    useSphereTracing_ = enabled;
}


void Goursat::doIntersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const
{
    const TPoint3D abc = this->abc(sample.time());

    TAabb3D bound = boundingBox();

    TScalar tMin;
    if (prim::intersect(bound, ray.unboundedRay(), tMin, ray.nearLimit()) == prim::rNone)
    {
        result = Intersection::empty();
        return;
    }
    TScalar tMax;
    if (prim::intersect(bound, ray.unboundedRay(), tMax, tMin) == prim::rNone)
    {
        tMax = tMin;
        tMin = ray.nearLimit();
    }
    if (tMin > ray.farLimit())
    {
        result = Intersection::empty();
        return;
    }
    tMax = std::min(tMax, ray.farLimit());
    LASS_ASSERT(tMin >= ray.nearLimit() && tMax > tMin);

    const TScalar tStep = static_cast<TScalar>(.1);
    const TScalar tTolerance = 2 * liar::tolerance;

    if (useSphereTracing_)
    {
        TScalar t1 = tMin;
        TPoint3D p1 = ray.point(t1);
        TScalar v1 = potential(p1, abc);
        while (t1 < tMax)
        {
            const TScalar g1 = gradient(p1, abc).norm();
            const TScalar dt = g1 > 0 ? (num::abs(v1 / g1) / 2) : 0;
            /*
            TScalar t2 = t1 + dt;
            TPoint3D p2 = ray.point(t2);
            TScalar v2 = potential(p2, abc);
            if ( dt < tolerance )
            {
                const SolidEvent event = v2 <= v1 ? seEntering : seLeaving;
                result = Intersection(this, t2, event);
                return;
            }
            /*/
            TScalar t2 = t1 + std::max(dt, tStep);
            TPoint3D p2 = ray.point(t2);
            TScalar v2 = potential(p2, abc);
            if (v1 * v2 <= 0)
            {
                size_t iterations = 100;
                while (num::abs(t2 - t1) > tTolerance)
                {
                    const TScalar t3 = t2 - v2 * (t2 - t1) / (v2 - v1);
                    t1 = t2;
                    t2 = t3;
                    v1 = v2;
                    v2 = potential(ray.point(t2), abc);
                    if (--iterations == 0)
                    {
                        result = Intersection::empty();
                        return;
                    }
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
            /**/
            t1 = t2;
            p1 = p2;
            v1 = v2;
        }
        result = Intersection::empty();

    }
    else
    {
        TScalar t1 = tMin;
        TScalar v1 = potential(ray.point(t1), abc);
        while (t1 < tMax)
        {
            TScalar t2 = t1 + tStep;
            TScalar v2 = potential(ray.point(t2), abc);
            if (v1 * v2 <= 0)
            {
                size_t iterations = 100;
                while (num::abs(t2 - t1) > tTolerance)
                {
                    const TScalar t3 = t2 - v2 * (t2 - t1) / (v2 - v1);
                    t1 = t2;
                    t2 = t3;
                    v1 = v2;
                    v2 = potential(ray.point(t2), abc);
                    if (--iterations == 0)
                    {
                        result = Intersection::empty();
                        return;
                    }
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

}




bool Goursat::doIsIntersecting(const Sample& sample, const BoundedRay& ray) const
{
    Intersection temp;
    this->intersect(sample, ray, temp);
    return !temp.isEmpty() && temp.t() > ray.nearLimit();
}



void Goursat::doLocalContext(const Sample& sample, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const
{
    const TScalar t = intersection.t();
    const TPoint3D point = ray.point(t);
    result.setT(t);
    result.setPoint(point);

    const TVector3D g = gradient(point, abc(sample.time()) );
    result.setNormal(-g.normal());

    result.setUv(TPoint2D());
    result.setDPoint_dU(TVector3D());
    result.setDPoint_dV(TVector3D());
    result.setDNormal_dU(TVector3D());
    result.setDNormal_dV(TVector3D());
}



bool Goursat::doContains(const Sample& sample, const TPoint3D& point) const
{
    return potential(point, abc(sample.time()) ) <= 0;
}



const TAabb3D Goursat::doBoundingBox() const
{
    TScalar s = 100; // 2.27;
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



bool Goursat::doHasMotion() const
{
    return !dAbc_.isZero();
}



const TPyObjectPtr Goursat::doGetState() const
{
    return python::makeTuple(abc0_, dAbc_, t0_, useSphereTracing_);
}



void Goursat::doSetState(const TPyObjectPtr& state)
{
    LASS_ENFORCE(python::decodeTuple(state, abc0_, dAbc_, t0_, useSphereTracing_));
}


inline TPoint3D Goursat::abc(TTime t) const
{
    return abc0_ + static_cast<TScalar>(t - t0_) * dAbc_;
}


inline TScalar Goursat::potential(const TPoint3D& point, const TPoint3D& abc) const
{
    const TScalar x2 = num::sqr(point.x);
    const TScalar y2 = num::sqr(point.y);
    const TScalar z2 = num::sqr(point.z);
    return num::sqr(x2) + num::sqr(y2) + num::sqr(z2) + abc.x * num::sqr(x2 + y2 + z2) + abc.y * (x2 + y2 + z2) + abc.z;
}


inline TVector3D Goursat::gradient(const TPoint3D& point, const TPoint3D& abc) const
{
    //*
    const TScalar x2 = num::sqr(point.x);
    const TScalar y2 = num::sqr(point.y);
    const TScalar z2 = num::sqr(point.z);
    const TScalar alpha = 4 * abc.x * (x2 + y2 + z2) + 2 * abc.y;
    return TVector3D(
        (4 * x2 + alpha) * point.x,
        (4 * y2 + alpha) * point.y,
        (4 * z2 + alpha) * point.z
        );
    /*/
    const TScalar dt = 0.1;
    const TVector3D dx(dt, 0, 0);
    const TVector3D dy(0, dt, 0);
    const TVector3D dz(0, 0, dt);
    return TVector3D(
        potential( point + dx, abc ) - potential( point - dx, abc ),
        potential( point + dy, abc ) - potential( point - dy, abc ),
        potential( point + dz, abc ) - potential( point - dz, abc ) ) / ( 2 * dt );
    /**/
}

}
}
