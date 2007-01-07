/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
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
 *  http://liar.sourceforge.net
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

    void lookAt(const TPoint3D&);

    const TScalar fovAngle() const;
    void setFovAngle(TScalar fov);

    const TScalar aspectRatio() const;
    void setAspectRatio(TScalar ratio);

    void tilt(TScalar angle);
    void pitch(TScalar angle);
    void roll(TScalar angle);

    const TVector3D& direction() const;
    void setDirection(const TVector3D& direction);

    const TVector3D& down() const;
    void setDown(const TVector3D& down);

    const TVector3D& right() const;
    void setRight(const TVector3D& iRight);

	const TVector3D& sky() const;
	void setSky(const TVector3D& sky);

	const TTime shutterOpenDelta() const;
	const TTime shutterCloseDelta() const;
	void setShutterOpenDelta(TTime shutterOpenDelta);
	void setShutterCloseDelta(TTime shutterCloseDelta);

	TScalar focalDistance() const;
	TScalar lensRadius() const;
	void setFocalDistance(TScalar distance);
	void setLensRadius(TScalar radius);

	TScalar nearLimit() const;
	TScalar farLimit() const;
	void setNearLimit(TScalar distance);
	void setFarLimit(TScalar distance);

private:

    const BoundedRay doGenerateRay(const Sample& sample, 
		const TVector2D& screenSpaceDelta) const;
	const TimePeriod doShutterDelta() const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

    void initTransformation();

    TPoint3D position_;
    TVector3D right_;        /**< i unit */
    TVector3D down_;         /**< j unit */
    TVector3D direction_;    /**< k unit */
    TVector3D sky_;
	TVector3D rightNormal_;
	TVector3D downNormal_;
	TTime shutterOpenDelta_;
 	TTime shutterCloseDelta_;
	TScalar fovAngle_;
    TScalar aspectRatio_;
	TScalar focalDistance_;
	TScalar lensRadius_;
	TScalar nearLimit_;
	TScalar farLimit_;

    TVector3D directionBase_;
};

}

}

#endif

// EOF
