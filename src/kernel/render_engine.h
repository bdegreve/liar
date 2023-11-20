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

/** @class liar::RenderEngine
 *  @brief keeps the ray tracer running
 *  @author Bram de Greve [Bramz]
 */

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

	enum { autoNumberOfThreads = util::ThreadPool<>::autoNumberOfThreads };

	RenderEngine();
	~RenderEngine();

	const TCameraPtr& camera() const;
	const TSamplerPtr& sampler() const;
	const TSceneObjectPtr& scene() const;
	const TRenderTargetPtr& target() const;
	const TRayTracerPtr& tracer() const;
	size_t numberOfThreads() const;

	void setCamera(const TCameraPtr& camera);
	void setSampler(const TSamplerPtr& sampler);
	void setScene(const TSceneObjectPtr& scene);
	void setTarget(const TRenderTargetPtr& renderTarget);
	void setTracer(const TRayTracerPtr& iRayTracer);
	void setNumberOfThreads(size_t number);

	void render(TTime time, const TBucket& bucket);
	void render(TTime time);
	void render(const TBucket& bucket);
	void render();

private:

	class Progress
	{
	public:
		Progress(const std::string& caption, size_t totalNumberOfSamples);
		~Progress();
		Progress& operator+=(size_t numNewSamplesWritten);
	private:
		util::ProgressIndicator indicator_;
		std::mutex mutex_;
		size_t numSamplesWritten_;
		size_t totalNumSamples_;
	};

	class Consumer
	{
	public:
		Consumer(RenderEngine& engine, const TRayTracerPtr& rayTracer, const TSamplerPtr& sampler,
			Progress& progress, const TVector2D& pixelSize, const TimePeriod& timePeriod);
		Consumer(const Consumer& other);
		Consumer& operator=(const Consumer& other);
		void operator()(const Sampler::TTaskPtr& task);
	private:
		RenderEngine* engine_;
		TRayTracerPtr rayTracer_;
		TSamplerPtr sampler_;
		Progress* progress_;
		TVector2D pixelSize_;
		TVector2D sampleSize_;
		TimePeriod timePeriod_;
	};

	friend class Consumer;

	void writeRender(const OutputSample* first, const OutputSample* last, Progress& ioProgress);
	bool isCanceling() const;

	TCameraPtr camera_;
	TRayTracerPtr rayTracer_;
	TRenderTargetPtr renderTarget_;
	TSamplerPtr sampler_;
	TSceneObjectPtr scene_;
	size_t numberOfThreads_;
	bool isDirty_;

	static const TBucket bucketBound_;
};

}

}

#endif

// EOF
