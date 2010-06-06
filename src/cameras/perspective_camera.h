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

/** @class liar::cameras::PerspectiveCamera
 *  @brief a standard perspective camera
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_CAMERAS_PERSPECTIVE_CAMERA_H
#define LIAR_GUARDIAN_OF_INCLUSION_CAMERAS_PERSPECTIVE_CAMERA_H

#include "cameras_common.h"
#include "../kernel/camera.h"
#include <lass/prim/point_2d.h>

namespace liar
{
namespace cameras
{

class LIAR_CAMERAS_DLL PerspectiveCamera: public Camera
{
	PY_HEADER(Camera)
public:

	PerspectiveCamera();

	const TPoint3D& position() const;
	void setPosition(const TPoint3D& iPosition);

	const TVector3D& direction() const;
	void setDirection(const TVector3D& direction);
	void lookAt(const TPoint3D& target);

	const TVector3D& sky() const;
	void setSky(const TVector3D& sky);

	/*
	void tilt(TScalar angle);
	void pitch(TScalar angle);
	void roll(TScalar angle);
	*/

	const TVector3D& down() const;
	const TVector3D& right() const;
	void setDown(const TVector3D& down);
	void setRight(const TVector3D& right);

	TScalar width() const;
	TScalar height() const;
	TScalar aspectRatio() const;
	void setWidth(TScalar width);
	void setHeight(TScalar height);
	void setAspectRatio(TScalar ratio);

	TScalar focalLength() const;
	TScalar fovAngle() const;
	void setFocalLength(TScalar length);
	void setFovAngle(TScalar fov);

	TScalar nearLimit() const;
	TScalar farLimit() const;
	void setNearLimit(TScalar distance);
	void setFarLimit(TScalar distance);

	TScalar focusDistance() const;
	void setFocusDistance(TScalar distance);
	void focusAt(const TPoint3D& target);

	TScalar fNumber() const;
	TScalar lensRadius() const;
	void setFNumber(TScalar fNumber);
	void setLensRadius(TScalar radius);

	TTime shutterOpenDelta() const;
	TTime shutterCloseDelta() const;
	TTime shutterTime() const;
	void setShutterOpenDelta(TTime shutterOpenDelta);
	void setShutterCloseDelta(TTime shutterCloseDelta);
	void setShutterTime(TTime shutterTime);

private:

	const BoundedRay doGenerateRay(const Sample& sample, const TVector2D& screenSpaceDelta) const;
	const TimePeriod doShutterDelta() const;
	TScalar doAsDepth(const TRay3D& ray, TScalar t) const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	void initTransformation();

	TPoint3D position_;
	TVector3D right_;        /**< i unit */
	TVector3D down_;         /**< j unit */
	TVector3D direction_;    /**< k unit */
	TVector3D sky_;
	TVector3D directionNormal_;
	TVector3D rightNormal_;
	TVector3D downNormal_;
	TTime shutterOpenDelta_;
 	TTime shutterCloseDelta_;
	TScalar width_;
	TScalar height_;
	TScalar aspectRatio_;
	TScalar focalLength_;
	TScalar fovAngle_;
	TScalar nearLimit_;
	TScalar farLimit_;
	TScalar focusDistance_;
	TScalar fNumber_;
	TScalar lensRadius_;

	TVector3D directionBase_;
};

}

}

#endif

// EOF
