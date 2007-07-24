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

#include "samplers_common.h"
#include "stratifier.h"
#include "../kernel/sampler.h"
#include <lass/io/proxy_man.h>

void setDefaultSampler(const liar::kernel::TSamplerPtr& iDefaultSampler)
{
	liar::kernel::Sampler::defaultSampler() = iDefaultSampler;
}

PY_DECLARE_MODULE(samplers)
PY_MODULE_FUNCTION(samplers, setDefaultSampler)

extern "C"
{
LIAR_SAMPLERS_DLL void initsamplers(void)
{
#ifndef _DEBUG
	//lass::io::proxyMan()->clog()->remove(&std::clog);
#endif

    using namespace liar::samplers;

	PY_INJECT_MODULE_EX(samplers, "liar.samplers", "LiAR sample generators")
    PY_INJECT_CLASS_IN_MODULE(Stratifier, samplers, "(jittered) stratified sampler")
        
	setDefaultSampler(liar::kernel::TSamplerPtr(new Stratifier));

	PyRun_SimpleString("print 'liar.samplers imported (v" 
		LIAR_VERSION_FULL " - " __DATE__ ", " __TIME__ ")'\n");
}

}

// EOF
