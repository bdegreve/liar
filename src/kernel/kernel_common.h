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

/** @namespace liar
 *  @brief LiAR isn't a raytracer
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

/** @namespace liar::kernel
 *  @brief namespace with core elements of LiAR
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_KERNEL_COMMON_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_KERNEL_COMMON_H

#if defined(LIAR_KERNEL_BUILD_DLL)
#   define LIAR_KERNEL_DLL LASS_DLL_EXPORT
#else
#   define LIAR_KERNEL_DLL LASS_DLL_IMPORT
#   if defined(_DEBUG)
#       pragma comment(lib, "kernel_d.lib")
#   else
#       pragma comment(lib, "kernel.lib")
#   endif
#endif

#define LASS_USE_DLL
#include <lass/num/num_traits.h>
#include <lass/num/basic_ops.h>
#include <lass/num/floating_point_comparison.h>
#include <lass/prim/vector_2d.h>
#include <lass/prim/point_2d.h>
#include <lass/prim/vector_3d.h>
#include <lass/prim/point_3d.h>
#include <lass/prim/ray_3d.h>
#include <lass/prim/aabb_3d.h>
#include <lass/prim/color_rgba.h>
#include <lass/prim/transformation_3d.h>
#include <lass/util/pyobject_plus.h>

#include "config.h"

#define LIAR_VERSION_FULL LASS_STRINGIFY(LIAR_VERSION_MAJOR) "."\
	LASS_STRINGIFY(LIAR_VERSION_MINOR) "." LASS_STRINGIFY(LIAR_VERSION_REVISION)

namespace liar
{

using namespace lass;



// typedefs

typedef LIAR_SCALAR TScalar;
typedef LIAR_TIME TTime;
typedef LIAR_TIME_DELTA TTimeDelta;

typedef num::NumTraits<TScalar> TNumTraits;

typedef prim::Vector2D<TScalar> TVector2D;
typedef prim::Point2D<TScalar> TPoint2D;
typedef prim::Vector3D<TScalar> TVector3D;
typedef prim::Point3D<TScalar> TPoint3D;
typedef prim::Ray3D<TScalar, prim::Normalized, prim::Unbounded> TRay3D;
typedef prim::Aabb3D<TScalar> TAabb3D;
typedef prim::Transformation3D<TScalar> TTransformation3D;

typedef python::PyObjectPtr<PyObject>::Type TPyObjectPtr;



// globals

LIAR_KERNEL_DLL extern TScalar tolerance;



// constants

const std::string name = LIAR_NAME_FULL;
const std::string version = LIAR_VERSION_FULL;
const std::string authors = LIAR_AUTHORS;
const std::string website = LIAR_WEBSITE;

const std::string license =	
	LIAR_NAME_FULL "\n"
    "Copyright (C) 2004-2005 " LIAR_AUTHORS "\n"
	"\n"
	"This is free software; you can redistribute it and/or modify\n"
	"it under the terms of the GNU General Public License as published by\n"
	"the Free Software Foundation; either version 2 of the License, or\n"
	"(at your option) any later version.\n"
	"\n"
	"This program is distributed in the hope that it will be useful,\n"
	"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	"GNU General Public License for more details.\n"
	"\n"
	"You should have received a copy of the GNU General Public License\n"
	"along with this program; if not, write to the Free Software\n"
	"Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n"
    "\n"
    LIAR_WEBSITE "\n";



namespace kernel
{
}

}

#endif

// EOF
