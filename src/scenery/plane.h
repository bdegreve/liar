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

/** @class liar::scenery::Plane
 *  @brief planar object of infinite size
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_PLANE_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_PLANE_H

#include "scenery_common.h"
#include "../kernel/scene_object.h"

#include <lass/prim/plane_3d.h>

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL Plane: public kernel::SceneObject
{
    PY_HEADER(kernel::SceneObject)
public:

	Plane();
    Plane(const TVector3D& i_normal, TScalar i_d);

    const TVector3D& normal() const;
	void setNormal(const TVector3D& iNormal);

    const TScalar d() const;
	void setD(TScalar iD);

private:

    typedef prim::Plane3D<TScalar, prim::Combined> TPlane3D;

    LASS_UTIL_ACCEPT_VISITOR
    
	void doIntersect(const kernel::Sample& iSample, const TRay3D& iRay, 
		kernel::Intersection& oResult) const;
	const bool doIsIntersecting(const kernel::Sample& iSample, const TRay3D& iRay, 
		TScalar iMaxT) const;
	void doLocalContext(const kernel::Sample& iSample, const TRay3D& iRay, 
		const kernel::Intersection& iIntersection, kernel::IntersectionContext& oResult) const;
	const TAabb3D doBoundingBox(const kernel::TimePeriod& iPeriod) const;

    TPlane3D plane_;
};



}

}

#endif

// EOF
