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
 *
 *  http://liar.sourceforge.net
 */

/** @class liar::RayTracer
 *  @brief base class of the actual ray tracer
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_RAY_TRACER_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_RAY_TRACER_H

#include "kernel_common.h"
#include "differential_ray.h"
#include "sample.h"
#include "scene_object.h"
#include "light_context.h"
#include "light_sample.h"

#include <lass/stde/iterator_range.h>

namespace liar
{
namespace kernel
{
class IntersectionContext;
class RayTracer;
class Sampler;

typedef python::PyObjectPtr<RayTracer>::Type TRayTracerPtr;

class LIAR_KERNEL_DLL RayTracer: public python::PyObjectPlus
{
    PY_HEADER(python::PyObjectPlus)
public:

	typedef stde::iterator_range<TLightSamples::const_iterator> TLightRange;

    virtual ~RayTracer();

    const TSceneObjectPtr& scene() const;
	const unsigned maxRayGeneration() const;

    void setScene(const TSceneObjectPtr& iScene);
	void setMaxRayGeneration(const unsigned iRayGeneration);

	void requestSamples(const TSamplerPtr& iSampler);

	/** @warning castRay is NOT THREAD SAFE!
	 */
    const Spectrum castRay(const Sample& iSample, const DifferentialRay& iPrimaryRay) const 
    { 
		RayGenerationIncrementor incrementor(*this);
		if (rayGeneration_ <= maxRayGeneration_)
		{
			return doCastRay(iSample, iPrimaryRay);
		}
		return Spectrum();
    }

	/** @warning sampleLights is NOT THREAD SAFE!
	 */
	const TLightRange sampleLights(const Sample& iSample, const IntersectionContext& iContext) const
	{
		return doSampleLights(iSample, iContext);
	}

	const TRayTracerPtr clone() const;

	const TPyObjectPtr reduce() const;
	const TPyObjectPtr getState() const;
	void setState(const TPyObjectPtr& iState);

protected:

    RayTracer(PyTypeObject* iType);

	const TLightContexts& lights() const { return lights_; }

private:

    virtual void doPreprocess() = 0;
	virtual void doRequestSamples(const TSamplerPtr& iSampler) = 0;
    virtual const Spectrum doCastRay(const Sample& iSample, 
		const DifferentialRay& iPrimaryRay) const = 0;
	virtual const TLightRange doSampleLights(const Sample& iSample, 
		const IntersectionContext& iContext) const = 0;
	virtual const TRayTracerPtr doClone() const = 0;

	virtual const TPyObjectPtr doGetState() const = 0;
	virtual void doSetState(const TPyObjectPtr& iState) = 0;

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
	unsigned maxRayGeneration_;
	mutable unsigned rayGeneration_;
};

}

}

#endif

// EOF
