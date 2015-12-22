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

#include "tracers_common.h"
#include "adjoint_photon_tracer.h"

namespace liar
{
namespace tracers
{

PY_DECLARE_CLASS_DOC(AdjointPhotonTracer, "Image Synthesis using Adjoint Photons")
PY_CLASS_CONSTRUCTOR_0(AdjointPhotonTracer)


// --- public --------------------------------------------------------------------------------------

AdjointPhotonTracer::AdjointPhotonTracer()
{
}


// --- protected -----------------------------------------------------------------------------------

// --- private -------------------------------------------------------------------------------------

void AdjointPhotonTracer::doRequestSamples(const kernel::TSamplerPtr& sampler)
{
	strategySample_.clear();
	bsdfSample_.clear();
	lightChoiceSample_.clear();
	lightSample_.clear();

	for (size_t k = 0; k < maxRayGeneration(); ++k)
	{
		strategySample_.push_back(sampler->requestSubSequence1D(1));
		bsdfSample_.push_back(sampler->requestSubSequence2D(1));
		lightChoiceSample_.push_back(sampler->requestSubSequence1D(1));
		lightSample_.push_back(sampler->requestSubSequence2D(1));
	}
}



void AdjointPhotonTracer::doPreProcess(const kernel::TSamplerPtr&, const TimePeriod&, size_t)
{
}



const Spectral AdjointPhotonTracer::doCastRay(
	const kernel::Sample& sample, const kernel::DifferentialRay& primaryRay,
	TScalar& tIntersection, TScalar& alpha, size_t generation, bool highQuality) const
{
	Spectral power(1);
	return tracePhoton(sample, power, primaryRay, tIntersection, alpha, generation);
}

const Spectral AdjointPhotonTracer::tracePhoton(const Sample& sample, const Spectral& power, const DifferentialRay& ray, TScalar& tIntersection, TScalar& alpha, size_t generation) const
{
	if (generation >= maxRayGeneration())
	{
		return Spectral(0);
	}

	num::DistributionUniform<TScalar, TRandom> uniform(random_);
	TScalar weight = 1;

	Intersection intersection;
	scene()->intersect(sample, ray, intersection);
	if (!intersection)
	{
		tIntersection = TNumTraits::infinity;
		alpha = 0;
		return Spectral(0);
	}
	tIntersection = intersection.t();
	alpha = 1;

	const TPoint3D hitPoint = ray.point(intersection.t());
	TScalar tScatter, pdf;
	const BoundedRay mediumRay = bound(ray.centralRay(), ray.nearLimit(), tIntersection);
	const Spectral transmittance = mediumStack().sampleScatterOutOrTransmittance(sample, uniform(), mediumRay, tScatter, pdf);
	Spectral transmittedPower = power * transmittance / static_cast<Spectral::TValue>(pdf);
	// RR to keep power constant (if possible)
	/*const TScalar transmittanceProbability = std::min<TScalar>(transmittedPower.absAverage() / power.absAverage(), TNumTraits::one);
	if (!russianRoulette(transmittedPower, transmittanceProbability, uniform()))
	{
		return Spectral(0);
	}*/

	if (tScatter < tIntersection)
	{
		const TPoint3D scatterPoint = ray.point(tScatter);
		TVector3D dirOut;
		TScalar pdfOut;
		const Spectral reflectance = mediumStack().samplePhase(sample, TPoint2D(uniform(), uniform()), scatterPoint, ray.direction(), dirOut, pdfOut);
		if (pdfOut <= 0 || !reflectance)
		{
			return Spectral(0);
		}
		Spectral scatteredPower = transmittedPower * reflectance / static_cast<Spectral::TValue>(pdfOut);
		const TScalar scatteredProbability = std::min<TScalar>(scatteredPower.absAverage() / transmittedPower.absAverage(), TNumTraits::one);
		if (!russianRoulette(scatteredPower, scatteredProbability, uniform()))
		{
			return Spectral(0);
		}
		const BoundedRay scatteredRay(scatterPoint, ray.direction());
		TScalar t, a;
		return tracePhoton(sample, scatteredPower, DifferentialRay(scatteredRay), t, a, generation + 1);
	}

	Spectral result;
	IntersectionContext context(*scene(), sample, ray, intersection, generation);
	const SceneLight* light = context.rayGeneration() > 0 ? context.object().asLight() : 0;

	if (light)
	{
		BoundedRay LASS_UNUSED(shadowRay);
		TScalar LASS_UNUSED(lightPdf); // ??
		const Spectral emission = light->emission(sample, ray.centralRay().unboundedRay(), shadowRay, lightPdf);
		result += transmittedPower * emission;
	}

	if (const Shader* const shader = context.shader())
	{
		shader->shadeContext(sample, context);

		const TVector3D omegaIn = context.worldToBsdf(-ray.direction());
		LASS_ASSERT(omegaIn.z > 0);

		if (!light)
		{
			result += transmittedPower * context.shader()->emission(sample, context, omegaIn);
		}

		const TBsdfPtr bsdf = shader->bsdf(sample, context);
		if (bsdf)
		{
			const TScalar strategyPdf = 0.5;
			if (*sample.subSequence1D(strategySample_[generation]) < strategyPdf)
			{
				const TPoint2D sampleBsdf = *sample.subSequence2D(bsdfSample_[generation]);
				const TScalar sampleComponent = uniform();
				const SampleBsdfOut out = bsdf->sample(omegaIn, sampleBsdf, sampleComponent, Bsdf::capsAll);
				if (out)
				{
					const TScalar cosTheta = out.omegaOut.z;
					Spectral newPower = transmittedPower * out.value * static_cast<Spectral::TValue>(num::abs(cosTheta) / (out.pdf * strategyPdf));
					const TScalar attenuation = newPower.average() / transmittedPower.average();
					//LASS_ASSERT(attenuation < 1.1);
					const TScalar scatterProbability = std::min(TNumTraits::one, attenuation);
					if (russianRoulette(newPower, scatterProbability, uniform()))
					{
						const BoundedRay newRay(hitPoint, context.bsdfToWorld(out.omegaOut));
						MediumChanger mediumChanger(mediumStack(), context.interior(), out.omegaOut.z < 0 ? context.solidEvent() : seNoEvent);
						TScalar t, a;
						result += tracePhoton(sample, newPower, DifferentialRay(newRay), t, a, generation + 1);
					}
				}
			}
			else
			{
				TScalar lightPdf;
				const LightContext* light = lights().sample(*sample.subSequence1D(lightChoiceSample_[generation]), lightPdf);
				if (light && lightPdf > 0)
				{
					const TPoint2D lightSample = *sample.subSequence2D(lightSample_[generation]);
					BoundedRay shadowRay;
					TScalar pdf;
					light->sampleEmission(sample, lightSample, hitPoint, context.worldNormal(), shadowRay, pdf);
					if (pdf > 0)
					{
						pdf *= lightPdf * (1 - strategyPdf);
						const TVector3D omegaOut = bsdf->worldToBsdf(shadowRay.direction());
						const BsdfOut out = bsdf->evaluate(omegaIn, omegaOut, Bsdf::capsAll);
						if (out)
						{
							const TScalar cosTheta = omegaOut.z;
							Spectral newPower = transmittedPower * out.value * static_cast<Spectral::TValue>(num::abs(cosTheta) / pdf);
							const TScalar attenuation = newPower.average() / transmittedPower.average();
							//LASS_ASSERT(attenuation < 1.1);
							const TScalar scatterProbability = std::min(TNumTraits::one, attenuation);
							if (russianRoulette(newPower, scatterProbability, uniform()))
							{
								const BoundedRay newRay(hitPoint, shadowRay.direction());
								MediumChanger mediumChanger(mediumStack(), context.interior(), omegaOut.z < 0 ? context.solidEvent() : seNoEvent);
								TScalar t, a;
								result += tracePhoton(sample, newPower, DifferentialRay(newRay), t, a, generation + 1);
							}
						}
					}
				}
			}
		}
	}
	else
	{
		// entering or leaving something ...

		MediumChanger mediumChanger(mediumStack(), context.interior(), context.solidEvent());
		const DifferentialRay newRay = bound(ray, intersection.t() + tolerance, ray.farLimit());
		TScalar t, a;
		result += tracePhoton(sample, transmittedPower, newRay, t, a, generation + 1);
	}
	
	return result;
}



const TRayTracerPtr AdjointPhotonTracer::doClone() const
{
	TRayTracerPtr::Rebind<AdjointPhotonTracer>::Type result(new AdjointPhotonTracer(*this));
	result->random_.seed(random_());
	return result;
}



const TPyObjectPtr AdjointPhotonTracer::doGetState() const
{
	return python::makeTuple();
}



void AdjointPhotonTracer::doSetState(const TPyObjectPtr&)
{
}



}
}

