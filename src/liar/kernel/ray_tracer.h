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

/** @class liar::RayTracer
 *  @brief base class of the actual ray tracer
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_RAY_TRACER_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_RAY_TRACER_H

#include "kernel_common.h"
#include "differential_ray.h"
#include "sample.h"
#include "scene_object.h"
#include "light_context.h"
#include "light_sample.h"
#include "spectral.h"

namespace liar
{
namespace kernel
{
class IntersectionContext;
class RayTracer;
class Sampler;
class SceneObject;

typedef python::PyObjectPtr<RayTracer>::Type TRayTracerPtr;
typedef python::PyObjectPtr<SceneObject>::Type TSceneObjectPtr;

class LIAR_KERNEL_DLL RayTracer: public python::PyObjectPlus
{
	PY_HEADER(python::PyObjectPlus)
public:

	typedef num::Tuint32 TSeed;

	virtual ~RayTracer();

	const TSceneObjectPtr& scene() const;
	size_t maxRayGeneration() const;

	void setScene(const TSceneObjectPtr& scene);
	void setMaxRayGeneration(size_t rayGeneration);

	void requestSamples(const TSamplerPtr& sampler);
	void preProcess(const TSamplerPtr& sampler, const TimePeriod& period, size_t numberOfThreads = 0);

	/** @warning castRay is NOT THREAD SAFE!
	 */
	const Spectral castRay(const Sample& sample, const DifferentialRay& primaryRay, TScalar& tIntersection, TScalar& alpha, bool highQuality = true) const
	{
		RayGenerationIncrementor incrementor(*this);
		if (static_cast<size_t>(rayGeneration_) > maxRayGeneration_)
		{
			tIntersection = TNumTraits::infinity;
			alpha = 0;
			return Spectral();
		}
		return doCastRay(sample, primaryRay, tIntersection, alpha, static_cast<size_t>(rayGeneration_), highQuality);
	}

	const TRayTracerPtr clone() const;
	void seed(TSeed seed);

	const TPyObjectPtr reduce() const;
	const TPyObjectPtr getState() const;
	void setState(const TPyObjectPtr& state);

protected:

	RayTracer();

	const LightContexts& lights() const { return lights_; }
	MediumStack& mediumStack() const { return mediumStack_; }

	const Spectral estimateLightContribution(
			const Sample& sample, const TBsdfPtr& bsdf, const LightContext& light,
			const Sample::TSubSequence2D& lightSamples,  const Sample::TSubSequence2D& bsdfSamples, const Sample::TSubSequence1D& componentSamples,
			const TPoint3D& target, const TVector3D& targetNormal, const TVector3D& omegaIn) const;

	void requestLightAndSceneSamples(const TSamplerPtr& sampler);

private:

	virtual void doRequestSamples(const TSamplerPtr& sampler) = 0;
	virtual void doPreProcess(const TSamplerPtr& sampler, const TimePeriod& period, size_t numberOfThreads) = 0;
	virtual const Spectral doCastRay(const Sample& sample, const DifferentialRay& primaryRay, TScalar& tIntersection, TScalar& alpha, size_t generation, bool highQuality) const = 0;
	virtual const TRayTracerPtr doClone() const = 0;
	virtual void doSeed(num::Tuint32 seed) = 0;

	virtual const TPyObjectPtr doGetState() const = 0;
	virtual void doSetState(const TPyObjectPtr& state) = 0;

	class RayGenerationIncrementor: public util::NonCopyable
	{
	public:
		RayGenerationIncrementor(const RayTracer& rayTracer):
			rayTracer_(rayTracer)
		{
			++rayTracer_.rayGeneration_;
			LASS_ASSERT(rayTracer_.rayGeneration_ >= 0);
		}
		~RayGenerationIncrementor()
		{
			LASS_ASSERT(rayTracer_.rayGeneration_ >= 0);
			--rayTracer_.rayGeneration_;
		}
	private:
		const RayTracer& rayTracer_;
	};

	friend class RayGenerationIncrementor;

	TSceneObjectPtr scene_;
	LightContexts lights_;
	size_t maxRayGeneration_;
	mutable int rayGeneration_;
	mutable MediumStack mediumStack_;
};

}

}

#endif

// EOF
