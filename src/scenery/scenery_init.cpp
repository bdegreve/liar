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

#include "scenery_common.h"

// keep in alphabetical order please! [Bramz]
//
#include "light_point.h"
#include "list.h"
#include "motion_translation.h"
#include "plane.h"
#include "sphere.h"
#include "transformation.h"
#include "translation.h"

#include <lass/io/proxy_man.h>

PY_DECLARE_MODULE(scenery)

extern "C"
{
LIAR_SCENERY_DLL void initscenery(void)
{
#ifndef _DEBUG
	//lass::io::proxyMan()->clog()->remove(&std::clog);
#endif

    using namespace liar::scenery;

	PY_INJECT_MODULE_EX(scenery, "liar.scenery", "LiAR scene objects")

	// keep in alphabetical order please! [Bramz]
	//
	PY_INJECT_CLASS_IN_MODULE(LightPoint, scenery, "point light")
    PY_INJECT_CLASS_IN_MODULE(List, scenery, "flat list of child objects")
	PY_INJECT_CLASS_IN_MODULE(MotionTranslation, scenery, "time-dependent translation")
    PY_INJECT_CLASS_IN_MODULE(Plane, scenery, "infinite plane")
    PY_INJECT_CLASS_IN_MODULE(Sphere, scenery, "a nice sphere")
    PY_INJECT_CLASS_IN_MODULE(Transformation, scenery, "transformation of local space")
    PY_INJECT_CLASS_IN_MODULE(Translation, scenery, "translation of local space")

	PyRun_SimpleString("print 'liar.scenery imported'\n");
}

}

// EOF