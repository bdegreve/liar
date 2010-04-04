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

#include "output_common.h"

#include <lass/io/proxy_man.h>

// keep in alphabetical order please! [Bramz]
//
#include "depth_channel.h"
#include "display.h"
#include "filter_mitchell.h"
#include "image.h"
#include "splitter.h"

using namespace liar::output;

PY_DECLARE_MODULE_DOC(output, 
	"LiAR output devices\n"
	"\n"
	"this software is based in part on the PixelToaster library."
	)

// keep in alphabetical order please! [Bramz]
//
PY_MODULE_CLASS(output, DepthChannel)
#if LIAR_OUTPUT_HAVE_PIXELTOASTER_H
PY_MODULE_CLASS(output, Display)
#endif
PY_MODULE_CLASS(output, FilterMitchell)
PY_MODULE_CLASS(output, Image)
PY_MODULE_CLASS(output, Splitter)

void outputPostInject(PyObject*)
{
	PyRun_SimpleString( "import sys" );
	PyRun_SimpleString("sys.stdout.write('''liar.output imported (v" LIAR_VERSION_FULL " - " __DATE__ ", " __TIME__ ")\n''')\n");
}

LASS_EXECUTE_BEFORE_MAIN(
	output.setPostInject(outputPostInject);
	)

PY_MODULE_ENTRYPOINT(output)

// EOF
