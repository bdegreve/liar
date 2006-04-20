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

#include "kernel_common.h"
#include "render_engine.h"
#include <lass/io/keyboard.h>
#include <lass/util/progress_indicator.h>

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS(RenderEngine)
PY_CLASS_CONSTRUCTOR_0(RenderEngine)
PY_CLASS_MEMBER_RW(RenderEngine, "camera", camera, setCamera)
PY_CLASS_MEMBER_RW(RenderEngine, "tracer", tracer, setTracer)
PY_CLASS_MEMBER_RW(RenderEngine, "target", target, setTarget)
PY_CLASS_MEMBER_RW(RenderEngine, "sampler", sampler, setSampler)
PY_CLASS_MEMBER_RW(RenderEngine, "scene", scene, setScene)
PY_CLASS_MEMBER_RW(RenderEngine, "numberOfThreads", numberOfThreads, setNumberOfThreads)
PY_CLASS_METHOD_QUALIFIED_0(RenderEngine, render, void)
PY_CLASS_METHOD_QUALIFIED_1(RenderEngine, render, void, TTime)
PY_CLASS_METHOD_QUALIFIED_1(RenderEngine, render, void, const RenderEngine::TBucket&)
PY_CLASS_METHOD_QUALIFIED_2(RenderEngine, render, void, TTime, const RenderEngine::TBucket&)


const RenderEngine::TBucket RenderEngine::bucketBound_(
    RenderEngine::TBucket::TPoint(TNumTraits::zero, TNumTraits::zero),
    RenderEngine::TBucket::TPoint(TNumTraits::one, TNumTraits::one));

// --- public --------------------------------------------------------------------------------------

RenderEngine::RenderEngine():
    PyObjectPlus(&Type),
	numberOfThreads_(1),
	isDirty_(false)
{
}



RenderEngine::~RenderEngine()
{
    try
    {
        renderTarget_->endRender();
    }
    catch(...)
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
    return rayTracer_->scene();
}



const TRenderTargetPtr& RenderEngine::target() const
{
    return renderTarget_;
}



const TRayTracerPtr& RenderEngine::tracer() const
{
    return rayTracer_;
}



const unsigned RenderEngine::numberOfThreads() const
{
    return numberOfThreads_;
}



void RenderEngine::setCamera(const TCameraPtr& iCamera)
{
    camera_ = iCamera;
	isDirty_ = true;
}



void RenderEngine::setSampler(const TSamplerPtr& iSampler)
{
    sampler_ = iSampler;
	isDirty_ = true;
}



void RenderEngine::setScene(const TSceneObjectPtr& iScene)
{
    scene_ = iScene;
	isDirty_ = true;
}



void RenderEngine::setTarget(const TRenderTargetPtr& iRenderTarget)
{
    renderTarget_ = iRenderTarget;
}



void RenderEngine::setTracer(const TRayTracerPtr& iRayTracer)
{
    rayTracer_ = iRayTracer;
	isDirty_ = true;
}



void RenderEngine::setNumberOfThreads(unsigned iNumber)
{
	numberOfThreads_ = std::max<unsigned>(iNumber, 1);
}



void RenderEngine::render(TTime iFrameTime, const TBucket& iBucket)
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
    if (!bucketBound_.contains(iBucket))
    {
        LASS_THROW("can't render - render bucket '" << iBucket << "' goes outside valid boundary '"
            << bucketBound_ << "'.");
    }
	
    typedef Sampler::TResolution TResolution;

    const TResolution resolution = sampler_->resolution();
    const TVector2D pixelSize = TVector2D(resolution).reciprocal();
    const unsigned samplesPerPixel = sampler_->samplesPerPixel();
	const TimePeriod timePeriod = iFrameTime + camera_->shutterDelta();

    const TResolution min(num::round(iBucket.min().x * resolution.x), num::round(iBucket.min().y * resolution.y));
    const TResolution max(num::round(iBucket.max().x * resolution.x), num::round(iBucket.max().y * resolution.y));
	const unsigned numberOfPixels = (max.x - min.x) * (max.y - min.y);
	const unsigned numberOfSamples = numberOfPixels * samplesPerPixel;


	if (isDirty_)
	{
		
		scene_->preProcess(timePeriod);
		rayTracer_->setScene(scene_);
		rayTracer_->requestSamples(sampler_);
		isDirty_ = false;
	}

	Progress progress("rendering bucket " + util::stringCast<std::string>(iBucket), 
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
		TThreadPool pool(numberOfThreads_, 2 * numberOfThreads_, consumer);

		const TResolution::TValue step = 64;
		TResolution i;
		for (i.y = min.y; i.y < max.y; i.y += step)
		{
			for (i.x = min.x; i.x < max.x; i.x += step)
			{
				if (isCanceling())
				{
					pool.clearQueue();
					return;
				}

				TResolution end(std::min(i.x + step, max.x), std::min(i.y + step, max.y));
				pool.add(Task(i, end));
			}
		}

		// ~TThreadPool will wait for completion ...
	}
}



void RenderEngine::render(TTime iShutterOpen)
{
    render(iShutterOpen, bucketBound_);
}



void RenderEngine::render(const TBucket& iBucket)
{
    render(0, iBucket);
}



void RenderEngine::render()
{
    render(0, bucketBound_);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void RenderEngine::writeRender(const OutputSample* iFirst, const OutputSample* iLast, 
		Progress& ioProgress)
{
	LASS_LOCK(mutex_)
	{
		renderTarget_->writeRender(iFirst, iLast);
		ioProgress += static_cast<unsigned>(iLast - iFirst);
	}
}



const bool RenderEngine::isCanceling() const
{
	return renderTarget_->isCanceling();
}



// --- Consumer ------------------------------------------------------------------------------------

RenderEngine::Consumer::Consumer(RenderEngine& iEngine, const TRayTracerPtr& iRayTracer,
		const TSamplerPtr& iSampler, Progress& ioProgress,
		const TVector2D& iPixelSize, const TimePeriod& iTimePeriod):
	engine_(&iEngine),
	rayTracer_(LASS_ENFORCE_POINTER(iRayTracer)),
	sampler_(LASS_ENFORCE_POINTER(iSampler)),
	progress_(&ioProgress),
	pixelSize_(iPixelSize),
	timePeriod_(iTimePeriod)
{
}



RenderEngine::Consumer::Consumer(const Consumer& iOther):
	engine_(iOther.engine_),
	rayTracer_(iOther.rayTracer_->clone()),
	sampler_(iOther.sampler_->clone()),
	progress_(iOther.progress_),
	pixelSize_(iOther.pixelSize_),
	timePeriod_(iOther.timePeriod_)
{
}			



RenderEngine::Consumer& RenderEngine::Consumer::operator=(const Consumer& iOther)
{	
	engine_ = iOther.engine_;
	rayTracer_ = iOther.rayTracer_->clone();
	sampler_ = iOther.sampler_->clone();
	progress_ = iOther.progress_;
	pixelSize_ = iOther.pixelSize_;
	timePeriod_ = iOther.timePeriod_;
	return *this;
}			
	


void RenderEngine::Consumer::operator()(const Task& iTask)
{
	typedef Sampler::TResolution TResolution;

	const TResolution begin = iTask.begin();
	const TResolution end = iTask.end();
	const unsigned samplesPerPixel = sampler_->samplesPerPixel();

	const unsigned outputSize = 1024;
	TOutputSamples outputSamples(outputSize);
	unsigned outputIndex = 0;

	Sample sample;
	TResolution i;
	for (i.y = begin.y; i.y < end.y; ++i.y)
	{
		for (i.x = begin.x; i.x < end.x; ++i.x)
		{
			for (unsigned k = 0; k < samplesPerPixel; ++k)
			{
				if (engine_->isCanceling())
				{
					return;
				}

				sampler_->sample(i, k, timePeriod_, sample);
				const DifferentialRay primaryRay = engine_->camera_->primaryRay(sample, pixelSize_);
				const Spectrum radiance = rayTracer_->castRay(sample, primaryRay);

				outputSamples[outputIndex++] = OutputSample(sample, radiance);
				if (outputIndex == outputSize)
				{
					engine_->writeRender(&outputSamples[0], &outputSamples[outputSize], *progress_);
					outputIndex = 0;
				}
			}
		}
	}

	engine_->writeRender(&outputSamples[0], &outputSamples[outputIndex], *progress_);
}



// --- Progress ------------------------------------------------------------------------------------

RenderEngine::Progress::Progress(const std::string& iCaption, unsigned iTotalNumberOfSamples):
	indicator_(iCaption),
	numSamplesWritten_(0),
	totalNumSamples_(iTotalNumberOfSamples)
{
}



RenderEngine::Progress::~Progress()
{
	indicator_(1.);
}



RenderEngine::Progress& RenderEngine::Progress::operator+=(unsigned iNumNewSamplesWritten)
{
	numSamplesWritten_ += iNumNewSamplesWritten;
	indicator_(static_cast<double>(numSamplesWritten_) / totalNumSamples_);
	return *this;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF