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

/** @namespace liar::scenery
 *  @brief scene objects
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_SCENERY_COMMON_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_SCENERY_COMMON_H

#include "../kernel/kernel_common.h"

#if defined(LIAR_SCENERY_BUILD_DLL)
#   define LIAR_SCENERY_DLL LASS_DLL_EXPORT
#else
#   define LIAR_SCENERY_DLL LASS_DLL_IMPORT
#   if defined(_DEBUG)
#       pragma comment(lib, "scenery_d.lib")
#   else
#       pragma comment(lib, "scenery.lib")
#   endif
#endif

namespace liar
{
namespace scenery
{
}

}

#endif

// EOF
