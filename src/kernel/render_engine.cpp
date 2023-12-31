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

#include "kernel_common.h"
#include "render_engine.h"
#include "sampler_tiled.h"
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
PY_CLASS_METHOD_DOC(RenderEngine, seed, "seed the samplers and tracers with a 32 bit unsigned integer")


const RenderEngine::TBucket RenderEngine::bucketBound_(
	RenderEngine::TBucket::TPoint(TNumTraits::zero, TNumTraits::zero),
	RenderEngine::TBucket::TPoint(TNumTraits::one, TNumTraits::one));

// --- public --------------------------------------------------------------------------------------

RenderEngine::RenderEngine():
	numberOfThreads_(autoNumberOfThreads),
	isDirty_(false)
{
	seed(0);
}



RenderEngine::~RenderEngine()
{
try
{
	if (renderTarget_)
	{
		renderTarget_->endRender();
	}
}
	catch (...)
{
}
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

	const TimePeriod timePeriod = iFrameTime + camera_->shutterDelta();

	if (isDirty_)
	{
		scene_->preProcess(timePeriod);
		rayTracer_->setScene(scene_);
		rayTracer_->requestSamples(sampler_);
		rayTracer_->preProcess(sampler_, timePeriod, numberOfThreads_);
		isDirty_ = false;
	}

	// we need an unbounded progress indicator ...
	size_t numberOfSamples = num::NumTraits<size_t>::max;
	TVector2D sampleSize = TVector2D(renderTarget_->resolution()).reciprocal();
	if (SamplerTiled* sampler = dynamic_cast<SamplerTiled*>(sampler_.get()))
	{
		numberOfSamples = sampler->resolution().x * sampler->resolution().y * sampler->samplesPerPixel();
		sampleSize /= num::sqrt(TScalar(sampler->samplesPerPixel()));
	}
	else
	{
		// sampler is unbounded, we don't know how many samples we'll take per pixel. So just take a guess.
		sampleSize /= 10;
	}
	Progress progress("rendering bucket " + util::stringCast<std::string>(bucket), numberOfSamples);

	sampler_->setBucket(bucket); // bit unorthodox, since we modify sampler here.
	Consumer consumer(*this, rayTracer_, sampler_, progress, sampleSize, timePeriod);

	typedef Sampler::TTaskPtr TTaskPtr;
	typedef util::ThreadPool<TTaskPtr, Consumer> TThreadPool;

	TThreadPool pool(numberOfThreads_, std::max<size_t>(2 * numberOfThreads_, 16), consumer);

	renderTarget_->beginRender();
	TTaskPtr task = sampler_->getTask();
	while (task)
	{
		pool.addTask(task);
		task = sampler_->getTask();

		if (isCanceling())
		{
			pool.clearQueue();
			return;
		}
	}

	// ~TThreadPool will wait for completion ...
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



void RenderEngine::seed(num::Tuint32 seed)
{
	seedGenerator_.seed(seed ^ 0xbad5eed);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void RenderEngine::writeRender(const OutputSample* first, const OutputSample* last,
		Progress& ioProgress)
{
	renderTarget_->writeRender(first, last);
	ioProgress += static_cast<unsigned>(last - first);
}



bool RenderEngine::isCanceling() const
{
	return renderTarget_->isCanceling();
}



// --- Consumer ------------------------------------------------------------------------------------

RenderEngine::Consumer::Consumer(
		RenderEngine& engine, const TRayTracerPtr& rayTracer, const TSamplerPtr& sampler,
		Progress& progress, const TVector2D& sampleSize, const TimePeriod& timePeriod):
	engine_(&engine),
	rayTracer_(LASS_ENFORCE_POINTER(rayTracer)),
	sampler_(LASS_ENFORCE_POINTER(sampler)),
	progress_(&progress),
	sampleSize_(sampleSize),
	timePeriod_(timePeriod)
{
}



RenderEngine::Consumer::Consumer(const Consumer& other):
	engine_(other.engine_),
	rayTracer_(other.rayTracer_->clone()),
	sampler_(other.sampler_->clone()),
	progress_(other.progress_),
	sampleSize_(other.sampleSize_),
	timePeriod_(other.timePeriod_)
{
	rayTracer_->seed(static_cast<RayTracer::TSeed>(engine_->seedGenerator_()));
	sampler_->seed(static_cast<Sampler::TSeed>(engine_->seedGenerator_()));
}



void RenderEngine::Consumer::operator()(const Sampler::TTaskPtr& task)
{
	typedef OutputSample::TValue TValue;

	const size_t outputSize = 1024;
	TOutputSamples outputSamples(outputSize);
	size_t outputIndex = 0;

	Sample sample;
	while (task->drawSample(*sampler_, timePeriod_, sample))
	{
		if (engine_->isCanceling())
		{
			return;
		}

		const DifferentialRay primaryRay = engine_->camera_->primaryRay(sample, sampleSize_);
		sample.setWeight(engine_->camera_->weight(primaryRay));
		TScalar alpha;
		TScalar tIntersection;
		const Spectral radiance = rayTracer_->castRay(sample, primaryRay, tIntersection, alpha);
		const TScalar depth = engine_->camera_->asDepth(primaryRay, tIntersection);

		outputSamples[outputIndex++] = OutputSample(sample, radiance, static_cast<TValue>(depth), static_cast<TValue>(alpha));
		if (outputIndex == outputSize)
		{
			engine_->writeRender(&outputSamples[0], &outputSamples[0] + outputSize, *progress_);
			outputIndex = 0;
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
	std::lock_guard<std::mutex> lock(mutex_);
	numSamplesWritten_ += numNewSamplesWritten;
	indicator_(static_cast<double>(numSamplesWritten_) / static_cast<double>(totalNumSamples_));
	return *this;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
