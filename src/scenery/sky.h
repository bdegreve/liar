/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
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
 *  http://liar.bramz.org
 */

/** @class liar::scenery::Sphere
 *  @brief a nice spherical object
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_SKY_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_SKY_H

#include "scenery_common.h"
#include "../kernel/scene_object.h"

#include <lass/prim/sphere_3d.h>

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL Sky: public SceneObject
{
    PY_HEADER(SceneObject)
public:

	Sky();
	Sky(TScalar radius);

private:

    LASS_UTIL_VISITOR_DO_ACCEPT
    
	void doIntersect(const Sample& sample, const BoundedRay& ray, 
		Intersection& result) const;
	const bool doIsIntersecting(const Sample& sample, const BoundedRay& ray) const;
	void doLocalContext(const Sample& sample, const BoundedRay& ray,
		const Intersection& intersection, IntersectionContext& result) const;
	const bool doContains(const Sample& sample, const TPoint3D& point) const;
	const TAabb3D doBoundingBox() const;
	const TScalar doArea() const;

	const bool doHasSurfaceSampling() const;
	const TPoint3D doSampleSurface(const TPoint2D& sample, TVector3D& normal,
		TScalar& pdf) const;
	const TPoint3D doSampleSurface(const TPoint2D& sample, const TPoint3D& target,
		TVector3D& normal,	TScalar& pdf) const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	TScalar radius_;
};



}

}

#endif

// EOF
