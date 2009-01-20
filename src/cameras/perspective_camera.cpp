/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
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

#include "cameras_common.h"
#include "perspective_camera.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace cameras
{


PY_DECLARE_CLASS(PerspectiveCamera)
PY_CLASS_CONSTRUCTOR_0(PerspectiveCamera)
PY_CLASS_MEMBER_RW(PerspectiveCamera, position, setPosition)
PY_CLASS_MEMBER_RW(PerspectiveCamera, sky, setSky)
PY_CLASS_MEMBER_RW(PerspectiveCamera, direction, setDirection)
PY_CLASS_METHOD(PerspectiveCamera, lookAt)
PY_CLASS_MEMBER_RW(PerspectiveCamera, right, setRight)
PY_CLASS_MEMBER_RW(PerspectiveCamera, down, setDown)
PY_CLASS_MEMBER_RW(PerspectiveCamera, width, setWidth)
PY_CLASS_MEMBER_RW(PerspectiveCamera, height, setHeight)
PY_CLASS_MEMBER_RW(PerspectiveCamera, aspectRatio, setAspectRatio)
PY_CLASS_MEMBER_RW(PerspectiveCamera, focalLength, setFocalLength)
PY_CLASS_MEMBER_RW(PerspectiveCamera, fovAngle, setFovAngle)
PY_CLASS_MEMBER_RW(PerspectiveCamera, nearLimit, setNearLimit)
PY_CLASS_MEMBER_RW(PerspectiveCamera, farLimit, setFarLimit)
PY_CLASS_MEMBER_RW(PerspectiveCamera, focusDistance, setFocusDistance)
PY_CLASS_METHOD(PerspectiveCamera, focusAt)
PY_CLASS_MEMBER_RW(PerspectiveCamera, fNumber, setFNumber)
PY_CLASS_MEMBER_RW(PerspectiveCamera, lensRadius, setLensRadius)
PY_CLASS_MEMBER_RW(PerspectiveCamera, shutterOpenDelta, setShutterOpenDelta)
PY_CLASS_MEMBER_RW(PerspectiveCamera, shutterCloseDelta, setShutterCloseDelta)
PY_CLASS_MEMBER_RW(PerspectiveCamera, shutterTime, setShutterTime)

// --- public --------------------------------------------------------------------------------------

PerspectiveCamera::PerspectiveCamera():
	position_(0, 0, 0),
	sky_(0, 0, 1),
	shutterOpenDelta_(0),
	shutterCloseDelta_(0),
	width_(0.036f),
	nearLimit_(TNumTraits::zero),
	farLimit_(TNumTraits::max),
	focusDistance_(TNumTraits::max)
{
	setAspectRatio(4.f / 3.f);
	setLensRadius(0);
	setFovAngle(TNumTraits::pi / 2);
	setLensRadius(0),
	lookAt(TPoint3D(0, 1, 0));
}



/** return position of camera
 */
const TPoint3D& PerspectiveCamera::position() const
{
	return position_;
}



/** set position of camera
 */
void PerspectiveCamera::setPosition(const TPoint3D& position)
{
	position_ = position;
}



const TVector3D& PerspectiveCamera::sky() const
{
	return sky_;
}



void PerspectiveCamera::setSky(const TVector3D& sky)
{
	sky_ = sky;
}



/** return direction vector of camera.
 *  not always normalized
 */
const TVector3D& PerspectiveCamera::direction() const
{
	return direction_;
}



/** set @e direction of camera directly.
 */
void PerspectiveCamera::setDirection(const TVector3D& direction) 
{
	directionNormal_ = direction.normal();
	rightNormal_ = cross(directionNormal_, sky_).normal();
	downNormal_ = cross(directionNormal_, rightNormal_).normal();
	initTransformation();
}



/** sets camera so it looks to @a target point.
 *  The PerspectiveCamera tilts around the @e sky vector towards @a target (
 *  so that @e position, @a target and the @e sky vector are coplanar), 
 *  and then pitches to line up @e direction with @a target.
 *  @e right and @e down are reset to be orthogonal to the direction and to 
 *  fit @e fovAngle and @e aspectRatio.
 */
void PerspectiveCamera::lookAt(const TPoint3D& target)
{
	setDirection(target - position_);
	focusAt(target);
}



const TVector3D& PerspectiveCamera::right() const
{
	return right_;
}



const TVector3D& PerspectiveCamera::down() const
{
	return down_;
}



void PerspectiveCamera::setRight(const TVector3D& right)
{
	rightNormal_ = right.normal();
	initTransformation();
}



void PerspectiveCamera::setDown(const TVector3D& down)
{
	downNormal_ = down.normal();
	initTransformation();
}



const TScalar PerspectiveCamera::width() const
{
	return width_;
}



const TScalar PerspectiveCamera::height() const
{
	return height_;
}



/** return aspect ratio
 */
const TScalar PerspectiveCamera::aspectRatio() const
{
	return aspectRatio_;
}



void PerspectiveCamera::setWidth(TScalar width)
{
	width_ = width;
	aspectRatio_ = width_ / height_;
	fovAngle_ = 2 * num::atan((width_ / 2) / focalLength_);
	initTransformation();
}



void PerspectiveCamera::setHeight(TScalar height)
{
	height_ = height;
	aspectRatio_ = width_ / height_;
	initTransformation();
}



/** set aspect ratio and scale @e down
 */
void PerspectiveCamera::setAspectRatio(TScalar ratio)
{
	aspectRatio_ = ratio;
	height_ = width_ / aspectRatio_;
	initTransformation();
}



const TScalar PerspectiveCamera::focalLength() const
{
	return focalLength_;
}



/** return horizontal FOV in radians
 */
const TScalar PerspectiveCamera::fovAngle() const
{
	return fovAngle_;
}



void PerspectiveCamera::setFocalLength(TScalar length)
{
	focalLength_ = length;
	fovAngle_ = 2 * num::atan((width_ / 2) / focalLength_);
	setFNumber(fNumber_);
	initTransformation();
}


/** adjust horizontal FOV in radians.
 *  - @c fovAngle = fov
 *  - @c right is scaled so that its orthogonal part to direction expands the FOV.
 *  - @c down is also scaled so that the aspect ratio is preserved.
 */
void PerspectiveCamera::setFovAngle(TScalar radians)
{
	fovAngle_ = radians;
	focalLength_ = (width_ / 2) / num::tan(fovAngle_ / 2);
	setFNumber(fNumber_);
	initTransformation();
}


const TScalar PerspectiveCamera::nearLimit() const
{
	return nearLimit_;
}



const TScalar PerspectiveCamera::farLimit() const
{
	return farLimit_;
}



void PerspectiveCamera::setNearLimit(TScalar distance)
{
	nearLimit_ = distance;
}



void PerspectiveCamera::setFarLimit(TScalar distance)
{
	farLimit_ = distance;
}



const TScalar PerspectiveCamera::focusDistance() const
{
	return focusDistance_;
}



void PerspectiveCamera::setFocusDistance(TScalar distance)
{
	focusDistance_ = distance;
}



void PerspectiveCamera::focusAt(const TPoint3D& target)
{
	focusDistance_ = distance(position_, target);
}



const TScalar PerspectiveCamera::fNumber() const
{
	return fNumber_;
}



const TScalar PerspectiveCamera::lensRadius() const
{
	return lensRadius_;
}



void PerspectiveCamera::setFNumber(TScalar fNumber)
{
	fNumber_ = fNumber;
	lensRadius_ = focalLength_ / (2 * fNumber);
}



void PerspectiveCamera::setLensRadius(TScalar radius)
{
	lensRadius_ = radius;
	fNumber_ = focalLength_ / (2 * radius);
}



const TTime PerspectiveCamera::shutterOpenDelta() const
{
	return shutterOpenDelta_;
}



const TTime PerspectiveCamera::shutterCloseDelta() const
{
	return shutterCloseDelta_;
}



const TTime PerspectiveCamera::shutterTime() const
{
	return shutterCloseDelta_ - shutterOpenDelta_;
}



void PerspectiveCamera::setShutterOpenDelta(TTimeDelta shutterOpenDelta)
{
	shutterOpenDelta_ = shutterOpenDelta;
}



void PerspectiveCamera::setShutterCloseDelta(TTimeDelta shutterCloseDelta)
{
	shutterCloseDelta_ = shutterCloseDelta;
}



void PerspectiveCamera::setShutterTime(TTimeDelta shutterTime)
{
	shutterCloseDelta_ = shutterOpenDelta_ + shutterTime;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------
   
const BoundedRay PerspectiveCamera::doGenerateRay(
		const Sample& sample, const TVector2D& screenSpaceDelta) const
{
	const TPoint2D& screen = sample.screenCoordinate() + screenSpaceDelta;

	TPoint3D raySupport = position_;
	TVector3D rayDirection = directionBase_ + screen.x * right_ + screen.y * down_;
	rayDirection.normalize();
	TScalar rayFactor = num::inv(dot(rayDirection, directionNormal_));

	if (lensRadius_ > 0 && focusDistance_ > 0 && focusDistance_ < TNumTraits::max)
	{
#pragma LASS_TODO("do something with a focal plane versus focal cyilinder")
		TScalar pdf;
		const TPoint2D& lens = num::uniformDisk(sample.lensCoordinate(), pdf);
		const TPoint3D lensPoint = position_ +
			lensRadius_ * lens.x * rightNormal_ +
			lensRadius_ * lens.y * downNormal_;		
		const TPoint3D focusPoint = position_ + (rayFactor * focusDistance_) * rayDirection;

		raySupport = lensPoint;
		rayDirection = focusPoint - lensPoint;
		rayDirection.normalize();
		rayFactor = num::inv(dot(rayDirection, directionNormal_));
	}
	
	return BoundedRay(raySupport, rayDirection, 
		std::max(tolerance, rayFactor * nearLimit_), rayFactor * farLimit_,
		prim::IsAlreadyNormalized());
}



const TimePeriod PerspectiveCamera::doShutterDelta() const
{
	return TimePeriod(shutterOpenDelta_, shutterCloseDelta_);
}



const TScalar PerspectiveCamera::doAsDepth(const TRay3D& ray, TScalar t) const
{
	return t * prim::dot(ray.direction(), direction_.normal());
}



void PerspectiveCamera::initTransformation()
{
	direction_ = directionNormal_ * focalLength_;
	right_ = rightNormal_ * width_;
	down_ = downNormal_ * height_;
	directionBase_ = direction_ - (down_ + right_) / 2;
}



const TPyObjectPtr PerspectiveCamera::doGetState() const
{
	return python::makeTuple(
		position_,
		sky_,
		directionNormal_,
		rightNormal_,
		downNormal_,
		shutterOpenDelta_,
		shutterCloseDelta_,
		width_,
		height_,
		focalLength_,
		nearLimit_,
		farLimit_,
		focusDistance_,
		fNumber_);
}



void PerspectiveCamera::doSetState(const TPyObjectPtr& state)
{
	LASS_ENFORCE(python::decodeTuple(state,
		position_,
		sky_,
		directionNormal_,
		rightNormal_,
		downNormal_,
		shutterOpenDelta_,
		shutterCloseDelta_,
		width_,
		height_,
		focalLength_,
		nearLimit_,
		farLimit_,
		focusDistance_,
		fNumber_
	) == 0);

	setWidth(width_);
	setHeight(height_);
	setFocalLength(focalLength_);
	setFNumber(fNumber_);	
	initTransformation();
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
