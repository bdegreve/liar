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
 */

/** @class liar::tracers::DirectLighting
 *  @brief a ray tracer that only uses direct lighting.
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_TRACERS_DIRECT_LIGHTING_H
#define LIAR_GUARDIAN_OF_INCLUSION_TRACERS_DIRECT_LIGHTING_H

#include "tracers_common.h"
#include "../kernel/ray_tracer.h"

namespace liar
{
namespace tracers
{

class LIAR_TRACERS_DLL DirectLighting: public RayTracer
{
	PY_HEADER(RayTracer)
public:

	DirectLighting();
    
private:
	

	void doPreprocess();
	void doRequestSamples(const TSamplerPtr& iSampler);
	const Spectrum doCastRay(const Sample& iSample,
		const DifferentialRay& iPrimaryRay) const;
	const TLightRange doSampleLights(const Sample& iSample,
		const IntersectionContext& iContext) const;
	const TRayTracerPtr doClone() const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& iState);

	mutable TLightSamples lightSamples_;
	mutable size_t rayGeneration_;
	size_t maxRayGeneration_;
};

}

}

#endif

// EOF
