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

/** @class liar::scenery::Plane
 *  @brief planar object of infinite size
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_PLANE_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_PLANE_H

#include "scenery_common.h"
#include "../kernel/scene_object.h"

#include <lass/prim/plane_3d.h>

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL Plane: public SceneObject
{
	PY_HEADER(SceneObject)
public:

	Plane();
	Plane(const TVector3D& normal, TScalar d);

	const TVector3D& normal() const;
	TScalar d() const;
	const TPoint3D& support() const;
	const TVector3D& directionU() const;
	const TVector3D& directionV() const;

	void setNormal(const TVector3D& normal);
	void setD(TScalar d);
	void setSupport(const TPoint3D& support);
	void setDirectionU(const TVector3D& directionU);
	void setDirectionV(const TVector3D& directionV);

	void setDirections(const TVector3D& directionU, const TVector3D& directionV);

private:

	typedef prim::Plane3D<TScalar, prim::Combined> TPlane3D;

	LASS_UTIL_VISITOR_DO_ACCEPT

	void doIntersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const override;
	bool doIsIntersecting(const Sample& sample, const BoundedRay& ray) const override;
	void doLocalContext(const Sample& sample, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const override;
	bool doContains(const Sample& sample, const TPoint3D& point) const override;
	const TAabb3D doBoundingBox() const override;
	TScalar doArea() const override;
	TScalar doArea(const TVector3D& normal) const override;

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

	TPlane3D plane_;
};



}

}

#endif

// EOF
