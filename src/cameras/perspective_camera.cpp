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
PY_CLASS_MEMBER_RW(PerspectiveCamera, "shutterTime", shutterTime, setShutterTime)

// --- public --------------------------------------------------------------------------------------

PerspectiveCamera::PerspectiveCamera():
    Camera(&Type),
    position_(0, 0, 0),
    sky_(0, 0, 1),
	fovAngle_(TNumTraits::pi / 2),
	aspectRatio_(0.75f),
	shutterTime_(0)
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
    right_ = prim::cross(direction_, sky_);
    down_ = prim::cross(direction_, right_);
    setFovAngle(fovAngle_);
    initTransformation();
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
    initTransformation();
}



const TVector3D& PerspectiveCamera::right() const
{
    return right_;
}



void PerspectiveCamera::setRight(const TVector3D& iRight)
{
    right_ = iRight;

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



const TTimeDelta PerspectiveCamera::shutterTime() const
{
    return shutterTime_;
}



void PerspectiveCamera::setShutterTime(TTimeDelta iShutterTime)
{
    shutterTime_ = iShutterTime;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------
   
const kernel::DifferentialRay 
PerspectiveCamera::doPrimaryRay(kernel::Sample& iSample, const TVector2D& iScreenSpaceDelta) const
{
    using namespace kernel;

    const TPoint2D& screenCoordinate = iSample.screenCoordinate();
    TVector3D dir = directionBase_ + screenCoordinate.x * right_ + screenCoordinate.y * down_;
    return DifferentialRay(TRay3D(position_, dir),
                           TRay3D(position_, dir + iScreenSpaceDelta.x * right_),
                           TRay3D(position_, dir + iScreenSpaceDelta.y * down_));
}



const TTimeDelta PerspectiveCamera::doShutterTime() const
{
	return shutterTime_;
}



void PerspectiveCamera::initTransformation()
{
    directionBase_ = direction_ - (down_ + right_) / 2;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF