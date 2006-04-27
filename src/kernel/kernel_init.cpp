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

#include "kernel_common.h"

// keep in alphabetical order please! [Bramz]
//
#include "attenuation.h"
#include "camera.h"
#include "sampler.h"
//#include "scene_composite.h"
#include "scene_light.h"
#include "scene_object.h"
#include "shader.h"
#include "ray_tracer.h"
#include "render_engine.h"
#include "render_target.h"
#include "rgb_space.h"
#include "spectrum.h"
#include "spectrum_format.h"
#include "texture.h"

#include <lass/io/proxy_man.h>
#include <lass/stde/extended_string.h>
#include <lass/util/process.h>

void license()
{
    std::string text = lass::stde::replace_all(
        liar::license, std::string("\n"), std::string("\\n"));
	PyRun_SimpleString( std::string("print \"" + text + "\"\n").c_str());
}

liar::TScalar tolerance()
{
	return liar::tolerance;
}

void setTolerance(liar::TScalar iTolerance)
{
	liar::tolerance = iTolerance;
}

PY_DECLARE_MODULE(kernel)
PY_MODULE_FUNCTION(kernel, license)
PY_MODULE_FUNCTION(kernel, tolerance)
PY_MODULE_FUNCTION(kernel, setTolerance)

using liar::util::setProcessPriority;
PY_MODULE_FUNCTION_QUALIFIED_DOC_1(kernel, setProcessPriority, void, const std::string&,
	"setProcessPriority({low, belownormal, normal, abovenormal, high}\n")

using liar::TScalar;
using liar::TVector3D;
using liar::kernel::xyz;
using liar::kernel::Spectrum;
using liar::kernel::TSpectrumFormatPtr;
PY_MODULE_FUNCTION_QUALIFIED_DOC_1(kernel, xyz, Spectrum, const TVector3D&,
	"xyz({<X>, <Y>, <Z>} | <(X, Y, Z)>} [, <spectrumFormat>]\n"
	"Create a spectrum from XYZ tristimulus value\n");
PY_MODULE_FUNCTION_QUALIFIED_3(kernel, xyz, Spectrum, TScalar, TScalar, TScalar)
//PY_MODULE_FUNCTION_QUALIFIED_2(kernel, xyz, Spectrum, const TVector3D&, const TSpectrumFormatPtr&)
//PY_MODULE_FUNCTION_QUALIFIED_4(kernel, xyz, Spectrum, TScalar, TScalar, TScalar, const TSpectrumFormatPtr&)

using lass::prim::ColorRGBA;
using liar::kernel::rgb;
using liar::kernel::RgbSpace;
using liar::kernel::TRgbSpacePtr;
PY_MODULE_FUNCTION_QUALIFIED_DOC_1(kernel, rgb, Spectrum, const ColorRGBA&,
	"rgb({<R>, <G>, <B>} | <(R, G, B)>} [, <RgbSpace>] [, <SpectrumFormat>]\n"
	"Create a spectrum from RGB color value\n");
PY_MODULE_FUNCTION_QUALIFIED_2(kernel, rgb, Spectrum, const ColorRGBA&, const TRgbSpacePtr&)
PY_MODULE_FUNCTION_QUALIFIED_3(kernel, rgb, Spectrum, TScalar, TScalar, TScalar)
PY_MODULE_FUNCTION_QUALIFIED_4(kernel, rgb, Spectrum, TScalar, TScalar, TScalar, const TRgbSpacePtr&)
//PY_MODULE_FUNCTION_QUALIFIED_2(kernel, rgb, Spectrum, const ColorRGBA&, const TSpectrumFormatPtr&)
//PY_MODULE_FUNCTION_QUALIFIED_3(kernel, rgb, Spectrum, const ColorRGBA&, const TRgbSpacePtr&, const TSpectrumFormatPtr&)
//PY_MODULE_FUNCTION_QUALIFIED_4(kernel, rgb, Spectrum, TScalar, TScalar, TScalar, const TSpectrumFormatPtr&)
//PY_MODULE_FUNCTION_QUALIFIED_5(kernel, rgb, Spectrum, TScalar, TScalar, TScalar, const TRgbSpacePtr&, const TSpectrumFormatPtr&)


extern "C"
{
void LIAR_KERNEL_DLL initkernel(void)
{
#ifndef _DEBUG
	lass::io::proxyMan()->clog()->remove(&std::clog);
#endif

	using namespace liar;
    using namespace liar::kernel;

	PY_INJECT_MODULE_EX(kernel, "liar.kernel", "LiAR isn't a raytracer")

	// keep in alphabetical order please! [Bramz]
	//
	PY_INJECT_CLASS_IN_MODULE(Attenuation, kernel, "Attenuation constants")
    PY_INJECT_CLASS_IN_MODULE(Camera, kernel, "Abstract base class of render viewports")
    PY_INJECT_CLASS_IN_MODULE(RayTracer, kernel, "Abstract base class of ray tracers")
    PY_INJECT_CLASS_IN_MODULE(RenderEngine, kernel, "Render engine")
    PY_INJECT_CLASS_IN_MODULE(RenderTarget, kernel, "Abstract base class of render targets")
    PY_INJECT_CLASS_IN_MODULE(Sampler, kernel, "Abstract base class of samplers")
    PY_INJECT_CLASS_IN_MODULE(SceneObject, kernel, "Abstract base class of scene objects")
    PY_INJECT_CLASS_IN_MODULE(Shader, kernel, "Abstract base class of shaders")
    //PY_INJECT_CLASS_IN_MODULE(SpectrumFormat, kernel, "Format of a spectrum's colour representation")
    PY_INJECT_CLASS_IN_MODULE(Texture, kernel, "Abstract base class of textures")

	// must be injected after SceneObject
	//
    //PY_INJECT_CLASS_IN_MODULE(SceneComposite, kernel, "Abstract base class of invisible scene objects that perform spatial subdivision")
    PY_INJECT_CLASS_IN_MODULE(SceneLight, kernel, "Abstract base class of scene lights")

	// init rgb spaces
	//
	//PY_INJECT_OBJECT_IN_MODULE_EX(TRgbSpacePtr(rgbSpaceIdentity), kernel, "rgbSpaceIdentity");
	//PY_INJECT_OBJECT_IN_MODULE_EX(TRgbSpacePtr(rgbSpaceCie), kernel, "rgbSpaceCie");
	//PY_INJECT_OBJECT_IN_MODULE_EX(SpectrumFormat::defaultFormat(), kernel, "formatTripleBand");

	std::ostringstream header;
	header << liar::name << " v" << liar::version << " ("
        << LASS_LIB_PLATFORM "_" LASS_LIB_COMPILER LASS_LIB_DEBUG << ")\\n"
        << "authors: " << liar::authors << "\\n"
		<< "website: " << liar::website << "\\n"
		<< liar::name << " comes with ABSOLUTELY NO WARRANTY.\\n"
		<< "This is free software, and you are welcome to redistribute it \\n"
        << "under certain conditions.  Call license() for details.\\n";
		
	PyRun_SimpleString( std::string("print \"" + header.str() + "\"\n").c_str());
	PyRun_SimpleString("print 'liar.kernel imported (v" 
		LIAR_VERSION_FULL " - " __DATE__ ", " __TIME__ ")'\n");

	util::setProcessPriority(util::ppBelowNormal);
}

}

// EOF