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

#include "cameras_common.h"
#include "orthographic_camera.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace cameras
{


PY_DECLARE_CLASS_DOC(OrthographicCamera, "Orthographic projection camera")
PY_CLASS_CONSTRUCTOR_0(OrthographicCamera)
PY_CLASS_MEMBER_RW(OrthographicCamera, position, setPosition)
PY_CLASS_MEMBER_RW(OrthographicCamera, sky, setSky)
PY_CLASS_MEMBER_RW(OrthographicCamera, direction, setDirection)
PY_CLASS_METHOD(OrthographicCamera, lookAt)
PY_CLASS_MEMBER_RW(OrthographicCamera, right, setRight)
PY_CLASS_MEMBER_RW(OrthographicCamera, down, setDown)
PY_CLASS_MEMBER_RW(OrthographicCamera, width, setWidth)
PY_CLASS_MEMBER_RW(OrthographicCamera, height, setHeight)
PY_CLASS_MEMBER_RW(OrthographicCamera, aspectRatio, setAspectRatio)
PY_CLASS_MEMBER_RW(OrthographicCamera, nearLimit, setNearLimit)
PY_CLASS_MEMBER_RW(OrthographicCamera, farLimit, setFarLimit)
PY_CLASS_MEMBER_RW(OrthographicCamera, shutterOpenDelta, setShutterOpenDelta)
PY_CLASS_MEMBER_RW(OrthographicCamera, shutterCloseDelta, setShutterCloseDelta)
PY_CLASS_MEMBER_RW(OrthographicCamera, shutterTime, setShutterTime)

// --- public --------------------------------------------------------------------------------------

OrthographicCamera::OrthographicCamera():
	position_(0, 0, 0),
	sky_(0, 0, 1),
	shutterOpenDelta_(0),
	shutterCloseDelta_(0),
	width_(1.f),
	nearLimit_(0),
	farLimit_(TNumTraits::max)
{
	setAspectRatio(4.f / 3.f);
	lookAt(TPoint3D(0, 1, 0));
}



/** return position of camera
 */
const TPoint3D& OrthographicCamera::position() const
{
	return position_;
}



/** set position of camera
 */
void OrthographicCamera::setPosition(const TPoint3D& position)
{
	position_ = position;
}



const TVector3D& OrthographicCamera::sky() const
{
	return sky_;
}



void OrthographicCamera::setSky(const TVector3D& sky)
{
	sky_ = sky;
}



/** return direction vector of camera.
 */
const TVector3D& OrthographicCamera::direction() const
{
	return directionNormal_;
}



/** set @e direction of camera directly.
 */
void OrthographicCamera::setDirection(const TVector3D& direction)
{
	directionNormal_ = direction.normal();
	rightNormal_ = cross(directionNormal_, sky_).normal();
	downNormal_ = cross(directionNormal_, rightNormal_).normal();
	initTransformation();
}



/** sets camera so it looks to @a target point.
 *  The OrthographicCamera tilts around the @e sky vector towards @a target (
 *  so that @e position, @a target and the @e sky vector are coplanar),
 *  and then pitches to line up @e direction with @a target.
 *  @e right and @e down are reset to be orthogonal to the direction and to
 *  fit @e fovAngle and @e aspectRatio.
 */
void OrthographicCamera::lookAt(const TPoint3D& target)
{
	setDirection(target - position_);
}



const TVector3D& OrthographicCamera::right() const
{
	return right_;
}



const TVector3D& OrthographicCamera::down() const
{
	return down_;
}



void OrthographicCamera::setRight(const TVector3D& right)
{
	rightNormal_ = right.normal();
	initTransformation();
}



void OrthographicCamera::setDown(const TVector3D& down)
{
	downNormal_ = down.normal();
	initTransformation();
}



TScalar OrthographicCamera::width() const
{
	return width_;
}



TScalar OrthographicCamera::height() const
{
	return height_;
}



/** return aspect ratio
 */
TScalar OrthographicCamera::aspectRatio() const
{
	return width_ / height_;
}



void OrthographicCamera::setWidth(TScalar width)
{
	width_ = width;
	initTransformation();
}



void OrthographicCamera::setHeight(TScalar height)
{
	height_ = height;
	initTransformation();
}



/** set aspect ratio and scale @e down
 */
void OrthographicCamera::setAspectRatio(TScalar ratio)
{
	height_ = width_ / ratio;
	initTransformation();
}


TScalar OrthographicCamera::nearLimit() const
{
	return nearLimit_;
}



TScalar OrthographicCamera::farLimit() const
{
	return farLimit_;
}



void OrthographicCamera::setNearLimit(TScalar distance)
{
	nearLimit_ = distance;
}



void OrthographicCamera::setFarLimit(TScalar distance)
{
	farLimit_ = distance;
}



TTime OrthographicCamera::shutterOpenDelta() const
{
	return shutterOpenDelta_;
}



TTime OrthographicCamera::shutterCloseDelta() const
{
	return shutterCloseDelta_;
}



TTime OrthographicCamera::shutterTime() const
{
	return shutterCloseDelta_ - shutterOpenDelta_;
}



void OrthographicCamera::setShutterOpenDelta(TTimeDelta shutterOpenDelta)
{
	shutterOpenDelta_ = shutterOpenDelta;
}



void OrthographicCamera::setShutterCloseDelta(TTimeDelta shutterCloseDelta)
{
	shutterCloseDelta_ = shutterCloseDelta;
}



void OrthographicCamera::setShutterTime(TTime shutterTime)
{
	shutterCloseDelta_ = shutterOpenDelta_ + shutterTime;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const BoundedRay OrthographicCamera::doGenerateRay(const Sample& sample, const TVector2D& screenSpaceDelta) const
{
	const TPoint2D& screen = sample.screenSample() + screenSpaceDelta;

	TPoint3D raySupport = position_ + (screen.x - .5f) * right_ + (screen.y - .5f) * down_;
	TVector3D rayDirection = directionNormal_;

	return BoundedRay(raySupport, rayDirection,
		nearLimit_, farLimit_,
		prim::IsAlreadyNormalized());
}



const TimePeriod OrthographicCamera::doShutterDelta() const
{
	return TimePeriod(shutterOpenDelta_, shutterCloseDelta_);
}



TScalar OrthographicCamera::doWeight(const TRay3D&) const
{
	return 1;
}



TScalar OrthographicCamera::doAsDepth(const TRay3D& ray, TScalar t) const
{
	return t * prim::dot(ray.direction(), directionNormal_);
}



void OrthographicCamera::initTransformation()
{
	right_ = rightNormal_ * width_;
	down_ = downNormal_ * height_;
}



const TPyObjectPtr OrthographicCamera::doGetState() const
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
		nearLimit_,
		farLimit_);
}



void OrthographicCamera::doSetState(const TPyObjectPtr& state)
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
		nearLimit_,
		farLimit_
	) == 0);

	initTransformation();
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
