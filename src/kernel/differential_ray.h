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

/** @class liar::kernel::DifferentialRay
 *  @brief a bounded ray plus two rays for screen space differentials
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_DIFFERENTIAL_RAY_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_DIFFERENTIAL_RAY_H

#include "kernel_common.h"
#include "bounded_ray.h"
#include <lass/prim/point_2d.h>

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL DifferentialRay
{
public:

    DifferentialRay(const BoundedRay& iCentralRay, 
		const TRay3D& iDifferentialI, 
		const TRay3D& iDifferentialJ);

	const BoundedRay& centralRay() const { return centralRay_; }
	const TRay3D& differentialI() const { return differentialI_; }
	const TRay3D& differentialJ() const { return differentialJ_; }

	const TPoint3D& support() const { return centralRay_.support(); }		/**< return support point of central ray */
	const TVector3D& direction() const { return centralRay_.direction(); }	/**< return diretion of central ray */

private:

    BoundedRay centralRay_;
    TRay3D differentialI_;
    TRay3D differentialJ_;
};

}

}

#endif

// EOF
