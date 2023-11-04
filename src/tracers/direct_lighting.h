/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2023  Bram de Greve (bramz@users.sourceforge.net)
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
#include <random>

namespace liar
{
namespace tracers
{

class LIAR_TRACERS_DLL DirectLighting: public RayTracer
{
	PY_HEADER(RayTracer)
public:

	DirectLighting();

	size_t numSecondaryLightSamples() const;
	void setNumSecondaryLightSamples(size_t numSamples);

	bool traceGlossy() const;
	void setTraceGlossy(bool traceGlossy);

protected:

	typedef std::conditional_t<sizeof(TScalar) >= 8, std::mt19937_64, std::mt19937> TRandomSecondary;

	void doRequestSamples(const TSamplerPtr& sampler);
	void doPreProcess(const kernel::TSamplerPtr& sampler, const TimePeriod& period, size_t numberOfThreads);
	const Spectral doCastRay(const Sample& sample, const DifferentialRay& primaryRay, TScalar& tIntersection, TScalar& alpha, size_t generation, bool highQuality) const;
	const TRayTracerPtr doClone() const;
	void doSeed(num::Tuint32 seed) override;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	virtual const Spectral doShadeMedium(const kernel::Sample& sample, const kernel::BoundedRay& ray, Spectral& transparency) const;
	virtual const Spectral doShadeSurface(const kernel::Sample& sample, const DifferentialRay& primaryRay, const IntersectionContext& context,
		const TPoint3D& point, const TVector3D& normal, const TVector3D& omega, bool highQuality) const;

	const Spectral traceDirect(const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& target, const TVector3D& targetNormal, const TVector3D& omegaIn, bool highQuality) const;
	const Spectral traceSpecularAndGlossy(
		const Sample& sample, const kernel::DifferentialRay& primaryRay, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& target, const TVector3D& targetNormal, const TVector3D& omegaIn, bool highQuality) const;
	const Spectral traceSingleScattering(const Sample& sample, const kernel::BoundedRay& ray) const;

	TRandomSecondary& secondarySampler() const { return secondarySampler_; }

private:

	mutable std::vector<TScalar> secondaryLightSelectorSamples_;
	mutable std::vector<TPoint2D> secondaryLightSamples_;
	mutable std::vector<TPoint2D> secondaryBsdfSamples_;
	mutable std::vector<TScalar> secondaryBsdfComponentSamples_;
	mutable TRandomSecondary secondarySampler_;

	size_t numSecondaryLightSamples_;
	bool traceGlossy_;
};

}

}

#endif

// EOF
