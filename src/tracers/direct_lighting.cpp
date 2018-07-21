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
#include "direct_lighting.h"
#include <lass/num/floating_point_comparison.h>

#define EVAL(x) LASS_COUT << LASS_STRINGIFY(x) << ": " << (x) << std::endl

namespace liar
{
namespace tracers
{

PY_DECLARE_CLASS_DOC(DirectLighting, "simple ray tracer")
PY_CLASS_CONSTRUCTOR_0(DirectLighting)
PY_CLASS_MEMBER_RW(DirectLighting, numSecondaryLightSamples, setNumSecondaryLightSamples)


// --- public --------------------------------------------------------------------------------------

DirectLighting::DirectLighting()
{
	setNumSecondaryLightSamples(1);
}



size_t DirectLighting::numSecondaryLightSamples() const
{
	return numSecondaryLightSamples_;
}



void DirectLighting::setNumSecondaryLightSamples(size_t numSamples)
{
	numSecondaryLightSamples_ = std::max<size_t>(numSamples, 1);
	secondaryLightSelectorSamples_.resize(numSecondaryLightSamples_);
	secondaryLightSamples_.resize(numSecondaryLightSamples_);
	secondaryBsdfSamples_.resize(numSecondaryLightSamples_);
	secondaryBsdfComponentSamples_.resize(numSecondaryLightSamples_);
}



// --- protected -----------------------------------------------------------------------------------

// --- private -------------------------------------------------------------------------------------

void DirectLighting::doRequestSamples(const kernel::TSamplerPtr& sampler)
{
	requestLightAndSceneSamples(sampler);
}



void DirectLighting::doPreProcess(const kernel::TSamplerPtr&, const TimePeriod&, size_t)
{
}



namespace temp
{
	typedef util::AllocatorSingleton<
		util::AllocatorLocked<
			util::AllocatorBinned<
				util::AllocatorFreeList<>,
				256
			>,
			util::CriticalSection
		>
	>
	CustomAllocator;

	template <typename T>
	class custom_stl_allocator: public stde::lass_allocator<T, CustomAllocator> {};
}



const Spectral DirectLighting::doCastRay(
		const kernel::Sample& sample, const kernel::DifferentialRay& primaryRay,
		TScalar& tIntersection, TScalar& alpha, size_t generation, bool highQuality) const
{
	Intersection intersection;
	scene()->intersect(sample, primaryRay, intersection);
	tIntersection = intersection ? intersection.t() : TNumTraits::infinity;
	alpha = intersection ? 1 : 0;

	const BoundedRay mediumRay = bound(primaryRay.centralRay(), primaryRay.centralRay().nearLimit(), tIntersection);
	LASS_ENFORCE(!mediumRay.isEmpty());
	Spectral transparency;
	Spectral result = doShadeMedium(sample, mediumRay, transparency);
	if (!transparency || !intersection)
	{
		return result;
	}

	Spectral surfaceResult;
	IntersectionContext context(*scene(), sample, primaryRay, intersection, generation);
	if (context.shader())
	{
		const TPoint3D point = primaryRay.point(intersection.t());
		const TVector3D normal = context.worldNormal();
		const TVector3D omega = context.worldToBsdf(-primaryRay.direction());
		LASS_ASSERT(omega.z >= 0);
		result += transparency * doShadeSurface(sample, primaryRay, context, point, normal, omega, highQuality);
	}
	else
	{
		// leaving or entering something
		MediumChanger mediumChanger(mediumStack(), context.interior(), context.solidEvent());
		const DifferentialRay continuedRay = bound(primaryRay, intersection.t() + liar::tolerance);
		result += transparency * this->castRay(sample, continuedRay, tIntersection, alpha, highQuality);
	}

	return result;
}



const TRayTracerPtr DirectLighting::doClone() const
{
	return TRayTracerPtr(new DirectLighting(*this));
}



const TPyObjectPtr DirectLighting::doGetState() const
{
	return python::makeTuple();
}



void DirectLighting::doSetState(const TPyObjectPtr&)
{
}



const Spectral DirectLighting::doShadeMedium(const kernel::Sample& sample, const kernel::BoundedRay& ray, Spectral& transparency) const
{
	transparency = mediumStack().transmittance(sample, ray);
	return mediumStack().emission(sample, ray) + traceSingleScattering(sample, ray);
}



const Spectral DirectLighting::doShadeSurface(
		const kernel::Sample& sample, const DifferentialRay& primaryRay, const IntersectionContext& context,
		const TPoint3D& point, const TVector3D& normal, const TVector3D& omega, bool highQuality) const
{
	const TBsdfPtr bsdf = context.bsdf();

	Spectral result;
	if (context.rayGeneration() == 0 || !context.object().asLight())
	{
		// actually, we block this because currently this is our way to include area lights in the camera rays only.
		// we should always do the shader emission thing, but get the light emission seperately.
		// and generation == 0 isn't a good discriminator eighter.
		result += context.shader()->emission(sample, context, omega);
	}
	result += traceDirect(sample, context, bsdf, point, normal, omega, highQuality);
	result += traceSpecularAndGlossy(sample, primaryRay, context, bsdf, point, normal, omega, highQuality);

	return result;
}



const Spectral DirectLighting::traceDirect(
		const Sample& sample, const IntersectionContext&, const TBsdfPtr& bsdf,
		const TPoint3D& point, const TVector3D& normal, const TVector3D& omega, bool highQuality) const
{
	if (!bsdf)
	{
		return Spectral(0);
	}

	Spectral result;
	if (highQuality)
	{
		const LightContexts::TIterator end = lights().end();
		for (LightContexts::TIterator light = lights().begin(); light != end; ++light)
		{
			Sample::TSubSequence2D lightSamples = sample.subSequence2D(light->idLightSamples());
			Sample::TSubSequence2D bsdfSamples = sample.subSequence2D(light->idBsdfSamples());
			Sample::TSubSequence1D compSamples = sample.subSequence1D(light->idBsdfComponentSamples());
			result += estimateLightContribution(sample, bsdf, *light, lightSamples, bsdfSamples, compSamples, point, normal, omega);
		}
	}
	else
	{
		const size_t n = numSecondaryLightSamples_;
		LASS_ASSERT(n > 0);
		TScalar* lightSelectors = &secondaryLightSelectorSamples_[0];
		TPoint2D* lightSamples = &secondaryLightSamples_[0];
		TPoint2D* bsdfSamples = &secondaryBsdfSamples_[0];
		TScalar* componentSamples = &secondaryBsdfComponentSamples_[0];
		stratifier1D(lightSelectors, lightSelectors + n, secondarySampler_);
		latinHypercube2D(lightSamples, lightSamples + n, secondarySampler_);
		latinHypercube2D(bsdfSamples, bsdfSamples + n, secondarySampler_);
		stratifier1D(componentSamples, componentSamples + n, secondarySampler_);
		for (size_t k = 0; k < n; ++k)
		{
			TScalar pdf;
			const LightContext* light = lights().sample(lightSelectors[k], pdf);
			if (!light || pdf <= 0)
			{
				continue;
			}
			Spectral radiance = RayTracer::estimateLightContribution(
				sample, bsdf, *light,
				Sample::TSubSequence2D(lightSamples + k, lightSamples + k + 1),
				Sample::TSubSequence2D(bsdfSamples + k, bsdfSamples + k + 1),
				Sample::TSubSequence1D(componentSamples + k, componentSamples + k + 1),
				point, normal, omega);
			radiance /= static_cast<Spectral::TValue>(n) * static_cast<Spectral::TValue>(pdf);
			result += radiance;
		}
	}

	return result;
}



const Spectral DirectLighting::traceSpecularAndGlossy(
		const Sample& sample, const kernel::DifferentialRay& primaryRay, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& point, const TVector3D& normal, const TVector3D& omega, bool highQuality) const
{
	typedef Sample::TSubSequence2D::difference_type difference_type;

	const Shader* const shader = context.shader();
	LASS_ASSERT(shader);

	if (!(shader->hasCaps(Bsdf::capsSpecular) || shader->hasCaps(Bsdf::capsGlossy)))
	{
		return Spectral(0);
	}

	const TVector3D dNormal_dI = prim::normalTransform(context.dNormal_dI(), context.localToWorld());
	const TVector3D dNormal_dJ = prim::normalTransform(context.dNormal_dJ(), context.localToWorld());
	const TVector3D dPoint_dI = prim::transform(context.dPoint_dI(), context.localToWorld());
	const TVector3D dPoint_dJ = prim::transform(context.dPoint_dJ(), context.localToWorld());

	Spectral result;
	if (bsdf->hasCaps(Bsdf::capsReflection) && shader->idReflectionSamples() != -1)
	{
		const TPoint3D beginCentral = point + 10 * liar::tolerance * normal;
		const TPoint3D beginI = beginCentral + dPoint_dI;
		const TPoint3D beginJ = beginCentral + dPoint_dJ;

		const TVector3D incident = primaryRay.centralRay().direction();
		const TScalar cosTheta = -dot(incident, normal);

		const TVector3D dIncident_dI = primaryRay.differentialI().direction() - incident;
		const TScalar dCosTheta_dI = -dot(dIncident_dI, normal) - dot(incident, dNormal_dI);
		const TVector3D dReflected_dI = dIncident_dI + 2 * (dCosTheta_dI * normal + cosTheta * dNormal_dI);

		const TVector3D dIncident_dJ = primaryRay.differentialJ().direction() - incident;
		const TScalar dCosTheta_dJ = -dot(dIncident_dJ, normal) - dot(incident, dNormal_dJ);
		const TVector3D dReflected_dJ = dIncident_dJ + 2 * (dCosTheta_dJ * normal + cosTheta * dNormal_dJ);

		const Sample::TSubSequence2D bsdfSample = sample.subSequence2D(shader->idReflectionSamples());
		const Sample::TSubSequence1D compSample = sample.subSequence1D(shader->idReflectionComponentSamples());
		LASS_ASSERT(bsdfSample.size() > 0 && compSample.size() == bsdfSample.size());

		const difference_type n = highQuality ? bsdfSample.size() : 1;
		for (difference_type i = 0; i < n; ++i)
		{
			const SampleBsdfOut out = bsdf->sample(omega, bsdfSample[i], compSample[i], Bsdf::capsReflection | Bsdf::capsSpecular | Bsdf::capsGlossy);
			if (!out)
			{
				continue;
			}
			LASS_ASSERT(out.omegaOut.z > 0);
			const TVector3D directionCentral = context.bsdfToWorld(out.omegaOut);
			LASS_ASSERT(dot(normal, directionCentral) > 0);
			LASS_ASSERT(dot(normal, incident) < 0);
			const TVector3D directionI = directionCentral + dReflected_dI;
			const TVector3D directionJ = directionCentral + dReflected_dJ;

			const DifferentialRay reflectedRay(
				BoundedRay(beginCentral, directionCentral, liar::tolerance),
				TRay3D(beginI, directionI),
				TRay3D(beginJ, directionJ));
			TScalar t;
			TScalar a;
			const Spectral reflected = castRay(sample, reflectedRay, t, a, highQuality && (out.usedCaps & Bsdf::capsSpecular));
			result += out.value * reflected * static_cast<Spectral::TValue>(a * num::abs(out.omegaOut.z) / (static_cast<TScalar>(n) * out.pdf));
		}
	}
	//*
	if (shader->hasCaps(Bsdf::capsTransmission) && shader->idTransmissionSamples() != -1)
	{
		const MediumChanger mediumChanger(mediumStack(), context.interior(), context.solidEvent());
		const TPoint3D beginCentral = point - 10 * liar::tolerance * normal;

		const Sample::TSubSequence2D bsdfSample = sample.subSequence2D(shader->idTransmissionSamples());
		const Sample::TSubSequence1D compSample = sample.subSequence1D(shader->idTransmissionComponentSamples());
		LASS_ASSERT(bsdfSample.size() > 0 && compSample.size() == bsdfSample.size());

		const difference_type n = highQuality ? bsdfSample.size() : 1;
		for (difference_type i = 0; i < n; ++i)
		{
			const SampleBsdfOut out = bsdf->sample(omega, bsdfSample[i], compSample[i], Bsdf::capsTransmission | Bsdf::capsSpecular | Bsdf::capsGlossy);
			if (!out)
			{
				continue;
			}
			LASS_ASSERT(out.omegaOut.z < 0);
			const TVector3D directionCentral = context.bsdfToWorld(out.omegaOut);

#pragma LASS_TODO("preserve ray differentials! [Bramz]")
			const DifferentialRay transmittedRay(
				BoundedRay(beginCentral, directionCentral, liar::tolerance),
				TRay3D(beginCentral, directionCentral),
				TRay3D(beginCentral, directionCentral));
			TScalar t;
			TScalar a;
			const Spectral transmitted = castRay(sample, transmittedRay, t, a, highQuality && (out.usedCaps & Bsdf::capsSpecular));
			result += out.value * transmitted * static_cast<Spectral::TValue>(a * num::abs(out.omegaOut.z) / (static_cast<TScalar>(n) * out.pdf));
		}
	}
	/**/
	return result;
}


const Spectral DirectLighting::traceSingleScattering(const Sample& sample, const kernel::BoundedRay& ray) const
{
	typedef Sample::TSubSequence1D::difference_type difference_type;

	const Medium* medium = mediumStack().medium();
	if (!medium)
	{
		return Spectral();
	}

	const Sample::TSubSequence1D stepSamples = sample.subSequence1D(medium->idStepSamples()); // these are unsorted!!!
	const Sample::TSubSequence1D lightSamples = sample.subSequence1D(medium->idLightSamples());
	const Sample::TSubSequence2D surfaceSamples = sample.subSequence2D(medium->idSurfaceSamples());

	Spectral result;
	const difference_type n = stepSamples.size();
	LASS_ASSERT(lightSamples.size() == n && surfaceSamples.size() == n);
	for (difference_type k = 0; k < n; ++k)
	{
		TScalar tScatter, tPdf;
		const Spectral transRay = medium->sampleScatterOut(stepSamples[k], ray, tScatter, tPdf);
		if (tPdf <= 0)
		{
			continue;
		}
		const TPoint3D point = ray.point(tScatter);
		TScalar lightPdf;
		const LightContext* light = lights().sample(lightSamples[k], lightPdf);
		if (!light || lightPdf <= 0)
		{
			continue;
		}
		BoundedRay shadowRay;
		TScalar surfacePdf;
		const Spectral radiance = light->sampleEmission(sample, surfaceSamples[k], point, shadowRay, surfacePdf);
		if (surfacePdf <= 0 || !radiance)
		{
			continue;
		}
		const Spectral phase = medium->phase(sample, point, ray.direction(), shadowRay.direction());
		if (scene()->isIntersecting(sample, shadowRay))
		{
			continue;
		}
		const Spectral transShadow = medium->transmittance(sample, shadowRay);
		result += transRay * transShadow * phase * radiance / static_cast<Spectral::TValue>(static_cast<TScalar>(n) * tPdf * lightPdf * surfacePdf);
	}

	return result;
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
