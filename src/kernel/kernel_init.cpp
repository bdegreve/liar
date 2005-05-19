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

#include "kernel_common.h"

// keep in alphabetical order please! [Bramz]
//
#include "camera.h"
#include "sampler.h"
#include "scene_composite.h"
#include "scene_light.h"
#include "scene_object.h"
#include "shader.h"
#include "ray_tracer.h"
#include "render_engine.h"
#include "render_target.h"
#include "texture.h"

#include <lass/io/proxy_man.h>
#include <lass/stde/extended_string.h>

void license()
{
    std::string text = lass::stde::replace_all(
        liar::license, std::string("\n"), std::string("\\n"));
	PyRun_SimpleString( std::string("print \"" + text + "\"\n").c_str());
}

PY_DECLARE_MODULE(kernel)
PY_MODULE_FUNCTION(kernel, license)

extern "C"
{
void LIAR_KERNEL_DLL initkernel(void)
{
#ifndef _DEBUG
	lass::io::proxyMan()->clog()->remove(&std::clog);
#endif

    using namespace liar::kernel;

	PY_INJECT_MODULE_EX_AT_RUNTIME(kernel, "liar.kernel", "LIAR isn't a raytracer")

	// keep in alphabetical order please! [Bramz]
	//
    PY_INJECT_CLASS_IN_MODULE_AT_RUNTIME(Camera, kernel, "Abstract base class of render viewports")
    PY_INJECT_CLASS_IN_MODULE_AT_RUNTIME(Sampler, kernel, "Abstract base class of samplers")
    PY_INJECT_CLASS_IN_MODULE_AT_RUNTIME(SceneObject, kernel, "Abstract base class of scene objects")
    PY_INJECT_CLASS_IN_MODULE_AT_RUNTIME(Shader, kernel, "Abstract base class of shaders")
    PY_INJECT_CLASS_IN_MODULE_AT_RUNTIME(RayTracer, kernel, "Abstract base class of ray tracers")
    PY_INJECT_CLASS_IN_MODULE_AT_RUNTIME(RenderEngine, kernel, "Render engine")
    PY_INJECT_CLASS_IN_MODULE_AT_RUNTIME(RenderTarget, kernel, "Abstract base class of render targets")
    PY_INJECT_CLASS_IN_MODULE_AT_RUNTIME(Texture, kernel, "Abstract base class of textures")

	// must be injected after SceneObject
	//
    PY_INJECT_CLASS_IN_MODULE_AT_RUNTIME(SceneComposite, kernel, "Abstract base class of invisible scene objects that perform spatial subdivision")
    PY_INJECT_CLASS_IN_MODULE_AT_RUNTIME(SceneLight, kernel, "Abstract base class of scene lights")

	std::ostringstream header;
	header << liar::name << " v" << liar::version << " ("
        << LASS_LIB_PLATFORM "_" LASS_LIB_COMPILER LASS_LIB_DEBUG << ") "
		<< __DATE__ << " - " << __TIME__ << "\\n"
        << "authors: " << liar::authors << "\\n"
		<< liar::name << " comes with ABSOLUTELY NO WARRANTY.\\n"
		<< "This is free software, and you are welcome to redistribute it \\n"
        << "under certain conditions.  Call license() for details.\\n";
		
	PyRun_SimpleString( std::string("print \"" + header.str() + "\"\n").c_str());
	PyRun_SimpleString("print 'liar.kernel imported'\n");
}

}

// EOF