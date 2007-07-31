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

#include "output_common.h"

#if LIAR_HAVE_LOCAL_CONFIG_H
#	include "../local_config.h"
#endif

#include <lass/io/proxy_man.h>

// keep in alphabetical order please! [Bramz]
//
#include "depth_channel.h"
#if LIAR_HAVE_PIXELTOASTER_H
#	include "display.h"
#endif
#include "filter_mitchell.h"
#include "image.h"
#include "splitter.h"

PY_DECLARE_MODULE(output)

extern "C"
{
LIAR_OUTPUT_DLL void initoutput(void)
{
    using namespace liar::output;

	PY_INJECT_MODULE_EX(output, "liar.output", "LiAR output devices")
	
	// keep in alphabetical order please! [Bramz]
	//
    PY_INJECT_CLASS_IN_MODULE(DepthChannel, output, "converts depth channel to color information")
#if LIAR_HAVE_PIXELTOASTER_H
    PY_INJECT_CLASS_IN_MODULE(Display, output, "render target in a window (PixelToaster)")
#endif
    PY_INJECT_CLASS_IN_MODULE(FilterMitchell, output, "Mitchell reconstruction filter")
    PY_INJECT_CLASS_IN_MODULE(Image, output, "simple image render target")
    PY_INJECT_CLASS_IN_MODULE(Splitter, output, "splits output stream to several render targets")

	PyRun_SimpleString("print 'liar.output imported (v" 
		LIAR_VERSION_FULL " - " __DATE__ ", " __TIME__ ")'\n");
}

}

// EOF
