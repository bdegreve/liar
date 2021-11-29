/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2021  Bram de Greve (bramz@users.sourceforge.net)
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
	bsdfComponentSample_.clear();
	lightChoiceSample_.clear();
	lightSample_.clear();
	scatterSample_.clear();
	scatterPhaseSample_.clear();

	for (size_t k = 0; k < maxRayGeneration(); ++k)
	{
		strategySample_.push_back(sampler->requestSubSequence1D(1));
		bsdfSample_.push_back(sampler->requestSubSequence2D(1));
		bsdfComponentSample_.push_back(sampler->requestSubSequence1D(1));
		lightChoiceSample_.push_back(sampler->requestSubSequence1D(1));
		lightSample_.push_back(sampler->requestSubSequence2D(1));
		scatterSample_.push_back(sampler->requestSubSequence1D(1));
		scatterPhaseSample_.push_back(sampler->requestSubSequence2D(1));
	}
}



void AdjointPhotonTracer::doPreProcess(const kernel::TSamplerPtr&, const TimePeriod&, size_t)
{
}



const Spectral AdjointPhotonTracer::doCastRay(
	const kernel::Sample& sample, const kernel::DifferentialRay& primaryRay,
	TScalar& tIntersection, TScalar& alpha, size_t generation, bool /*highQuality*/) const
{
	return tracePhoton(sample, primaryRay, tIntersection, alpha, generation);
}

const Spectral AdjointPhotonTracer::tracePhoton(const Sample& sample, const DifferentialRay& ray, TScalar& tIntersection, TScalar& alpha, size_t generation) const
{
	Spectral power(1);

	if (generation >= maxRayGeneration())
	{
		// play russian roulette to see if we continue.
		std::uniform_real_distribution<TScalar> uniform;
		const TScalar survivalPdf = 0.5;
		if (uniform(random_) < survivalPdf)
		{
			power /= survivalPdf;
		}
		else
		{
			return Spectral(0);
		}	
	}

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

	TScalar tScatter, pdf;
	const BoundedRay mediumRay = bound(ray.centralRay(), ray.nearLimit(), tIntersection);
	const Spectral transmittance = mediumStack().sampleScatterOutOrTransmittance(
		sample, scatterSample(sample, generation), mediumRay, tScatter, pdf);
	power *= transmittance / static_cast<Spectral::TValue>(pdf);

	if (tScatter < tIntersection)
	{
		const TPoint3D scatterPoint = ray.point(tScatter);
		TVector3D dirOut;
		TScalar pdfOut;
		const Spectral reflectance = mediumStack().samplePhase(
			sample, scatterPhaseSample(sample, generation), scatterPoint, ray.direction(), dirOut, pdfOut);
		if (pdfOut <= 0 || !reflectance)
		{
			return Spectral(0);
		}
		power *= reflectance / static_cast<Spectral::TValue>(pdfOut);
		const BoundedRay scatteredRay(scatterPoint, dirOut);
		TScalar t, a;
		return power * tracePhoton(sample, DifferentialRay(scatteredRay), t, a, generation + 1);
	}

	return power * shadeSurface(sample, ray, intersection, generation);
}

const Spectral AdjointPhotonTracer::shadeSurface(const Sample& sample, const DifferentialRay& ray, const Intersection& intersection, size_t generation) const
{
	Spectral result;

	IntersectionContext context(*scene(), sample, ray, intersection, generation);
	const SceneLight* light = generation > 0 ? context.object().asLight() : 0;
	if (light)
	{
		BoundedRay LASS_UNUSED(shadowRay);
		TScalar LASS_UNUSED(lightPdf); // ??
		const Spectral emission = light->emission(sample, ray.centralRay().unboundedRay(), shadowRay, lightPdf);
		result += emission;
	}

	
	if (const Shader* const shader = context.shader())
	{
		const TVector3D omegaIn = context.worldToBsdf(-ray.direction());
		LASS_ASSERT(omegaIn.z > 0);

		if (!light)
		{
			result += shader->emission(sample, context, omegaIn);
		}
		const TBsdfPtr bsdf = shader->bsdf(sample, context);
		if (bsdf)
		{
			const TPoint3D target = ray.point(intersection.t());
			SampleBsdfOut out = scatterSurface(sample, bsdf, target, omegaIn, generation);
			if (out)
			{
				const TScalar cosTheta = out.omegaOut.z;
				MediumChanger mediumChanger(mediumStack(), context.interior(),
					cosTheta > 0 ? seNoEvent : context.solidEvent());

				const TPoint3D start = target + (cosTheta > 0 ? 10 : -10) * liar::tolerance * context.worldGeometricNormal();
				const BoundedRay newRay(start, context.bsdfToWorld(out.omegaOut), liar::tolerance);
				TScalar t, a;
				out.value *= tracePhoton(sample, DifferentialRay(newRay), t, a, generation + 1);

				result.fma(out.value, static_cast<Spectral::TValue>(num::abs(cosTheta) / out.pdf));
			}
		}
	}
	else
	{
		// entering or leaving something ...
		MediumChanger mediumChanger(mediumStack(), context.interior(), context.solidEvent());
		const DifferentialRay newRay = bound(ray, intersection.t() + tolerance, ray.farLimit());
		TScalar t, a;
		result += tracePhoton(sample, newRay, t, a, generation + 1);
	}

	return result;
}



SampleBsdfOut AdjointPhotonTracer::scatterSurface(const Sample& sample, const TBsdfPtr& bsdf, const TPoint3D& target, const TVector3D& omegaIn, size_t generation) const
{
	LIAR_ASSERT(num::abs(omegaIn.norm() - 1) < 1e-6f, "omegaIn=" << omegaIn)

	const TScalar strategyPdf = bsdf->hasCaps(BsdfCaps::diffuse) ? 0.5f : 1.0f;
	if (strategySample(sample, generation) < strategyPdf)
	{
		// sample BSDF.
		SampleBsdfOut out = bsdf->sample(
			omegaIn, bsdfSample(sample, generation), bsdfComponentSample(sample, generation), BsdfCaps::all);
		if (!out)
		{
			return out;
		}
		out.pdf *= strategyPdf;

		// linear search through the lights. So this will only work great if there aren't too many.
		TRay3D ray(target, bsdf->bsdfToWorld(out.omegaOut));
		for (const LightContext& light : lights())
		{
			TScalar pdfLight;
			BoundedRay shadowRay;
			light.emission(sample, ray, shadowRay, pdfLight);
			if (pdfLight > 0)
			{
				out.pdf += (1 - strategyPdf) * lights().pdf(&light) * pdfLight;
			}
		}

		return out;
	}
	else
	{
		TScalar lightPdf;
		const LightContext* light = lights().sample(lightChoiceSample(sample, generation), lightPdf);
		if (!light || lightPdf <= 0)
		{
			return SampleBsdfOut();
		}

		BoundedRay shadowRay;
		TScalar pdf;
		light->sampleEmission(sample, lightSample(sample, generation), target, shadowRay, pdf);
		if (pdf <= 0)
		{
			return SampleBsdfOut();
		}
		pdf *= lightPdf * (1 - strategyPdf);

		const TVector3D omegaOut = bsdf->worldToBsdf(shadowRay.direction());
		LIAR_ASSERT(num::abs(omegaOut.norm() - 1) < 1e-6f, 
			"omegaOut=" << omegaIn << ", shadowRay.direction()=" << shadowRay.direction())
		const BsdfOut out = bsdf->evaluate(omegaIn, omegaOut, BsdfCaps::all);
		if (!out)
		{
			return SampleBsdfOut();
		}
		pdf += out.pdf * strategyPdf;

		return SampleBsdfOut(omegaOut, out.value, pdf, BsdfCaps::all);
	}
}


Sample::TSample1D AdjointPhotonTracer::strategySample(const Sample& sample, size_t generation) const
{
	if (generation < maxRayGeneration())
	{
		return *sample.subSequence1D(strategySample_[generation]);
	}
	std::uniform_real_distribution<Sample::TSample1D> uniform;
	return uniform(random_);
}


Sample::TSample2D AdjointPhotonTracer::bsdfSample(const Sample& sample, size_t generation) const
{
	if (generation < maxRayGeneration())
	{
		return *sample.subSequence2D(bsdfSample_[generation]);
	}
	std::uniform_real_distribution<Sample::TSample2D::TValue> uniform;
	return Sample::TSample2D(uniform(random_), uniform(random_));
}


Sample::TSample1D AdjointPhotonTracer::bsdfComponentSample(const Sample& sample, size_t generation) const
{
	if (generation < maxRayGeneration())
	{
		return *sample.subSequence1D(bsdfComponentSample_[generation]);
	}
	std::uniform_real_distribution<Sample::TSample1D> uniform;
	return uniform(random_);
}


Sample::TSample1D AdjointPhotonTracer::lightChoiceSample(const Sample& sample, size_t generation) const
{
	if (generation < maxRayGeneration())
	{
		return *sample.subSequence1D(lightChoiceSample_[generation]);
	}
	std::uniform_real_distribution<Sample::TSample1D> uniform;
	return uniform(random_);
}


Sample::TSample2D AdjointPhotonTracer::lightSample(const Sample& sample, size_t generation) const
{
	if (generation < maxRayGeneration())
	{
		return *sample.subSequence2D(lightSample_[generation]);
	}
	std::uniform_real_distribution<Sample::TSample2D::TValue> uniform;
	return Sample::TSample2D(uniform(random_), uniform(random_));
}


Sample::TSample1D AdjointPhotonTracer::scatterSample(const Sample& sample, size_t generation) const
{
	if (generation < maxRayGeneration())
	{
		return *sample.subSequence1D(scatterSample_[generation]);
	}
	std::uniform_real_distribution<Sample::TSample1D> uniform;
	return uniform(random_);
}


Sample::TSample2D AdjointPhotonTracer::scatterPhaseSample(const Sample& sample, size_t generation) const
{
	if (generation < maxRayGeneration())
	{
		return *sample.subSequence2D(scatterPhaseSample_[generation]);
	}
	std::uniform_real_distribution<Sample::TSample2D::TValue> uniform;
	return Sample::TSample2D(uniform(random_), uniform(random_));
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

