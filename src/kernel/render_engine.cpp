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

#include "kernel_common.h"
#include "render_engine.h"
#include <lass/io/keyboard.h>
#include <lass/util/progress_indicator.h>

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(RenderEngine, "Render engine")
PY_CLASS_CONSTRUCTOR_0(RenderEngine)
PY_CLASS_MEMBER_RW(RenderEngine, camera, setCamera)
PY_CLASS_MEMBER_RW(RenderEngine, tracer, setTracer)
PY_CLASS_MEMBER_RW(RenderEngine, target, setTarget)
PY_CLASS_MEMBER_RW(RenderEngine, sampler, setSampler)
PY_CLASS_MEMBER_RW(RenderEngine, scene, setScene)
PY_CLASS_MEMBER_RW(RenderEngine, numberOfThreads, setNumberOfThreads)
PY_CLASS_METHOD_QUALIFIED_0(RenderEngine, render, void)
PY_CLASS_METHOD_QUALIFIED_1(RenderEngine, render, void, TTime)
PY_CLASS_METHOD_QUALIFIED_1(RenderEngine, render, void, const RenderEngine::TBucket&)
PY_CLASS_METHOD_QUALIFIED_2(RenderEngine, render, void, TTime, const RenderEngine::TBucket&)
PY_CLASS_STATIC_CONST(RenderEngine, "AUTO_NUMBER_OF_THREADS", int(RenderEngine::autoNumberOfThreads));


const RenderEngine::TBucket RenderEngine::bucketBound_(
	RenderEngine::TBucket::TPoint(TNumTraits::zero, TNumTraits::zero),
	RenderEngine::TBucket::TPoint(TNumTraits::one, TNumTraits::one));

// --- public --------------------------------------------------------------------------------------

RenderEngine::RenderEngine():
	numberOfThreads_(autoNumberOfThreads),
	isDirty_(false)
{
}



RenderEngine::~RenderEngine()
try
{
	if (renderTarget_)
	{
		renderTarget_->endRender();
	}
}
catch(...)
{
}



const TCameraPtr& RenderEngine::camera() const
{
	return camera_;
}



const TSamplerPtr& RenderEngine::sampler() const
{
	return sampler_;
}



const TSceneObjectPtr& RenderEngine::scene() const
{
	return scene_;
}



const TRenderTargetPtr& RenderEngine::target() const
{
	return renderTarget_;
}



const TRayTracerPtr& RenderEngine::tracer() const
{
	return rayTracer_;
}



size_t RenderEngine::numberOfThreads() const
{
	return numberOfThreads_;
}



void RenderEngine::setCamera(const TCameraPtr& camera)
{
	camera_ = camera;
	isDirty_ = true;
}



void RenderEngine::setSampler(const TSamplerPtr& sampler)
{
	sampler_ = sampler;
	isDirty_ = true;
}



void RenderEngine::setScene(const TSceneObjectPtr& scene)
{
	scene_ = scene;
	isDirty_ = true;
}



void RenderEngine::setTarget(const TRenderTargetPtr& renderTarget)
{
	renderTarget_ = renderTarget;
}



void RenderEngine::setTracer(const TRayTracerPtr& iRayTracer)
{
	rayTracer_ = iRayTracer;
	isDirty_ = true;
}



void RenderEngine::setNumberOfThreads(size_t number)
{
	numberOfThreads_ = number;
}



void RenderEngine::render(TTime iFrameTime, const TBucket& bucket)
{
	if (!camera_)
	{
		LASS_THROW("can't render - no camera attached to engine.");
	}
	if (!rayTracer_)
	{
		LASS_THROW("can't render - no ray tracer attached to engine.");
	}
	if (!scene_)
	{
		LASS_THROW("can't render - no scene attached to engine.");
	}
	if (!sampler_)
	{
		LASS_THROW("can't render - no sampler attached to engine.");
	}
	if (!renderTarget_)
	{
		LASS_THROW("can't render - no render target attached to engine.");
	}
	if (!bucketBound_.contains(bucket))
	{
		LASS_THROW("can't render - render bucket '" << bucket 
			<< "' goes outside valid boundary '" << bucketBound_ << "'.");
	}
	
	const TResolution2D resolution = sampler_->resolution();
	const TVector2D pixelSize = TVector2D(resolution).reciprocal();
	const size_t samplesPerPixel = sampler_->samplesPerPixel();
	const TimePeriod timePeriod = iFrameTime + camera_->shutterDelta();

	const TResolution2D min(
		num::round(bucket.min().x * resolution.x), 
		num::round(bucket.min().y * resolution.y));
	const TResolution2D max(
		num::round(bucket.max().x * resolution.x), 
		num::round(bucket.max().y * resolution.y));
	const size_t numberOfPixels = (max.x - min.x) * (max.y - min.y);
	const size_t numberOfSamples = numberOfPixels * samplesPerPixel;

	if (isDirty_)
	{
		scene_->preProcess(scene_, timePeriod);
		rayTracer_->setScene(scene_);
		rayTracer_->requestSamples(sampler_);
		rayTracer_->preProcess(sampler_, timePeriod, numberOfThreads_);
		isDirty_ = false;
	}

	Progress progress("rendering bucket " + util::stringCast<std::string>(bucket), 
		numberOfSamples);

	Consumer consumer(*this, rayTracer_, sampler_, progress, pixelSize, timePeriod);
	if (numberOfThreads_ == 1)
	{
		Task task(min, max);
		consumer(task);
	}
	else
	{
		typedef util::ThreadPool<Task, Consumer> TThreadPool;
		TThreadPool pool(numberOfThreads_, TThreadPool::unlimitedNumberOfTasks, consumer);

		const TResolution2D::TValue step = 64;
		TResolution2D i;
		for (i.y = min.y; i.y < max.y; i.y += step)
		{
			for (i.x = min.x; i.x < max.x; i.x += step)
			{
				if (isCanceling())
				{
					pool.clearQueue();
					return;
				}

				TResolution2D end(std::min(i.x + step, max.x), std::min(i.y + step, max.y));
				pool.addTask(Task(i, end));
			}
		}

		// ~TThreadPool will wait for completion ...
	}
}



void RenderEngine::render(TTime iShutterOpen)
{
	render(iShutterOpen, bucketBound_);
}



void RenderEngine::render(const TBucket& bucket)
{
	render(0, bucket);
}



void RenderEngine::render()
{
	render(0, bucketBound_);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void RenderEngine::writeRender(const OutputSample* first, const OutputSample* last, 
		Progress& ioProgress)
{
	LASS_LOCK(lock_)
	{
		renderTarget_->writeRender(first, last);
		ioProgress += static_cast<unsigned>(last - first);
	}
}



bool RenderEngine::isCanceling() const
{
	return renderTarget_->isCanceling();
}



// --- Consumer ------------------------------------------------------------------------------------

RenderEngine::Consumer::Consumer(RenderEngine& iEngine, const TRayTracerPtr& iRayTracer,
		const TSamplerPtr& sampler, Progress& ioProgress,
		const TVector2D& iPixelSize, const TimePeriod& iTimePeriod):
	engine_(&iEngine),
	rayTracer_(LASS_ENFORCE_POINTER(iRayTracer)),
	sampler_(LASS_ENFORCE_POINTER(sampler)),
	progress_(&ioProgress),
	pixelSize_(iPixelSize),
	timePeriod_(iTimePeriod)
{
}



RenderEngine::Consumer::Consumer(const Consumer& other):
	engine_(other.engine_),
	rayTracer_(other.rayTracer_->clone()),
	sampler_(other.sampler_->clone()),
	progress_(other.progress_),
	pixelSize_(other.pixelSize_),
	timePeriod_(other.timePeriod_)
{
}



RenderEngine::Consumer& RenderEngine::Consumer::operator=(const Consumer& other)
{	
	engine_ = other.engine_;
	rayTracer_ = other.rayTracer_->clone();
	sampler_ = other.sampler_->clone();
	progress_ = other.progress_;
	pixelSize_ = other.pixelSize_;
	timePeriod_ = other.timePeriod_;
	return *this;
}



void RenderEngine::Consumer::operator()(const Task& iTask)
{
	const TResolution2D begin = iTask.begin();
	const TResolution2D end = iTask.end();
	const size_t samplesPerPixel = sampler_->samplesPerPixel();

	const size_t outputSize = 1024;
	TOutputSamples outputSamples(outputSize);
	size_t outputIndex = 0;

	Sample sample;
	TResolution2D i;
	for (i.y = begin.y; i.y < end.y; ++i.y)
	{
		for (i.x = begin.x; i.x < end.x; ++i.x)
		{
			for (size_t k = 0; k < samplesPerPixel; ++k)
			{
				if (engine_->isCanceling())
				{
					return;
				}

				sampler_->sample(i, k, timePeriod_, sample);
				const DifferentialRay primaryRay = engine_->camera_->primaryRay(sample, pixelSize_);
				sample.setWeight(engine_->camera_->weight(primaryRay));
				TScalar alpha, tIntersection;
				const XYZ radiance = rayTracer_->castRay(sample, primaryRay, tIntersection, alpha);
				const TScalar depth = engine_->camera_->asDepth(primaryRay, tIntersection);
				const XYZ freqFilter(3 * chromaticity(sample.frequency()));

				outputSamples[outputIndex++] = OutputSample(sample, radiance, depth, alpha);
				if (outputIndex == outputSize)
				{
					engine_->writeRender(&outputSamples[0], &outputSamples[0] + outputSize, *progress_);
					outputIndex = 0;
				}
			}
		}
	}

	engine_->writeRender(&outputSamples[0], &outputSamples[outputIndex], *progress_);
}



// --- Progress ------------------------------------------------------------------------------------

RenderEngine::Progress::Progress(const std::string& caption, size_t totalNumberOfSamples):
	indicator_(caption),
	numSamplesWritten_(0),
	totalNumSamples_(totalNumberOfSamples)
{
}



RenderEngine::Progress::~Progress()
{
	//indicator_(1.);
}



RenderEngine::Progress& RenderEngine::Progress::operator+=(size_t numNewSamplesWritten)
{
	numSamplesWritten_ += numNewSamplesWritten;
	indicator_(static_cast<double>(numSamplesWritten_) / totalNumSamples_);
	return *this;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
