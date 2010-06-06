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

protected:

	void doRequestSamples(const TSamplerPtr& sampler);
	void doPreProcess(const kernel::TSamplerPtr& sampler, const TimePeriod& period, size_t numberOfThreads);
	const XYZ doCastRay(const Sample& sample, const DifferentialRay& primaryRay, TScalar& tIntersection, TScalar& alpha, int generation) const;
	const TRayTracerPtr doClone() const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	virtual const XYZ doShadeMedium(const kernel::Sample& sample, const DifferentialRay& primaryRay, TScalar tMax, XYZ& transparency) const;
	virtual const XYZ doShadeSurface(const kernel::Sample& sample, const DifferentialRay& primaryRay, const IntersectionContext& context,
		const TPoint3D& point, const TVector3D& normal, const TVector3D& omega, int generation) const;

	const XYZ traceDirect(const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& target, const TVector3D& targetNormal, const TVector3D& omegaIn) const;
	const XYZ traceSpecularAndGlossy(
		const Sample& sample, const kernel::DifferentialRay& primaryRay, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& target, const TVector3D& targetNormal, const TVector3D& omegaIn, bool singleSample) const;
};

}

}

#endif

// EOF
