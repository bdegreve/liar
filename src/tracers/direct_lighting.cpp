/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
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
 *  http://liar.sourceforge.net
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

DirectLighting::DirectLighting()
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
DirectLighting::doCastRay(const Sample& sample, const DifferentialRay& primaryRay) const
{
	Intersection intersection;
	scene()->intersect(sample, primaryRay, intersection);
	if (!intersection)
	{
		return Spectrum();
	}

	IntersectionContext context(this);
	intersection.object()->localContext(sample, primaryRay, intersection, context);
	
	const Shader* shader = context.shader();
	if (!shader)
	{
		return Spectrum();
	}

	const TVector3D dirOut = -primaryRay.direction();

#pragma LASS_FIXME("dirOut and geometricNormal are not in the same space")
	if (dot(dirOut, intersectionContext.geometricNormal()) < 0)
	{
		intersectionContext.flipNormal();
	}

	Spectrum result = shader->emission(sample, context, dirOut);

	const TLightContexts::const_iterator end = lights().end();
	for (TLightContexts::const_iterator light = lights().begin(); light != end; ++light)
	{
		Sample::TSubSequence2D lightSample = sample.subSequence2D(light->idLightSamples());
		Sample::TSubSequence2D bsdfSample = sample.subSequence2D(light->idBsdfSamples());
		Sample::TSubSequence1D componentSample = sample.subSequence1D(light->idBsdfComponentSamples());
		LASS_ASSERT(bsdfSample.size() == lightSample.size() && componentSample.size() == lightSample.size());
		const TScalar n = static_cast<TScalar>(lightSample.size());
		while (lightSample)
		{
			const Spectrum radiance = light->sampleEmission(
				sample, *lightSample, iTarget, iTargetNormal, shadowRay, lightPdf);
			if (lightPdf > 0 && radiance)
			{
				const TVector3D dirLight = shadowRay.direction();
				TSCalar bsdfPdf;
				Spectrum bsdf = *BSDF*->bsdf(dirEye, dirLight, bsdfPdf)
				if (bsdf && !scene()->isIntersecting(sample, shadowRay))
				{
					const TScalar weight = light->isDelta() ?
						TNumTraits::one : powerHeuristic(1, lightPdf, 1, bsdfPdf);
					result += bsdf * radiance * 
						(weight * abs(dot(omegaIn.z)) / (n * lightPdf));
				}
				if (!light->isDelta())
				{
					*BSDF*->sampleBsdf(sample, *bsdfSample, *componentSample, 
						
				}


			++lightSample;
			++brdfSample;
			++componentSample;
		}
	}
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
