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

/** @class liar::kernel::ProjectionPerspective
 *  @brief handles the maths of the projection
 *  @author Bram de Greve [Bramz]
 *
 *  This class is in a bit of a refactoring zone.  We're splitting
 *  it off from PerspectiveCamera to reuse it in a PerspectiveMapping
 *  texture.  But I'm not sure whether to generalize it, or how to
 *  ease copying attributes from camera to texture.
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_PROJECTION_PERSPECTIVE_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_PROJECTION_PERSPECTIVE_H

#include "kernel_common.h"
#include "projection.h"
#include <lass/prim/plane_3d.h>

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL ProjectionPerspective: public Projection
{
	PY_HEADER(Projection)
public:

	ProjectionPerspective();

	const TPoint3D& position() const;
	void setPosition(const TPoint3D& iPosition);

	const TVector3D& direction() const;
	void setDirection(const TVector3D& direction);
	void lookAt(const TPoint3D& target);

	const TVector3D& sky() const;
	void setSky(const TVector3D& sky);

	const TVector3D& up() const;
	const TVector3D& right() const;
	void setUp(const TVector3D& down);
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

private:

	typedef prim::Plane3D<TScalar, prim::Parametric, prim::Unnormalized> TProjectionPlane;

	const TRay3D doRay(const TUv& uv, TScalar& pdf) const;
	const TUv doUv(const TPoint3D& point, TRay3D& ray, TScalar& t) const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	void initTransformation();

	TProjectionPlane plane_;
	TPoint3D position_;
	TVector3D right_;        /**< i unit */
	TVector3D up_;         /**< j unit */
	TVector3D direction_;    /**< k unit */
	TVector3D sky_;
	TVector3D directionNormal_;
	TVector3D rightNormal_;
	TVector3D upNormal_;
	TScalar width_;
	TScalar height_;
	TScalar aspectRatio_;
	TScalar focalLength_;
	TScalar fovAngle_;
};

}

}

#endif

// EOF
