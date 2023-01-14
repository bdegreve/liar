/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2023  Bram de Greve (bramz@users.sourceforge.net)
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

namespace liar
{
namespace kernel
{

/** do some preprocessing before rendering.
 *
 *	@param scene [in]
 *		pointer to the entire scene.
 *  @param period [in]
 *		timespan covered by render.
 *
 *	@remark you can't rely on the scene being preprocessed when this function is called,
 *		since this function is called at the very same monent scene _is_ preprocessed!
 */
inline void SceneObject::preProcess(const TSceneObjectPtr& scene, const TimePeriod& period)
{
	doPreProcess(scene, period);
}



/** intersect object with normal BoundedRay.
 *
 *  @param sample [in]
 *		sample information
 *	@param ray [in]
 *		ray to intersect with
 *  @param result [out]
 *		information on intersection.
 */
inline void SceneObject::intersect(const Sample& sample, const BoundedRay& ray,
		Intersection& result) const
{
	doIntersect(sample, ray, result);
	LASS_ASSERT(!result || result.object() == this);
}



/** intersect object with a DifferentialRay.
 *
 *  @param sample [in]
 *		sample information
 *	@param ray [in]
 *		ray to intersect with
 *  @param result [out]
 *		information on intersection.
 *
 *  Intersection with differential ray is equivalent to intersection with its central ray.
 */
inline void SceneObject::intersect(const Sample& sample, const DifferentialRay& ray,
		Intersection& result) const
{
	doIntersect(sample, ray.centralRay(), result);
	if (result.t() > ray.farLimit())
	{
		result = Intersection::empty();
	}
	LASS_ASSERT(!result || result.object() == this);
}



/** check if object intersects with BoundedRay.
 *
 *  @param sample [in]
 *		sample information
 *	@param ray [in]
 *		ray to intersect with
 *
 *  @return
 *		true if ray intersects object, false otherwise
 *
 *  If you're only interested in whether a ray intersects an object, this method may be cheaper
 *  than a full intersection test.
 */
inline bool SceneObject::isIntersecting(const Sample& sample, const BoundedRay& ray) const
{
	return doIsIntersecting(sample, ray);
}



/** check if object intersects with DifferentialRay.
 *
 *  @param sample [in]
 *		sample information
 *	@param ray [in]
 *		ray to intersect with
 *
 *  @return
 *		true if ray intersects object, false otherwise
 *
 *  Equivalent to isIntersecting with its central ray.
 *  If you're only interested in whether a ray intersects an object, this method may be cheaper
 *  than a full intersection test.
 */
inline bool SceneObject::isIntersecting(const Sample& sample, const DifferentialRay& ray) const
{
	return doIsIntersecting(sample, ray.centralRay());
}



/** get geometrical information on intersection with BoundedRay
 *
 *  @param sample [in]
 *		sample information
 *	@param ray [in]
 *		ray to intersect with
 *  @param intersection [in]
 *		the result of a successful intersection is used as input.
 *	@param IntersectionContext [out]
 *		geometrical information on intersection
 */
inline void SceneObject::localContext(const Sample& sample, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const
{
	LASS_ASSERT(intersection.object() == this);
	doLocalContext(sample, ray, intersection, result);
	if (!result.shader() || this->isOverridingShader_)
	{
		result.setShader(shader_);
	}
	if (!result.interior() || this->isOverridingInterior_)
	{
		result.setInterior(interior_);
		result.setSolidEvent(intersection.solidEvent());
	}
}



/** check if object contains a point.
 *
 *	@param sample [in]
 *		sample information
 *	@param point [in]
 *		the point to be checked
 *
 *	@return
 *		true if point is inside object, false if it is not.
 */
inline bool SceneObject::contains(const Sample& sample, const TPoint3D& point) const
{
	return doContains(sample, point);
}



/** Add local-to-world transformation to transformation object.
 *
 *	@param time [in]
 *		exact scene time for motion transformations, to get correct motion blur.
 *	@param localToWorld [in,out]
 *		transformation from local space to world space.
 */
inline void SceneObject::localSpace(TTime time, TTransformation3D& localToWorld) const
{
	doLocalSpace(time, localToWorld);
}



/** check if object supports surface sampling.
 *
 *  @return
 *		true if it supports surface sampling, false otherwise.
 *
 *  Surface sampling is necessary for using an object as the surface of an area light.
 */
inline bool SceneObject::hasSurfaceSampling() const
{
	return doHasSurfaceSampling();
}



/** sample surface without target position.
 *
 *  @param sample [in]
 *		(u,v) sample
 *	@param normal [out]
 *		normal of surface at sampled position.
 *  @param pdf [out]
 *		value of probability density function the position was choosen with.
 *
 *  This is also a fallback for sampleSurface with target position.
 */
inline const TPoint3D SceneObject::sampleSurface(const TPoint2D& sample, TVector3D& normal,
		TScalar& pdf) const
{
	LASS_ASSERT(hasSurfaceSampling());
	return doSampleSurface(sample, normal, pdf);
}



/** sample surface with target position.
 *
 *  @param sample [in]
 *		(u,v) sample
 *  @param target [in]
 *		point being illuminated.
 *	@param normal [out]
 *		normal of surface at sampled position.
 *  @param pdf [out]
 *		value of probability density function the position was choosen with.
 *
 *  This is used to sample a point on an area light to illuminate a target point.  This way
 *	we can avoid sampling points that won't contribute.
 *
 *  Scene objects that do not implement this will fallback on sampleSurface without target
 *	position.  This is also a fallback for sampleSurface with target position and target normal.
 */
inline const TPoint3D SceneObject::sampleSurface(const TPoint2D& sample, const TPoint3D& target, TVector3D& normal, TScalar& pdf) const
{
	return doSampleSurface(sample, target, normal, pdf);
}



/** sample surface with target position and target normal.
 *
 *  @param sample [in]
 *		(u,v) sample
 *  @param target [in]
 *		point being illuminated.
 *  @param targetNormal [in]
 *		surface normal at target
 *	@param normal [out]
 *		normal of surface at sampled position.
 *  @param pdf [out]
 *		value of probability density function the position was choosen with.
 *
 *  This is used to sample a point on an area light to illuminate a target point.  This way
 *	we can avoid sampling points that won't contribute.
 *
 *  Scene objects that do not implement this will fallback on sampleSurface with only a target
 *	point.
 */
inline const TPoint3D SceneObject::sampleSurface(const TPoint2D& sample, const TPoint3D& target, const TVector3D& targetNormal, TVector3D& normal, TScalar& pdf) const
{
	return doSampleSurface(sample, target, targetNormal, normal, pdf);
}



/** sample surface with viewing direction
 *
 *  @param sample [in]
 *		(u,v) sample
 *  @param view [in]
 *		direction from which you intersect the object (from infinity)
 *	@param normal [out]
 *		normal of surface at sampled position.
 *  @param pdf [out]
 *		value of probability density function the position was choosen with.
 *
 *  This is used to sample a point on an area light to illuminate a target point.  This way
 *	we can avoid sampling points that won't contribute.
 *
 *  Scene objects that do not implement this will fallback on sampleSurface with only a target
 *	point.
 */
inline const TPoint3D SceneObject::sampleSurface(const TPoint2D& sample, const TVector3D& viewDirection, TVector3D& normal, TScalar& pdf) const
{
	return doSampleSurface(sample, viewDirection, normal, pdf);
}


/**
 */
inline TScalar SceneObject::angularPdf(const Sample& sample, const TRay3D& ray, BoundedRay& shadowRay, TVector3D& normal) const
{
	return doAngularPdf(sample, ray, shadowRay, normal);
}



/** get AABB of object
 *
 *  @return
 *		AABB of object
 */
inline const TAabb3D SceneObject::boundingBox() const
{
	return doBoundingBox();
}



/** get bounding sphere of object
 *
 *  @return
 *		bounding sphere of object
 */
inline const TSphere3D SceneObject::boundingSphere() const
{
	return doBoundingSphere();
}



/** check whether object moves over time.
 *
 *  @return
 *		true if object moves, false otherwise.
 */
inline bool SceneObject::hasMotion() const
{
	return doHasMotion();
}



/** get total surface area of object.
 *
 *  @return
 *		total area.
 */
inline TScalar SceneObject::area() const
{
	return doArea();
}



/** get area of object surface projected according normal.
 *
 *  @return
 *		total area.
 */
inline TScalar SceneObject::area(const TVector3D& normal) const
{
	return doArea(normal);
}



}

}

// EOF
