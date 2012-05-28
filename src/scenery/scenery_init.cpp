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

#include "scenery_common.h"

// keep in alphabetical order please! [Bramz]
//
//#include "aabb_tree.h"
#include "box.h"
#include "clip_map.h"
#include "csg.h"
#include "disk.h"
#include "goursat.h"
#include "light_area.h"
#include "light_directional.h"
#include "light_point.h"
#include "light_sky.h"
#include "light_spot.h"
#include "list.h"
#include "motion_translation.h"
#include "object_trees.h"
#include "parallelogram.h"
#include "plane.h"
#include "sphere.h"
#include "sky.h"
#include "transformation.h"
#include "translation.h"
#include "triangle.h"
#include "triangle_mesh.h"
#include "triangle_mesh_composite.h"

#include <lass/io/proxy_man.h>

using namespace liar::scenery;

PY_DECLARE_MODULE_DOC(scenery, "LiAR scene objects")

// keep in alphabetical order please! [Bramz]
//
PY_MODULE_CLASS(scenery, AabbTree)
PY_MODULE_CLASS(scenery, AabpTree)
PY_MODULE_CLASS(scenery, Box)
PY_MODULE_CLASS(scenery, ClipMap)
PY_MODULE_CLASS(scenery, Csg)
PY_MODULE_CLASS(scenery, Disk)
PY_MODULE_CLASS(scenery, Goursat)
PY_MODULE_CLASS(scenery, LightArea)
PY_MODULE_CLASS(scenery, LightDirectional)
PY_MODULE_CLASS(scenery, LightPoint)
PY_MODULE_CLASS(scenery, LightSky)
PY_MODULE_CLASS(scenery, LightSpot)
PY_MODULE_CLASS(scenery, List)
PY_MODULE_CLASS(scenery, MotionTranslation)
PY_MODULE_CLASS(scenery, OctTree)
PY_MODULE_CLASS(scenery, Parallelogram)
PY_MODULE_CLASS(scenery, Plane)
PY_MODULE_CLASS(scenery, Sphere)
PY_MODULE_CLASS(scenery, Sky)
PY_MODULE_CLASS(scenery, Transformation)
PY_MODULE_CLASS(scenery, Translation)
PY_MODULE_CLASS(scenery, Triangle)
PY_MODULE_CLASS(scenery, TriangleMesh)
PY_MODULE_CLASS(scenery, TriangleMeshComposite)

void sceneryPostInject(PyObject*)
{
	LASS_COUT << "liar.scenery imported (v" LIAR_VERSION_FULL " - " __DATE__ ", " __TIME__ ")\n";
}

LASS_EXECUTE_BEFORE_MAIN(
	scenery.setPostInject(sceneryPostInject);
	)

PY_MODULE_ENTRYPOINT(scenery)

// EOF
