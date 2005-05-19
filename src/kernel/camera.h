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

/** @class liar::kernel::Camera
 *  @brief specifies view of scene
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_CAMERA_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_CAMERA_H

#include "kernel_common.h"
#include "differential_ray.h"
#include "sample.h"

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL Camera: public python::PyObjectPlus
{
    PY_HEADER(python::PyObjectPlus)
public:
    
    virtual ~Camera();

    DifferentialRay operator()(Sample& ioSample, const TVector2D& iScreenSpaceDelta) const
    {
        return doPrimaryRay(ioSample, iScreenSpaceDelta);
    }

protected:

    Camera(PyTypeObject* iType);

private:

    virtual DifferentialRay doPrimaryRay(Sample& ioSample, 
        const TVector2D& iScreenSpaceDelta) const = 0;
};

typedef python::PyObjectPtr<Camera>::Type TCameraPtr;

}

}

#endif

// EOF
