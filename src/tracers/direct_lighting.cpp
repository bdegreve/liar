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



Spectrum DirectLighting::doCastRay(const Sample& iSample,
								   const DifferentialRay& iPrimaryRay) const
{
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
		Sample::TSubSequence2D subSequence = iSample.subSequence2D(light->idLightSamples());
		const TScalar scale = TNumTraits::one / subSequence.size();
		while (subSequence)
		{
			BoundedRay shadowRay;
			TScalar pdf;
			Spectrum radiance = light->sampleEmission(
				iSample, *subSequence, iContext.point(), shadowRay, pdf);
			if (!scene()->isIntersecting(iSample, shadowRay))
			{
				*output++ = LightSample(scale * radiance, shadowRay.direction(), pdf);
			}

			++subSequence;
		}
	}

	//oLightSamples.swap(result);
	return TLightRange(lightSamples_.begin(), output.get());
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
