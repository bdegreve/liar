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
 *  http://liar.bramz.org
 */

/** @namespace liar::samplers
 *  @brief sample generators
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SAMPLERS_SAMPLERS_COMMON_H
#define LIAR_GUARDIAN_OF_INCLUSION_SAMPLERS_SAMPLERS_COMMON_H

#include "../kernel/kernel_common.h"

#if defined(LIAR_SAMPLERS_BUILD_DLL)
#   define LIAR_SAMPLERS_DLL LASS_DLL_EXPORT
#else
#   define LIAR_SAMPLERS_DLL LASS_DLL_IMPORT
#   if defined(_DEBUG)
#       pragma comment(lib, "samplers_d.lib")
#   else
#       pragma comment(lib, "samplers.lib")
#   endif
#endif

namespace liar
{
namespace samplers
{
	
using namespace liar::kernel;

}

}

#endif

// EOF
