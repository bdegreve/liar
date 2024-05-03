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

/** @class liar::scenery::Goursat
 *  @brief Goursat's surface, an implicit surface.
 *  @author Bram de Greve [Bramz]
 *
 *  x**4 + y**4 + z**4 + a * (x**2 + y**2 + z**2) ** 2 + b * (x**2 + y**2 + z**2) + c == 0
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_GOURSAT_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_GOURSAT_H

#include "scenery_common.h"
#include "../kernel/scene_object.h"

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL Goursat: public SceneObject
{
    PY_HEADER(SceneObject)
public:
    Goursat();
    Goursat(TScalar a, TScalar b, TScalar c);

    TScalar a() const;
    TScalar b() const;
    TScalar c() const;
    void setA(TScalar a);
    void setB(TScalar b);
    void setC(TScalar c);

    const TPoint3D& parameters() const;
    void setParameters(const TPoint3D& parameters);
    const TVector3D& parameterSpeed() const;
    void setParameterSpeed(const TVector3D& speed);
    TTime timeOffset() const;
    void setTimeOffset(TTime timeOffset);

    bool useSphereTracing() const;
    void setSphereTracing(bool enabled);

private:

    LASS_UTIL_VISITOR_DO_ACCEPT

    void doIntersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const override;
    bool doIsIntersecting(const Sample& sample, const BoundedRay& ray) const override;
    void doLocalContext(const Sample& sample, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const override;
    bool doContains(const Sample& sample, const TPoint3D& point) const override;
    const TAabb3D doBoundingBox() const override;
    TScalar doArea() const override;
    TScalar doArea(const TVector3D& normal) const override;
    bool doHasMotion() const override;

    const TPyObjectPtr doGetState() const override;
    void doSetState(const TPyObjectPtr& state) override;

    TPoint3D abc(TTime t) const;
    TScalar potential(const TPoint3D& point, const TPoint3D& abc) const;
    TVector3D gradient(const TPoint3D& point, const TPoint3D& abc) const;

    TPoint3D abc0_;
    TVector3D dAbc_;
    TTime t0_;
    bool useSphereTracing_;
};

}
}

#endif
