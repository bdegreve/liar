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
 *  http://liar.sourceforge.net
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

    virtual ~RayTracer();

	const TSceneObjectPtr& scene() const;
	const int maxRayGeneration() const;

	void setScene(const TSceneObjectPtr& scene);
	void setMaxRayGeneration(const int rayGeneration);

	void requestSamples(const TSamplerPtr& sampler);
	void preProcess(const TSamplerPtr& sampler, const TimePeriod& period);

	/** @warning castRay is NOT THREAD SAFE!
	 */
	const Spectrum castRay(const Sample& sample, const DifferentialRay& primaryRay, 
			TScalar& tIntersection, TScalar& alpha) const
	{ 
		RayGenerationIncrementor incrementor(*this);
		if (rayGeneration_ < maxRayGeneration_)
		{
			return doCastRay(sample, primaryRay, tIntersection, alpha, rayGeneration_);
		}
		return Spectrum();
	}

	/** @warning sampleLights is NOT THREAD SAFE!
	 */
	const TLightSamplesRange sampleLights(const Sample& sample, 
			const TPoint3D& target, const TVector3D& targetNormal) const
	{
		return doSampleLights(sample, target, targetNormal);
	}

	const TRayTracerPtr clone() const;

	const TPyObjectPtr reduce() const;
	const TPyObjectPtr getState() const;
	void setState(const TPyObjectPtr& state);

protected:

	RayTracer();

	const TLightContexts& lights() const { return lights_; }

private:

	virtual void doRequestSamples(const TSamplerPtr& sampler) = 0;
	virtual void doPreProcess(const TSamplerPtr& samper, const TimePeriod& period) = 0;
	virtual const Spectrum doCastRay(const Sample& sample, 
		const DifferentialRay& primaryRay, TScalar& tIntersection, TScalar& alpha, 
		int generation) const = 0;
	virtual const TLightSamplesRange doSampleLights(const Sample& sample, 
		const TPoint3D& target, const TVector3D& targetNormal) const = 0;
	virtual const TRayTracerPtr doClone() const = 0;

	virtual const TPyObjectPtr doGetState() const = 0;
	virtual void doSetState(const TPyObjectPtr& state) = 0;

	class RayGenerationIncrementor: public util::NonCopyable
	{
	public:
		RayGenerationIncrementor(const RayTracer& iRayTracer): 
			rayTracer_(iRayTracer) 
		{ 
			++rayTracer_.rayGeneration_; 
		}
		~RayGenerationIncrementor() 
		{
			--rayTracer_.rayGeneration_;
		}
	private:
		const RayTracer& rayTracer_;
	};			
	
	friend class RayGenerationIncrementor;

	TSceneObjectPtr scene_;
	TLightContexts lights_;
	int maxRayGeneration_;
	mutable int rayGeneration_;
};

}

}

#endif

// EOF
