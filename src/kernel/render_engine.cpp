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
PY_CLASS_METHOD_QUALIFIED_0(RenderEngine, render, void)
PY_CLASS_METHOD_QUALIFIED_1(RenderEngine, render, void, TTime)
PY_CLASS_METHOD_QUALIFIED_1(RenderEngine, render, void, const RenderEngine::TBucket&)
PY_CLASS_METHOD_QUALIFIED_2(RenderEngine, render, void, TTime, const RenderEngine::TBucket&)


const RenderEngine::TBucket RenderEngine::bucketBound_(
    RenderEngine::TBucket::TPoint(TNumTraits::zero, TNumTraits::zero),
    RenderEngine::TBucket::TPoint(TNumTraits::one, TNumTraits::one));

// --- public --------------------------------------------------------------------------------------

RenderEngine::RenderEngine():
    PyObjectPlus(&Type)
{
}



RenderEngine::~RenderEngine()
{
    try
    {
        if (renderTarget_->isRendering())
        {
            renderTarget_->endRender();
        }
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



void RenderEngine::setCamera(const TCameraPtr& iCamera)
{
    camera_ = iCamera;
}



void RenderEngine::setSampler(const TSamplerPtr& iSampler)
{
    sampler_ = iSampler;
	rayTracer_->requestSamples(sampler_);
}



void RenderEngine::setScene(const TSceneObjectPtr& iScene)
{
    rayTracer_->setScene(iScene);
	rayTracer_->requestSamples(sampler_);
}



void RenderEngine::setTarget(const TRenderTargetPtr& iRenderTarget)
{
    renderTarget_ = iRenderTarget;
}



void RenderEngine::setTracer(const TRayTracerPtr& iRayTracer)
{
    rayTracer_ = iRayTracer;
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

	util::ProgressIndicator progress("rendering bucket " + util::stringCast<std::string>(iBucket));
    renderTarget_->beginRender();
	Sample sample;
    TResolution i;
    for (i.y = min.y; i.y < max.y; ++i.y)
    {
		progress(static_cast<double>(i.y) / max.y);
        for (i.x = min.x; i.x < max.x; ++i.x)
        {
            for (unsigned k = 0; k < samplesPerPixel; ++k)
            {
                sampler_->sample(i, k, timePeriod, sample);
                DifferentialRay primaryRay = camera_->primaryRay(sample, pixelSize);
                Spectrum radiance = rayTracer_->castRay(sample, primaryRay);
                renderTarget_->writeRender(sample, radiance);
            }
        }
    }
	progress(1.);
    renderTarget_->endRender();
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



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF