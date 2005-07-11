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

#include "tracers_common.h"

// keep in alphabetical order please! [Bramz]
//
#include "direct_lighting.h"

#include <lass/io/proxy_man.h>

PY_DECLARE_MODULE(tracers)

extern "C"
{
LIAR_TRACERS_DLL void inittracers(void)
{
#ifndef _DEBUG
	//lass::io::proxyMan()->clog()->remove(&std::clog);
#endif

    using namespace liar::tracers;

	PY_INJECT_MODULE_EX(tracers, "liar.tracers", "LiAR ray tracers")

	// keep in alphabetical orderplease! [Bramz]
	//
	PY_INJECT_CLASS_IN_MODULE(DirectLighting, tracers, "simple ray tracer")

	PyRun_SimpleString("print 'liar.tracers imported'\n");
}

}

// EOF