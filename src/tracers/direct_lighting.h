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

/** @class liar::tracers::DirectLighting
 *  @brief a ray tracer that only uses direct lighting.
 *  @author Bram de Greve [Bramz]
 */

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
	void doRequestSamples(const TSamplerPtr& sampler);
	void DirectLighting::doPreProcess(const kernel::TSamplerPtr& sampler, const TimePeriod& period);
	const Spectrum doCastRay(const Sample& sample, const DifferentialRay& primaryRay, 
		TScalar& tIntersection, TScalar& alpha, int generation) const;
	const TLightSamplesRange doSampleLights(const Sample& sample, const TPoint3D& target, 
		const TVector3D& targetNormal) const;
	const TRayTracerPtr doClone() const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	const Spectrum traceDirect(const Sample& sample, 
		const IntersectionContext& context,	const TPoint3D& target, const TVector3D& targetNormal,
		const TVector3D& omegaIn) const;
	const Spectrum traceSpecularAndGlossy(
		const Sample& sample, const IntersectionContext& context, const kernel::DifferentialRay& primaryRay,
		const TPoint3D& target, const TVector3D& targetNormal, const TVector3D& omegaIn, bool singleSample) const;


	mutable MediumStack mediumStack_;
};

}

}

#endif

// EOF
