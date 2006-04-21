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

// --- public --------------------------------------------------------------------------------------

DirectLighting::DirectLighting():
	RayTracer(&Type)
{
}


// --- protected -----------------------------------------------------------------------------------

// --- private -------------------------------------------------------------------------------------

void DirectLighting::doPreprocess()
{
}



void DirectLighting::doRequestSamples(const TSamplerPtr& iSampler)
{
}



const Spectrum 
DirectLighting::doCastRay(const Sample& iSample, const DifferentialRay& iPrimaryRay) const
{
	Intersection intersection;
	scene()->intersect(iSample, iPrimaryRay, intersection);
	if (!intersection)
	{
		return Spectrum();
	}

	IntersectionContext context(this);
	intersection.object()->localContext(iSample, iPrimaryRay, intersection, context);
	if (!context.shader())
	{
		return Spectrum();
	}
	if (dot(iPrimaryRay.direction(), context.geometricNormal()) > 0)
	{
		context.flipNormal();
	}
	return context.shade(iSample, iPrimaryRay, intersection);
}



const TLightSamplesRange
DirectLighting::doSampleLights(const Sample& iSample, const TPoint3D& iTarget, 
		const TVector3D& iTargetNormal) const
{
	using namespace kernel;

	stde::overwrite_insert_iterator<TLightSamples> output(lightSamples_);

	const TLightContexts::const_iterator end = lights().end();
	for (TLightContexts::const_iterator light = lights().begin(); light != end; ++light)
	{
		Sample::TSubSequence2D subSequence = iSample.subSequence2D(light->idLightSamples());
		const TScalar scale = TNumTraits::one / subSequence.size();
		while (subSequence)
		{
			BoundedRay shadowRay;
			TScalar pdf;
			Spectrum radiance = light->sampleEmission(
				iSample, *subSequence, iTarget, iTargetNormal, shadowRay, pdf);
			if (pdf > 0 && !scene()->isIntersecting(iSample, shadowRay))
			{
				*output++ = LightSample((scale / pdf) * radiance, shadowRay.direction());
			}

			++subSequence;
		}
	}

	return TLightSamplesRange(lightSamples_.begin(), output.get());
}



const TRayTracerPtr DirectLighting::doClone() const
{
	return TRayTracerPtr(new DirectLighting(*this));
}



const TPyObjectPtr DirectLighting::doGetState() const
{
	return python::makeTuple(maxRayGeneration_);
}



void DirectLighting::doSetState(const TPyObjectPtr& iState)
{
	python::decodeTuple(iState, maxRayGeneration_);
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
