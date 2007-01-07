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
 *  http://liar.sourceforge.net
 */

#include "cameras_common.h"
#include "perspective_camera.h"
#include <lass/io/proxy_man.h>

PY_DECLARE_MODULE(cameras)

extern "C"
{
LIAR_CAMERAS_DLL void initcameras(void)
{
#ifndef _DEBUG
	//lass::io::proxyMan()->clog()->remove(&std::clog);
#endif

    using namespace liar::cameras;

	PY_INJECT_MODULE_EX(cameras, "liar.cameras", "LiAR cameras")
    PY_INJECT_CLASS_IN_MODULE(PerspectiveCamera, cameras, "plain old perspective photo camera")

	PyRun_SimpleString("print 'liar.cameras imported (v" 
		LIAR_VERSION_FULL " - " __DATE__ ", " __TIME__ ")'\n");
}

}

// EOF
