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

#include "kernel_common.h"
#include "perspective_projection.h"
//#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(PerspectiveProjection, "")
PY_CLASS_CONSTRUCTOR_0(PerspectiveProjection)
PY_CLASS_MEMBER_RW(PerspectiveProjection, position, setPosition)
PY_CLASS_MEMBER_RW(PerspectiveProjection, sky, setSky)
PY_CLASS_MEMBER_RW(PerspectiveProjection, direction, setDirection)
PY_CLASS_METHOD(PerspectiveProjection, lookAt)
PY_CLASS_MEMBER_RW(PerspectiveProjection, right, setRight)
PY_CLASS_MEMBER_RW(PerspectiveProjection, down, setDown)
PY_CLASS_MEMBER_RW(PerspectiveProjection, width, setWidth)
PY_CLASS_MEMBER_RW(PerspectiveProjection, height, setHeight)
PY_CLASS_MEMBER_RW(PerspectiveProjection, aspectRatio, setAspectRatio)
PY_CLASS_MEMBER_RW(PerspectiveProjection, focalLength, setFocalLength)
PY_CLASS_MEMBER_RW(PerspectiveProjection, fovAngle, setFovAngle)


// --- public --------------------------------------------------------------------------------------

PerspectiveProjection::PerspectiveProjection():
	Projection(),
	position_(0, 0, 0),
	sky_(0, 0, 1),
	width_(0.036f)
{
	setAspectRatio(4.f / 3.f);
	setFovAngle(TNumTraits::pi / 2);
	lookAt(TPoint3D(0, 1, 0));
}



/** return position of camera
 */
const TPoint3D& PerspectiveProjection::position() const
{
	return position_;
}



/** set position of camera
 */
void PerspectiveProjection::setPosition(const TPoint3D& position)
{
	position_ = position;
}



const TVector3D& PerspectiveProjection::sky() const
{
	return sky_;
}



void PerspectiveProjection::setSky(const TVector3D& sky)
{
	sky_ = sky;
}



/** return direction vector of camera.
 *  not always normalized
 */
const TVector3D& PerspectiveProjection::direction() const
{
	return direction_;
}



/** set @e direction of camera directly.
 */
void PerspectiveProjection::setDirection(const TVector3D& direction) 
{
	directionNormal_ = direction.normal();
	rightNormal_ = cross(directionNormal_, sky_).normal();
	downNormal_ = cross(directionNormal_, rightNormal_).normal();
	initTransformation();
}



/** sets camera so it looks to @a target point.
 *  The PerspectiveProjection tilts around the @e sky vector towards @a target (
 *  so that @e position, @a target and the @e sky vector are coplanar), 
 *  and then pitches to line up @e direction with @a target.
 *  @e right and @e down are reset to be orthogonal to the direction and to 
 *  fit @e fovAngle and @e aspectRatio.
 */
void PerspectiveProjection::lookAt(const TPoint3D& target)
{
	setDirection(target - position_);
}



const TVector3D& PerspectiveProjection::right() const
{
	return right_;
}



const TVector3D& PerspectiveProjection::down() const
{
	return down_;
}



void PerspectiveProjection::setRight(const TVector3D& right)
{
	rightNormal_ = right.normal();
	initTransformation();
}



void PerspectiveProjection::setDown(const TVector3D& down)
{
	downNormal_ = down.normal();
	initTransformation();
}



TScalar PerspectiveProjection::width() const
{
	return width_;
}



TScalar PerspectiveProjection::height() const
{
	return height_;
}



/** return aspect ratio
 */
TScalar PerspectiveProjection::aspectRatio() const
{
	return aspectRatio_;
}



void PerspectiveProjection::setWidth(TScalar width)
{
	width_ = width;
	aspectRatio_ = width_ / height_;
	fovAngle_ = 2 * num::atan((width_ / 2) / focalLength_);
	initTransformation();
}



void PerspectiveProjection::setHeight(TScalar height)
{
	height_ = height;
	aspectRatio_ = width_ / height_;
	initTransformation();
}



/** set aspect ratio and scale @e down
 */
void PerspectiveProjection::setAspectRatio(TScalar ratio)
{
	aspectRatio_ = ratio;
	height_ = width_ / aspectRatio_;
	initTransformation();
}



TScalar PerspectiveProjection::focalLength() const
{
	return focalLength_;
}



/** return horizontal FOV in radians
 */
TScalar PerspectiveProjection::fovAngle() const
{
	return fovAngle_;
}



void PerspectiveProjection::setFocalLength(TScalar length)
{
	focalLength_ = length;
	fovAngle_ = 2 * num::atan((width_ / 2) / focalLength_);
	initTransformation();
}


/** adjust horizontal FOV in radians.
 *  - @c fovAngle = fov
 *  - @c right is scaled so that its orthogonal part to direction expands the FOV.
 *  - @c down is also scaled so that the aspect ratio is preserved.
 */
void PerspectiveProjection::setFovAngle(TScalar radians)
{
	fovAngle_ = radians;
	focalLength_ = (width_ / 2) / num::tan(fovAngle_ / 2);
	initTransformation();
}



// --- protected -----------------------------------------------------------------------------------

const TRay3D PerspectiveProjection::doRay(const TUv& uv) const
{
	return TRay3D(position_, plane_.point(uv));
}



const PerspectiveProjection::TUv PerspectiveProjection::doUv(const TPoint3D& point) const
{
	const TRay3D ray(position_, point);
	TScalar t;
	if (prim::intersect(plane_, ray, t) != prim::rOne)
	{
		return TUv();
	}
	const TPoint3D intersection = ray.point(t);
	const TUv uv = plane_.uv(intersection);
	return TUv(uv.x, -uv.y);
}



// --- private -------------------------------------------------------------------------------------

void PerspectiveProjection::initTransformation()
{
	direction_ = directionNormal_ * focalLength_;
	right_ = rightNormal_ * width_;
	down_ = downNormal_ * height_;
	
	const TPoint3D support = position_ + direction_ - (down_ + right_) / 2;
	plane_ = TProjectionPlane(support, right_, down_);
}



const TPyObjectPtr PerspectiveProjection::doGetState() const
{
	return python::makeTuple(
		position_,
		sky_,
		directionNormal_,
		rightNormal_,
		downNormal_,
		width_,
		height_,
		focalLength_
		);
}



void PerspectiveProjection::doSetState(const TPyObjectPtr& state)
{
	LASS_ENFORCE(python::decodeTuple(state,
		position_,
		sky_,
		directionNormal_,
		rightNormal_,
		downNormal_,
		width_,
		height_,
		focalLength_
	) == 0);

	setWidth(width_);
	setHeight(height_);
	setFocalLength(focalLength_);
	initTransformation();
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
