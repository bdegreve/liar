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

/** @class liar::kernel::DifferentialRay
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_DIFFERENTIAL_RAY_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_DIFFERENTIAL_RAY_H

#include "kernel_common.h"
#include <lass/prim/point_2d.h>

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL DifferentialRay
{
public:

    DifferentialRay(const TRay3D& iRay, const TRay3D& iDifferentialX, const TRay3D& iDifferentialY);

    const TRay3D& ray() const;
    const TRay3D& differentialX() const;
    const TRay3D& differentialY() const;

private:

    TRay3D ray_;
    TRay3D differentialX_;
    TRay3D differentialY_;
};

}

}

#endif

// EOF
