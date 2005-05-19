/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LIAR isn't a raytracer
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

/** @class liar::cameras::PerspectiveCamera
 *  @brief a standard perspective camera
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_CAMERAS_PERSPECTIVE_CAMERA_H
#define LIAR_GUARDIAN_OF_INCLUSION_CAMERAS_PERSPECTIVE_CAMERA_H

#include "cameras_common.h"
#include "../kernel/camera.h"
#include <lass/prim/point_2d.h>

namespace liar
{
namespace cameras
{

class LIAR_CAMERAS_DLL PerspectiveCamera: public kernel::Camera
{
    PY_HEADER(kernel::Camera)
public:
    
    PerspectiveCamera();

    const TPoint3D& position() const;
    void setPosition(const TPoint3D& iPosition);

    void lookAt(const TPoint3D&);

    const TScalar fovAngle() const;
    void setFovAngle(TScalar iFov);

    const TScalar aspectRatio() const;
    void setAspectRatio(TScalar iRatio);

    void tilt(TScalar iAngle);
    void pitch(TScalar iAngle);
    void roll(TScalar iAngle);

    const TVector3D& direction() const;
    void setDirection(const TVector3D& iDirection);

    const TVector3D& down() const;
    void setDown(const TVector3D& iDown);

    const TVector3D& right() const;
    void setRight(const TVector3D& iRight);

	const TVector3D& sky() const;
	void setSky(const TVector3D& iSky);

private:

    virtual kernel::DifferentialRay doPrimaryRay(kernel::Sample& ioSample,
        const TVector2D& iScreenSpaceDelta) const;

    void initTransformation();

    TPoint3D position_;
    TVector3D right_;        /**< i unit */
    TVector3D down_;         /**< j unit */
    TVector3D direction_;    /**< k unit */
    TVector3D sky_;
    TScalar fovAngle_;
    TScalar aspectRatio_;

    TVector3D directionBase_;
};

}

}

#endif

// EOF
