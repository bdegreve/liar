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

/** @class liar::scenery::MotionRotation
 *  @brief a translation that varies in time.
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_MOTION_ROTATION_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_MOTION_ROTATION_H

#include "scenery_common.h"
#include "../kernel/scene_object.h"

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL MotionRotation: public SceneObject
{
	PY_HEADER(SceneObject)
public:

	MotionRotation(const TSceneObjectPtr& child, const TVector3D& axis, TScalar startAngleRadians, TScalar speedAngleRadians);

	const TSceneObjectPtr& child() const;
	void setChild(const TSceneObjectPtr& child);

	const TVector3D& axis() const;
	void setAxis(const TVector3D& axis);

	TScalar start() const;
	void setStart(TScalar start);

	TScalar speed() const;
	void setSpeed(TScalar speed);

private:

	void doAccept(lass::util::VisitorBase& visitor);

	void doPreProcess(const TSceneObjectPtr& scene, const TimePeriod& period);
	void doIntersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const;
	bool doIsIntersecting(const Sample& sample, const BoundedRay& ray) const;
	void doLocalContext(const Sample& sample, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const;
	void doLocalSpace(TTime time, TTransformation3D& localToWorld) const;
	bool doContains(const Sample& sample, const TPoint3D& point) const;
	const TAabb3D doBoundingBox() const;
	TScalar doArea() const;
	TScalar doArea(const TVector3D& normal) const;
	bool doHasMotion() const { return true; }

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	struct TransformationCache
	{
		TTransformation3D localToWorld;
		TTime time;
		TransformationCache(): time(num::NumTraits<TTime>::qNaN) {}
	};

	TScalar angle(TTime time) const;
	const TTransformation3D& worldToLocal(TTime time) const;

	TSceneObjectPtr child_;
	TAabb3D aabb_;
	TVector3D axis_;
	TScalar start_;
	TScalar speed_;
	mutable util::ThreadLocalVariable<TransformationCache> cache_;
};



}

}

#endif

// EOF
