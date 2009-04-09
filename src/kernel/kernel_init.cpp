/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
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

#include "kernel_common.h"

// keep in alphabetical order please! [Bramz]
//
#include "attenuation.h"
#include "camera.h"
#include "image_codec.h"
#include "medium.h"
#include "sampler.h"
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
#include <lass/io/logger.h>
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

PY_DECLARE_MODULE_DOC(kernel, "LiAR isn't a raytracer")

// keep in alphabetical order please! [Bramz]
//
PY_MODULE_CLASS(kernel, liar::kernel::Attenuation)
PY_MODULE_CLASS(kernel, liar::kernel::Camera)
PY_MODULE_CLASS(kernel, liar::kernel::ImageCodec)
PY_MODULE_CLASS(kernel, liar::kernel::ImageCodecLassLDR)
PY_MODULE_CLASS(kernel, liar::kernel::ImageCodecLassHDR)
PY_MODULE_CLASS(kernel, liar::kernel::Medium)
PY_MODULE_CLASS(kernel, liar::kernel::RgbSpace)
PY_MODULE_CLASS(kernel, liar::kernel::RayTracer)
PY_MODULE_CLASS(kernel, liar::kernel::RenderEngine)
PY_MODULE_CLASS(kernel, liar::kernel::RenderTarget)
PY_MODULE_CLASS(kernel, liar::kernel::Sampler)
PY_MODULE_CLASS(kernel, liar::kernel::SceneObject)
	PY_MODULE_CLASS(kernel, liar::kernel::SceneLight)
PY_MODULE_CLASS(kernel, liar::kernel::Shader)
PY_MODULE_CLASS(kernel, liar::kernel::Texture)

PY_MODULE_FUNCTION(kernel, license)
PY_MODULE_FUNCTION(kernel, tolerance)
PY_MODULE_FUNCTION(kernel, setTolerance)

using lass::util::setProcessPriority;
PY_MODULE_FUNCTION_QUALIFIED_DOC_1(kernel, setProcessPriority, void, const std::string&,
	"setProcessPriority('low' | 'belownormal' | 'normal' | 'abovenormal' | 'high')\n")

using liar::TScalar;
using liar::TVector3D;
using liar::kernel::xyz;
using liar::kernel::Spectrum;
using liar::kernel::TSpectrumFormatPtr;
PY_MODULE_FUNCTION_QUALIFIED_DOC_1(kernel, xyz, Spectrum, const TVector3D&,
	"xyz({<X>, <Y>, <Z>} | <(X, Y, Z)>} [, <spectrumFormat>]\n"
	"Create a spectrum from XYZ tristimulus value\n");
PY_MODULE_FUNCTION_QUALIFIED_3(kernel, xyz, Spectrum, TScalar, TScalar, TScalar)

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

using liar::kernel::imageCodecs;
PY_MODULE_FUNCTION(kernel, imageCodecs)

void kernelPreInject()
{
	lass::util::setProcessPriority(lass::util::ppBelowNormal);
#ifndef _DEBUG
	lass::io::proxyMan()->clog()->remove(&std::clog);
#endif
}

void kernelPostInject(PyObject*)
{
	std::ostringstream header;
	header << liar::name << " v" << liar::version << " ("
		<< LASS_LIB_PLATFORM "_" LASS_LIB_COMPILER LASS_LIB_DEBUG << ")\n"
		<< "authors: " << liar::authors << "\n"
		<< "website: " << liar::website << "\n"
		<< liar::name << " comes with ABSOLUTELY NO WARRANTY.\n"
		<< "This is free software, and you are welcome to redistribute it \n"
		<< "under certain conditions.  Call license() for details.\n";
		
	PyRun_SimpleString( "import sys" );
	PyRun_SimpleString( std::string("sys.stdout.write('''" + header.str() + "''')\n").c_str());
	PyRun_SimpleString("sys.stdout.write('''liar.kernel imported (v" 
		LIAR_VERSION_FULL " - " __DATE__ ", " __TIME__ ")\n''')\n");
}

LASS_EXECUTE_BEFORE_MAIN(
	kernel.setPreInject(kernelPreInject);
	kernel.setPostInject(kernelPostInject);
	)

PY_MODULE_ENTRYPOINT(kernel)

// EOF
