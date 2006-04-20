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

namespace liar
{
namespace kernel
{

/** do some preprocessing before rendering.
 *
 *  @param iPeriod [in]
 *		timespan covered by render.
 */
inline void SceneObject::preProcess(const TimePeriod& iPeriod) 
{ 
	doPreProcess(iPeriod); 
}



/** intersect object with normal BoundedRay.
 *
 *  @param iSample [in]
 *		sample information
 *	@param iRay [in]
 *		ray to intersect with
 *  @param oResult [out]
 *		information on intersection.
 */
inline void SceneObject::intersect(const Sample& iSample, const BoundedRay& iRay, 
		Intersection& oResult) const
{ 
	doIntersect(iSample, iRay, oResult);
	LASS_ASSERT(!oResult || oResult.object() == this);
}



/** intersect object with a DifferentialRay.
 *
 *  @param iSample [in]
 *		sample information
 *	@param iRay [in]
 *		ray to intersect with
 *  @param oResult [out]
 *		information on intersection.
 *
 *  Intersection with differential ray is equivalent to intersection with its central ray.
 */
inline void SceneObject::intersect(const Sample& iSample, const DifferentialRay& iRay, 
		Intersection& oResult) const
{ 
	doIntersect(iSample, iRay.centralRay(), oResult); 
	LASS_ASSERT(!oResult || oResult.object() == this);
}
	


/** check if object intersects with BoundedRay.
 *
 *  @param iSample [in]
 *		sample information
 *	@param iRay [in]
 *		ray to intersect with
 *
 *  @return
 *		true if ray intersects object, false otherwise
 *
 *  If you're only interested in whether a ray intersects an object, this method may be cheaper
 *  than a full intersection test.
 */
inline const bool SceneObject::isIntersecting(const Sample& iSample, const BoundedRay& iRay) const
{
	return doIsIntersecting(iSample, iRay);
}
	


/** check if object intersects with DifferentialRay.
 *
 *  @param iSample [in]
 *		sample information
 *	@param iRay [in]
 *		ray to intersect with
 *
 *  @return
 *		true if ray intersects object, false otherwise
 *
 *  Equivalent to isIntersecting with its central ray.
 *  If you're only interested in whether a ray intersects an object, this method may be cheaper
 *  than a full intersection test.
 */
inline const bool SceneObject::isIntersecting(const Sample& iSample, 
		const DifferentialRay& iRay) const 
{
	return doIsIntersecting(iSample, iRay.centralRay()); 
}



/** get geometrical information on intersection with BoundedRay
 *
 *  @param iSample [in]
 *		sample information
 *	@param iRay [in]
 *		ray to intersect with
 *  @param iIntersection [in]
 *		the result of a successful intersection is used as input.
 *	@param IntersectionContext [out]
 *		geometrical information on intersection
 */
inline void SceneObject::localContext(const Sample& iSample, const BoundedRay& iRay, 
		const Intersection& iIntersection, IntersectionContext& oResult) const
{
	LASS_ASSERT(iIntersection.object() == this);
    doLocalContext(iSample, iRay, iIntersection, oResult);
    if (shader_)
    {
        oResult.setShader(shader_);
    }
}



/** get geometrical information on intersection with DifferentialRay.
 *
 *  @param iSample [in]
 *		sample information
 *	@param iRay [in]
 *		ray to intersect with
 *  @param iIntersection [in]
 *		the result of a successful intersection is used as input.
 *	@param IntersectionContext [out]
 *		geometrical information on intersection
 *
 *  Also sets screen differentials.
 */
inline void SceneObject::localContext(const Sample& iSample, const DifferentialRay& iRay, 
		const Intersection& iIntersection, IntersectionContext& oResult) const
{
	localContext(iSample, iRay.centralRay(), iIntersection, oResult);
	oResult.setScreenSpaceDifferentials(iRay);
}



/** check if object contains a point.
 *	
 *	@param iSample [in]
 *		sample information
 *	@param iPoint [in]
 *		the point to be checked
 *
 *	@return
 *		true if point is inside object, false if it is not.
 */
inline const bool SceneObject::contains(const Sample& iSample, const TPoint3D& iPoint) const 
{ 
	return doContains(iSample, iPoint); 
}



/** Add local-to-world transformation to transformation object.
 *
 *	@param iTime [in]
 *		exact scene time for motion transformations, to get correct motion blur.
 *	@param ioLocalToWorld [in,out]
 *		transformation from local space to world space.
 */
inline void SceneObject::localSpace(TTime iTime, TTransformation3D& ioLocalToWorld) const 
{ 
	doLocalSpace(iTime, ioLocalToWorld); 
}



/** check if object supports surface sampling.
 *
 *  @return
 *		true if it supports surface sampling, false otherwise.
 *
 *  Surface sampling is necessary for using an object as the surface of an area light.
 */
inline const bool SceneObject::hasSurfaceSampling() const 
{ 
	return doHasSurfaceSampling(); 
}



/** sample surface without target position.
 *
 *  @param iSample [in]
 *		(u,v) sample
 *	@param oNormal [out]
 *		normal of surface at sampled position.
 *  @param oPdf [out]
 *		value of probability density function the position was choosen with.
 *
 *  This is also a fallback for sampleSurface with target position.
 */
inline const TPoint3D SceneObject::sampleSurface(const TVector2D& iSample, TVector3D& oNormal, 
		TScalar& oPdf) const
{
	LASS_ASSERT(hasSurfaceSampling());
	return doSampleSurface(iSample, oNormal, oPdf);
}



/** sample surface with target position.
 *
 *  @param iSample [in]
 *		(u,v) sample
 *  @param iTarget [in]
 *		point being illuminated.
 *	@param oNormal [out]
 *		normal of surface at sampled position.
 *  @param oPdf [out]
 *		value of probability density function the position was choosen with.
 *
 *  This is used to sample a point on an area light to illuminate a target point.  This way
 *	we can avoid sampling points that won't contribute.
 *
 *  Scene objects that do not implement this will fallback on sampleSurface without target 
 *	position.  This is also a fallback for sampleSurface with target position and target normal.
 */
inline const TPoint3D SceneObject::sampleSurface(const TVector2D& iSample, const TPoint3D& iTarget,
		TVector3D& oNormal, TScalar& oPdf) const
{
	return doSampleSurface(iSample, iTarget, oNormal, oPdf);
}



/** sample surface with target position and target normal.
 *
 *  @param iSample [in]
 *		(u,v) sample
 *  @param iTarget [in]
 *		point being illuminated.
 *  @param iTargetNormal [in]
 *		surface normal at iTarget
 *	@param oNormal [out]
 *		normal of surface at sampled position.
 *  @param oPdf [out]
 *		value of probability density function the position was choosen with.
 *
 *  This is used to sample a point on an area light to illuminate a target point.  This way
 *	we can avoid sampling points that won't contribute.
 *
 *  Scene objects that do not implement this will fallback on sampleSurface with only a target 
 *	point.
 */
inline const TPoint3D SceneObject::sampleSurface(const TVector2D& iSample, const TPoint3D& iTarget,
		const TVector3D& iTargetNormal, TVector3D& oNormal, TScalar& oPdf) const
{
	return doSampleSurface(iSample, iTarget, iTargetNormal, oNormal, oPdf);
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



/** check whether object moves over time.
 *
 *  @return
 *		true if object moves, false otherwise.
 */
inline const bool SceneObject::hasMotion() const 
{ 
	return doHasMotion(); 
}



/** get total surface area of object.
 *
 *  @return
 *		total area.
 */
inline const TScalar SceneObject::area() const 
{ 
	return doArea(); 
}

}

}

// EOF
