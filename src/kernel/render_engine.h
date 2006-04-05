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

/** @class liar::RenderEngine
 *  @brief keeps the ray tracer running
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_RENDER_ENGINE_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_RENDER_ENGINE_H

#include "kernel_common.h"
#include "camera.h"
#include "object.h"
#include "render_target.h"
#include "sampler.h"
#include "ray_tracer.h"

#include <lass/prim/aabb_2d.h>
#include <lass/util/progress_indicator.h>
#include <lass/util/thread_pool.h>

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL RenderEngine: public python::PyObjectPlus
{
    PY_HEADER(python::PyObjectPlus)
public:

    typedef prim::Aabb2D<TScalar> TBucket;

    RenderEngine();
    ~RenderEngine();

    const TCameraPtr& camera() const;
    const TSamplerPtr& sampler() const;
    const TSceneObjectPtr& scene() const;
    const TRenderTargetPtr& target() const;
    const TRayTracerPtr& tracer() const;
	const unsigned numberOfThreads() const;

    void setCamera(const TCameraPtr& iCamera);
    void setSampler(const TSamplerPtr& iSampler);
    void setScene(const TSceneObjectPtr& iScene);
    void setTarget(const TRenderTargetPtr& iRenderTarget);
    void setTracer(const TRayTracerPtr& iRayTracer);
	void setNumberOfThreads(unsigned iNumber);

    void render(TTime iTime, const TBucket& iBucket);
    void render(TTime iTime);
    void render(const TBucket& iBucket);
    void render();

private:

	class Progress
	{
	public:
		Progress(const std::string& iCaption, unsigned iTotalNumberOfSamples);
		~Progress();
		Progress& operator+=(unsigned iNumNewSamplesWritten);
	private:
		util::ProgressIndicator indicator_;
		unsigned numSamplesWritten_;
		unsigned totalNumSamples_;
	};

	class Task
	{
	public:
		typedef Sampler::TResolution TResolution;
		Task() {}
		Task(const TResolution& iBegin, const TResolution& iEnd): begin_(iBegin), end_(iEnd) {}
		const TResolution& begin() const { return begin_; }
		const TResolution& end() const { return end_; }
	private:
		TResolution begin_;
		TResolution end_;
	};

	class Consumer
	{
	public:
		Consumer(RenderEngine& iEngine, const TRayTracerPtr& iRayTracer,
				const TSamplerPtr& iSampler, Progress& ioProgress,
				const TVector2D& iPixelSize, const TimePeriod& iTimePeriod);
		Consumer(const Consumer& iOther);
		Consumer& operator=(const Consumer& iOther);
		void operator()(const Task& iTask);
	private:
		RenderEngine* engine_;
		TRayTracerPtr rayTracer_;
		TSamplerPtr sampler_;
		Progress* progress_;
		TVector2D pixelSize_;
		TimePeriod timePeriod_;
	};

	friend class Consumer;

	void writeRender(const OutputSample* iFirst, const OutputSample* iLast, Progress& ioProgress);
	const bool isCanceling() const;

    TCameraPtr camera_;
    TRayTracerPtr rayTracer_;
    TRenderTargetPtr renderTarget_;
    TSamplerPtr sampler_;
    TSceneObjectPtr scene_;
	util::CriticalSection mutex_;
	util::Condition signal_;
	unsigned numberOfThreads_;
	bool isDirty_;

    static const TBucket bucketBound_;
};

}

}

#endif

// EOF
