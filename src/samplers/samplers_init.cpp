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

#include "samplers_common.h"
#include "stratifier.h"
#include "latin_hypercube.h"
#include "../kernel/sampler.h"
#include <lass/io/proxy_man.h>

using namespace liar::samplers;

void setDefaultSampler(const liar::kernel::TSamplerPtr& defaultSampler)
{
	liar::kernel::Sampler::defaultSampler() = defaultSampler;
}

PY_DECLARE_MODULE_DOC(samplers, "LiAR sample generators")

PY_MODULE_CLASS(samplers, Stratifier)
PY_MODULE_CLASS(samplers, LatinHypercube)

PY_MODULE_FUNCTION(samplers, setDefaultSampler)

void samplersPostInject(PyObject*)
{
	setDefaultSampler(liar::kernel::TSamplerPtr(new LatinHypercube));
	PyRun_SimpleString( "import sys" );
	PyRun_SimpleString("sys.stdout.write('''liar.samplers imported (v" LIAR_VERSION_FULL " - " __DATE__ ", " __TIME__ ")\n''')\n");
}

LASS_EXECUTE_BEFORE_MAIN(
	samplers.setPostInject(samplersPostInject);
	)

PY_MODULE_ENTRYPOINT(samplers)

// EOF
