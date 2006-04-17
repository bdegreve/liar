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

#include "cameras_common.h"
#include "perspective_camera.h"

namespace liar
{
namespace cameras
{


PY_DECLARE_CLASS(PerspectiveCamera)
PY_CLASS_CONSTRUCTOR_0(PerspectiveCamera)
PY_CLASS_MEMBER_RW_DOC(PerspectiveCamera, "position", position, setPosition, "position of camera")
PY_CLASS_METHOD_DOC(PerspectiveCamera, lookAt, "set direction of camera by looking at a location")
PY_CLASS_MEMBER_RW(PerspectiveCamera, "fovAngle", fovAngle, setFovAngle)
PY_CLASS_MEMBER_RW(PerspectiveCamera, "aspectRatio", aspectRatio, setAspectRatio)
PY_CLASS_MEMBER_RW(PerspectiveCamera, "direction", direction, setDirection)
PY_CLASS_MEMBER_RW(PerspectiveCamera, "right", right, setRight)
PY_CLASS_MEMBER_RW(PerspectiveCamera, "down", down, setDown)
PY_CLASS_MEMBER_RW(PerspectiveCamera, "sky", sky, setSky)
PY_CLASS_MEMBER_RW(PerspectiveCamera, "focalDistance", focalDistance, setFocalDistance)
PY_CLASS_MEMBER_RW(PerspectiveCamera, "lensRadius", lensRadius, setLensRadius)
PY_CLASS_MEMBER_RW(PerspectiveCamera, "shutterOpenDelta", shutterOpenDelta, setShutterOpenDelta)
PY_CLASS_MEMBER_RW(PerspectiveCamera, "shutterCloseDelta", shutterCloseDelta, setShutterCloseDelta)

// --- public --------------------------------------------------------------------------------------

PerspectiveCamera::PerspectiveCamera():
    Camera(&Type),
    position_(0, 0, 0),
    sky_(0, 0, 1),
	fovAngle_(TNumTraits::pi / 2),
	aspectRatio_(0.75f),
	focalDistance_(TNumTraits::max),
	lensRadius_(TNumTraits::zero),
	nearLimit_(TNumTraits::zero),
	farLimit_(TNumTraits::max),
	shutterOpenDelta_(0),
	shutterCloseDelta_(0)
{
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
void PerspectiveCamera::setPosition(const TPoint3D& iPosition)
{
    position_ = iPosition;
}



/** sets camera so it looks to @a iLookAt point.
 *  The PerspectiveCamera tilts around the @e sky vector towards @a iLookAt (so that @e position, @a iLookAt and
 *  the @e sky vector are coplanar), and then pitches to line up @e direction with @a iLookAt.
 *  @e right and @e down are reset to be orthogonal to the direction and to fit @e fovAngle and
 *  @e aspectRatio.
 *
 *  - @e direction = @a iLookAt - @e position
 *  - @e right = @e direction cross @e sky, and scaled to fit @e fovAngle
 *  - @e down = @e direction cross @e right, and scaled to fit @e aspectRatio
 */
void PerspectiveCamera::lookAt(const TPoint3D& iLookAt)
{
    setDirection(iLookAt - position_);
	focalDistance_ = prim::distance(position_, iLookAt);
}



/** return horizontal FOV in radians
 */
const TScalar PerspectiveCamera::fovAngle() const
{
    return fovAngle_;
}



/** adjust horizontal FOV in radians.
 *  - @c fovAngle = iFov
 *  - @c right is scaled so that its orthogonal part to direction expands the FOV.
 *  - @c down is also scaled so that the aspect ratio is preserved.
 */
void PerspectiveCamera::setFovAngle(TScalar iFov)
{
    fovAngle_ = iFov;
    right_ *= 2 * num::tan(fovAngle_ / 2) * direction_.norm() / direction_.reject(right_).norm();
    setAspectRatio(aspectRatio_);
    initTransformation();
}



/** return aspect ratio
 */
const TScalar PerspectiveCamera::aspectRatio() const
{
    return aspectRatio_;
}



/** set aspect ratio and scale @e down
 */
void PerspectiveCamera::setAspectRatio(TScalar iAspect)
{
    aspectRatio_ = iAspect;
    down_ *= aspectRatio_ * direction_.reject(right_).norm() / direction_.reject(down_).norm();
    initTransformation();
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
void PerspectiveCamera::setDirection(const TVector3D& iDirection) 
{
    direction_ = iDirection;
    right_ = prim::cross(direction_, sky_);
    down_ = prim::cross(direction_, right_);
	rightNormal_ = right_.normal();
	downNormal_ = down_.normal();
    setFovAngle(fovAngle_);
    initTransformation();
}



const TVector3D& PerspectiveCamera::right() const
{
    return right_;
}



void PerspectiveCamera::setRight(const TVector3D& iRight)
{
    right_ = iRight;
	rightNormal_ = right_.normal();

    const TVector3D orthoRight = direction_.reject(right_);
    const TVector3D orthoDown = direction_.reject(down_);
    fovAngle_ = 2 * num::atan(orthoRight.norm() / (2 * direction_.norm()));
    aspectRatio_ = orthoRight.norm() / orthoDown.norm();

    initTransformation();
}



const TVector3D& PerspectiveCamera::down() const
{
    return down_;
}



void PerspectiveCamera::setDown(const TVector3D& iDown)
{
    down_ = iDown;
	downNormal_ = down_.normal();

    const TVector3D orthoRight = direction_.reject(down_);
    const TVector3D orthoDown = direction_.reject(right_);
    aspectRatio_ = orthoRight.norm() / orthoDown.norm();

    initTransformation();
}



const TVector3D& PerspectiveCamera::sky() const
{
    return sky_;
}



void PerspectiveCamera::setSky(const TVector3D& iSky)
{
    sky_ = iSky;
}



const TTime PerspectiveCamera::shutterOpenDelta() const
{
    return shutterOpenDelta_;
}



const TTime PerspectiveCamera::shutterCloseDelta() const
{
    return shutterCloseDelta_;
}



void PerspectiveCamera::setShutterOpenDelta(TTimeDelta iShutterOpenDelta)
{
    shutterOpenDelta_ = iShutterOpenDelta;
}



void PerspectiveCamera::setShutterCloseDelta(TTimeDelta iShutterCloseDelta)
{
    shutterCloseDelta_ = iShutterCloseDelta;
}



TScalar PerspectiveCamera::focalDistance() const
{
	return focalDistance_;
}



TScalar PerspectiveCamera::lensRadius() const
{
	return lensRadius_;
}



void PerspectiveCamera::setFocalDistance(TScalar iDistance)
{
    focalDistance_ = iDistance;
}



void PerspectiveCamera::setLensRadius(TScalar iRadius)
{
    lensRadius_ = iRadius;
}


TScalar PerspectiveCamera::nearLimit() const
{
	return nearLimit_;
}



TScalar PerspectiveCamera::farLimit() const
{
	return farLimit_;
}



void PerspectiveCamera::setNearLimit(TScalar iDistance)
{
	nearLimit_ = iDistance;
}



void PerspectiveCamera::setFarLimit(TScalar iDistance)
{
	farLimit_ = iDistance;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------
   
namespace impl
{

#pragma LASS_TODO("move this to LASS [Bramz]")
TPoint2D sampleConcentricDisk(const TPoint2D& uv)
{
	TScalar r, theta;
	const TScalar x = 2 * uv.x - 1;
	const TScalar y = 2 * uv.y - 1;
	const TScalar pi_4 = TNumTraits::pi / 4;

	if (x > -y) 
	{
		if (x > y) 
		{
			r = x;
			theta = pi_4 * (y / x);
		}
		else       
		{
			r = y;
			theta = pi_4 * (2 - (x / y));
		}
	}
	else 
	{
		if (x < y) 
		{	
			r = -x;
			theta = pi_4 * (4 + (y / x));
		}
		else       
		{
			r = -y;
			if (y != 0)
			{
                theta = pi_4 * (6 - (x / y));
			}
            else
			{
                theta = 0;
			}
		}
	}

	return TPoint2D(r * num::cos(theta), r * num::sin(theta));
}

}

const BoundedRay 
PerspectiveCamera::doGenerateRay(const Sample& iSample, const TVector2D& iScreenSpaceDelta) const
{
	const TPoint2D& screen = iSample.screenCoordinate() + iScreenSpaceDelta;

	const TVector3D cameraDirection = direction_.normal();
    
	TPoint3D raySupport = position_;
	TVector3D rayDirection = directionBase_ + screen.x * right_ + screen.y * down_;
	rayDirection.normalize();
	TScalar rayFactor = num::inv(dot(rayDirection, cameraDirection));

	if (lensRadius_ > 0 && focalDistance_ > 0)
	{
#pragma LASS_TODO("do something with a focal plane versus focal cyilinder")
		const TPoint2D& lens = impl::sampleConcentricDisk(iSample.lensCoordinate());
		const TPoint3D lensPoint = position_ +
			lensRadius_ * lens.x * rightNormal_ +
			lensRadius_ * lens.y * downNormal_;		
		const TPoint3D focusPoint = position_ + (rayFactor * focalDistance_) * rayDirection;

		raySupport = lensPoint;
		rayDirection = focusPoint - lensPoint;
		rayDirection.normalize();
		rayFactor = num::inv(dot(rayDirection, cameraDirection));
	}
	
	return BoundedRay(raySupport, rayDirection, 
		std::max(tolerance, rayFactor * nearLimit_), rayFactor * farLimit_,
		prim::IsAlreadyNormalized());
}



const TimePeriod PerspectiveCamera::doShutterDelta() const
{
	return TimePeriod(shutterOpenDelta_, shutterCloseDelta_);
}



void PerspectiveCamera::initTransformation()
{
    directionBase_ = direction_ - (down_ + right_) / 2;
}



const TPyObjectPtr PerspectiveCamera::doGetState() const
{
	return python::makeTuple(
			position_,
			right_,
			down_,
			direction_,
			sky_,
			rightNormal_,
			downNormal_,
			shutterOpenDelta_,
 			shutterCloseDelta_,
			fovAngle_,
			aspectRatio_,
			nearLimit_,
			farLimit_,
			focalDistance_,
			lensRadius_);
}



void PerspectiveCamera::doSetState(const TPyObjectPtr& iState)
{
	LASS_ENFORCE(python::decodeTuple(
		iState,
		position_,
		right_,
		down_,
		direction_,
		sky_,
		rightNormal_,
		downNormal_,
		shutterOpenDelta_,
 		shutterCloseDelta_,
		fovAngle_,
		aspectRatio_,
		nearLimit_,
		farLimit_,
		focalDistance_,
		lensRadius_
	) == 0);
	initTransformation();
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF