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
 */

#include "tracers_common.h"
#include "direct_lighting.h"
#include <lass/stde/extended_iterator.h>

namespace liar
{
namespace tracers
{

PY_DECLARE_CLASS(DirectLighting)
PY_CLASS_CONSTRUCTOR_0(DirectLighting)
PY_CLASS_MEMBER_RW(DirectLighting, "maxRayGeneration", maxRayGeneration, setMaxRayGeneration)

// --- public --------------------------------------------------------------------------------------

DirectLighting::DirectLighting():
	RayTracer(&Type),
	maxRayGeneration_(32),
	rayGeneration_(0)
{
}



const size_t DirectLighting::maxRayGeneration() const
{
	return maxRayGeneration_;
}



void DirectLighting::setMaxRayGeneration(size_t iMaxRayGeneration)
{
	maxRayGeneration_ = iMaxRayGeneration;
}


// --- protected -----------------------------------------------------------------------------------

// --- private -------------------------------------------------------------------------------------

void DirectLighting::doPreprocess()
{
}



void DirectLighting::doRequestSamples(const TSamplerPtr& iSampler)
{
}



Spectrum DirectLighting::doCastRay(const Sample& iSample,
										   const DifferentialRay& iPrimaryRay) const
{
	GenerationIncrementor incrementor(*this);
	if (rayGeneration_ > maxRayGeneration_)
	{
		return Spectrum();
	}

	Intersection intersection;
	scene()->intersect(iSample, iPrimaryRay, intersection);
	if (!intersection)
	{
		return Spectrum();
	}

	IntersectionContext context;
	intersection.object()->localContext(iSample, iPrimaryRay, intersection, context);
	if (dot(iPrimaryRay.direction(), context.geometricNormal()) > 0)
	{
		context.flipNormal();
	}

	return context.shader()->shade(iSample, iPrimaryRay, intersection, context, *this);
}



DirectLighting::TLightRange DirectLighting::doSampleLights(const Sample& iSample,
									const IntersectionContext& iContext) const
{
	using namespace kernel;

	//TLightSamples result;
	stde::overwrite_insert_iterator<TLightSamples> output(lightSamples_);

	const TLightContexts::const_iterator end = lights().end();
	for (TLightContexts::const_iterator light = lights().begin(); light != end; ++light)
	{
		for (Sample::TSubSequence2D lightSample = iSample.subSequence2D(light->idLightSamples()); lightSample; ++lightSample)
		{
			BoundedRay shadowRay;
			Spectrum radiance = light->sampleRadiance(
				*lightSample, iContext.point(), iContext.t(), shadowRay);
			if (!scene()->isIntersecting(iSample, shadowRay))
			{
				*output++ = LightSample(radiance, shadowRay.direction());
			}
		}
	}

	//oLightSamples.swap(result);
	return TLightRange(lightSamples_.begin(), output.get());
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
