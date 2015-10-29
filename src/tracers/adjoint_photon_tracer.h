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

/** @class liar::tracers::AdjointPhotonTracer
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_TRACERS_ADJOINT_PHOTON_TRACER_H
#define LIAR_GUARDIAN_OF_INCLUSION_TRACERS_ADJOINT_PHOTON_TRACER_H

#include "tracers_common.h"
#include "../kernel/ray_tracer.h"

namespace liar
{
namespace tracers
{

class LIAR_TRACERS_DLL AdjointPhotonTracer: public RayTracer
{
	PY_HEADER(RayTracer)
public:

	AdjointPhotonTracer();

private:

	void doRequestSamples(const TSamplerPtr& sampler);
	void doPreProcess(const kernel::TSamplerPtr& sampler, const TimePeriod& period, size_t numberOfThreads);
	const Spectral doCastRay(const Sample& sample, const DifferentialRay& primaryRay, TScalar& tIntersection, TScalar& alpha, size_t generation, bool highQuality) const;
	const TRayTracerPtr doClone() const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);
};

}

}

#endif

// EOF
