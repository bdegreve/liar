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

/** @class liar::scenery::Triangle
 *  @brief a single triangular object
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_TRIANGLE_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_TRIANGLE_H

#include "scenery_common.h"
#include "../kernel/scene_object.h"

#include <lass/prim/triangle_3d.h>

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL Triangle: public kernel::SceneObject
{
    PY_HEADER(kernel::SceneObject)
public:

    Triangle(const TPoint3D& iA, const TPoint3D& iB, const TPoint3D& iC);

private:

    typedef prim::Triangle3D<TScalar> TTriangle3D;

    LASS_UTIL_ACCEPT_VISITOR
    
	void doIntersect(const kernel::Sample& iSample, const kernel::BoundedRay& iRay, 
		kernel::Intersection& oResult) const;
	const bool doIsIntersecting(const kernel::Sample& iSample, const kernel::BoundedRay& iRay) const;
	void doLocalContext(const kernel::Sample& iSample, const kernel::BoundedRay& iRay,
		const kernel::Intersection& iIntersection, kernel::IntersectionContext& oResult) const;
	const bool doContains(const kernel::Sample& iSample, const TPoint3D& iPoint) const;
	const TAabb3D doBoundingBox(const kernel::TimePeriod& iPeriod) const;

    TTriangle3D triangle_;
};



}

}

#endif

// EOF
