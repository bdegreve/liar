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

/** @class liar::Sampler
 *  @brief generates samples to be feed to the ray tracer.
 *  @author Bram de Greve [Bramz]
 *
 *  @warning
 *  THE INTERFACE OF THE SAMPLER IS STILL PRELIMINARY.  The main problem is how to avoid that two 
 *  engines use the same sampler and thus screwing each other.  Or if they do use the same sampler,
 *  how can we manage they don't screw each other. 
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SAMPLER_PROGRESSIVE_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SAMPLER_PROGRESSIVE_H

#include "kernel_common.h"
#include "sampler.h"

namespace liar
{
namespace kernel
{

class SamplerProgressive;

typedef python::PyObjectPtr<SamplerProgressive>::Type TSamplerProgressivePtr;

class LIAR_KERNEL_DLL SamplerProgressive : public Sampler
{
	PY_HEADER(Sampler)
public:
	static TSamplerProgressivePtr& defaultProgressiveSampler();

protected:
	SamplerProgressive();

private:
	static TSamplerProgressivePtr defaultProgressiveSampler_;
};

}

}

#endif

// EOF

