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
#include "projection_perspective.h"
//#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(ProjectionPerspective, "")
PY_CLASS_CONSTRUCTOR_0(ProjectionPerspective)
PY_CLASS_MEMBER_RW(ProjectionPerspective, position, setPosition)
PY_CLASS_MEMBER_RW(ProjectionPerspective, sky, setSky)
PY_CLASS_MEMBER_RW(ProjectionPerspective, direction, setDirection)
PY_CLASS_METHOD(ProjectionPerspective, lookAt)
PY_CLASS_MEMBER_RW(ProjectionPerspective, right, setRight)
PY_CLASS_MEMBER_RW(ProjectionPerspective, up, setUp)
PY_CLASS_MEMBER_RW(ProjectionPerspective, width, setWidth)
PY_CLASS_MEMBER_RW(ProjectionPerspective, height, setHeight)
PY_CLASS_MEMBER_RW(ProjectionPerspective, aspectRatio, setAspectRatio)
PY_CLASS_MEMBER_RW(ProjectionPerspective, focalLength, setFocalLength)
PY_CLASS_MEMBER_RW(ProjectionPerspective, fovAngle, setFovAngle)


// --- public --------------------------------------------------------------------------------------

ProjectionPerspective::ProjectionPerspective():
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
const TPoint3D& ProjectionPerspective::position() const
{
	return position_;
}



/** set position of camera
 */
void ProjectionPerspective::setPosition(const TPoint3D& position)
{
	position_ = position;
}



const TVector3D& ProjectionPerspective::sky() const
{
	return sky_;
}



void ProjectionPerspective::setSky(const TVector3D& sky)
{
	sky_ = sky;
}



/** return direction vector of camera.
 *  not always normalized
 */
const TVector3D& ProjectionPerspective::direction() const
{
	return direction_;
}



/** set @e direction of camera directly.
 */
void ProjectionPerspective::setDirection(const TVector3D& direction) 
{
	directionNormal_ = direction.normal();
	rightNormal_ = cross(directionNormal_, sky_).normal();
	upNormal_ = cross(rightNormal_, directionNormal_).normal();
	initTransformation();
}



/** sets camera so it looks to @a target point.
 *  The ProjectionPerspective tilts around the @e sky vector towards @a target (
 *  so that @e position, @a target and the @e sky vector are coplanar), 
 *  and then pitches to line up @e direction with @a target.
 *  @e right and @e down are reset to be orthogonal to the direction and to 
 *  fit @e fovAngle and @e aspectRatio.
 */
void ProjectionPerspective::lookAt(const TPoint3D& target)
{
	setDirection(target - position_);
}



const TVector3D& ProjectionPerspective::right() const
{
	return right_;
}



const TVector3D& ProjectionPerspective::up() const
{
	return up_;
}



void ProjectionPerspective::setRight(const TVector3D& right)
{
	rightNormal_ = right.normal();
	initTransformation();
}



void ProjectionPerspective::setUp(const TVector3D& up)
{
	upNormal_ = up.normal();
	initTransformation();
}



TScalar ProjectionPerspective::width() const
{
	return width_;
}



TScalar ProjectionPerspective::height() const
{
	return height_;
}



/** return aspect ratio
 */
TScalar ProjectionPerspective::aspectRatio() const
{
	return aspectRatio_;
}



void ProjectionPerspective::setWidth(TScalar width)
{
	width_ = width;
	aspectRatio_ = width_ / height_;
	fovAngle_ = 2 * num::atan((width_ / 2) / focalLength_);
	initTransformation();
}



void ProjectionPerspective::setHeight(TScalar height)
{
	height_ = height;
	aspectRatio_ = width_ / height_;
	initTransformation();
}



/** set aspect ratio and scale @e down
 */
void ProjectionPerspective::setAspectRatio(TScalar ratio)
{
	aspectRatio_ = ratio;
	height_ = width_ / aspectRatio_;
	initTransformation();
}



TScalar ProjectionPerspective::focalLength() const
{
	return focalLength_;
}



/** return horizontal FOV in radians
 */
TScalar ProjectionPerspective::fovAngle() const
{
	return fovAngle_;
}



void ProjectionPerspective::setFocalLength(TScalar length)
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
void ProjectionPerspective::setFovAngle(TScalar radians)
{
	fovAngle_ = radians;
	focalLength_ = (width_ / 2) / num::tan(fovAngle_ / 2);
	initTransformation();
}



// --- protected -----------------------------------------------------------------------------------

const TRay3D ProjectionPerspective::doRay(const TUv& uv, TScalar& pdf) const
{
	const TVector3D d = plane_.point(uv) - position_;
	const TRay3D ray(position_, d);

	const TScalar cosTheta = dot(ray.direction(), directionNormal_);
	pdf = d.squaredNorm() / (width_ * height_ * cosTheta);

	return ray;
}



const Projection::TUv ProjectionPerspective::doUv(const TPoint3D& point, TRay3D& ray, TScalar& t) const
{
	if (squaredDistance(point, position_) < num::sqr(tolerance))
	{
		t = 0;
		return TUv();
	}
	ray = TRay3D(position_, point);
	if (prim::intersect(plane_, ray, t) != prim::rOne)
	{
		t = 0;
		return TUv();
	}
	const TPoint3D intersection = ray.point(t);
	return plane_.uv(intersection);
}



// --- private -------------------------------------------------------------------------------------

void ProjectionPerspective::initTransformation()
{
	direction_ = directionNormal_ * focalLength_;
	right_ = rightNormal_ * width_;
	up_ = upNormal_ * height_;
	
	const TPoint3D support = position_ + direction_ - (right_ + up_) / 2;
	plane_ = TProjectionPlane(support, right_, up_);
}



const TPyObjectPtr ProjectionPerspective::doGetState() const
{
	return python::makeTuple(
		position_,
		sky_,
		directionNormal_,
		rightNormal_,
		upNormal_,
		width_,
		height_,
		focalLength_
		);
}



void ProjectionPerspective::doSetState(const TPyObjectPtr& state)
{
	LASS_ENFORCE(python::decodeTuple(state,
		position_,
		sky_,
		directionNormal_,
		rightNormal_,
		upNormal_,
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
