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
/*
#include "../kernel/per_thread_buffer.h"
#include <lass/num/distribution.h>
#include <lass/util/progress_indicator.h>
#include <lass/stde/range_algorithm.h>
#include <lass/stde/extended_iterator.h>
#include <lass/util/thread.h>
*/
#include <lass/num/floating_point_comparison.h>

#define EVAL(x) LASS_COUT << LASS_STRINGIFY(x) << ": " << (x) << std::endl

namespace liar
{
namespace tracers
{

PY_DECLARE_CLASS_DOC(DirectLighting, "simple ray tracer")
PY_CLASS_CONSTRUCTOR_0(DirectLighting)


// --- public --------------------------------------------------------------------------------------

DirectLighting::DirectLighting()
{
}


// --- protected -----------------------------------------------------------------------------------

// --- private -------------------------------------------------------------------------------------

void DirectLighting::doRequestSamples(const kernel::TSamplerPtr&)
{
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



const XYZ DirectLighting::doCastRay(
		const kernel::Sample& sample, const kernel::DifferentialRay& primaryRay,
		TScalar& tIntersection, TScalar& alpha, int generation) const
{
	Intersection intersection;
	scene()->intersect(sample, primaryRay, intersection);
	if (!intersection)
	{
		tIntersection = TNumTraits::infinity;
		alpha = 0;
		return XYZ();
	}
	tIntersection = intersection.t();
	alpha = 1;

	const BoundedRay mediumRay = bound(primaryRay.centralRay(), primaryRay.centralRay().nearLimit(), tIntersection);
	LASS_ENFORCE(!mediumRay.isEmpty());
	XYZ transparency;
	XYZ result = doShadeMedium(sample, mediumRay, transparency);
	if (!transparency)
	{
		return result;
	}

	XYZ surfaceResult;
	IntersectionContext context(*scene(), sample, primaryRay, intersection);
	if (context.shader())
	{
		const TPoint3D point = primaryRay.point(intersection.t());
		const TVector3D normal = context.bsdfToWorld(TVector3D(0, 0, 1));
		const TVector3D omega = context.worldToBsdf(-primaryRay.direction());
		LASS_ASSERT(omega.z >= 0);
		result += transparency * doShadeSurface(sample, primaryRay, context, point, normal, omega, generation);
	}
	else
	{
		// leaving or entering something
		MediumChanger mediumChanger(mediumStack(), context.interior(), context.solidEvent());
		const DifferentialRay continuedRay = bound(primaryRay, intersection.t() + liar::tolerance);
		result += transparency * this->castRay(sample, continuedRay, tIntersection, alpha);
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



const XYZ DirectLighting::doShadeMedium(const kernel::Sample& sample, const kernel::BoundedRay& ray, XYZ& transparency) const
{
	transparency = mediumStack().transmittance(ray);
	return traceSingleScattering(sample, ray);
}



const XYZ DirectLighting::doShadeSurface(
		const kernel::Sample& sample, const DifferentialRay& primaryRay, const IntersectionContext& context,
		const TPoint3D& point, const TVector3D& normal, const TVector3D& omega, int generation) const
{

	const Shader* const shader = context.shader();
	const TBsdfPtr bsdf = context.bsdf();

	XYZ result = context.shader()->emission(sample, context, omega);
	result += traceDirect(sample, context, bsdf, point, normal, omega);
	const bool singleSample = generation > 0;
	result += traceSpecularAndGlossy(sample, primaryRay, context, bsdf, point, normal, omega, singleSample);

	return result;
}



const XYZ DirectLighting::traceDirect(
		const Sample& sample, const IntersectionContext&, const TBsdfPtr& bsdf,
		const TPoint3D& point, const TVector3D& normal, const TVector3D& omega) const
{
	XYZ result;
	const LightContexts::TIterator end = lights().end();
	for (LightContexts::TIterator light = lights().begin(); light != end; ++light)
	{
		Sample::TSubSequence2D lightSamples = sample.subSequence2D(light->idLightSamples());
		Sample::TSubSequence2D bsdfSamples = sample.subSequence2D(light->idBsdfSamples());
		Sample::TSubSequence1D compSamples = sample.subSequence1D(light->idBsdfComponentSamples());
		result += estimateLightContribution(sample, bsdf, *light, lightSamples, bsdfSamples, compSamples, point, normal, omega);
	}
	return result;
}



const XYZ DirectLighting::traceSpecularAndGlossy(
		const Sample& sample, const kernel::DifferentialRay& primaryRay, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& point, const TVector3D& normal, const TVector3D& omega, bool singleSample) const
{
	const Shader* const shader = context.shader();
	LASS_ASSERT(shader);

	if (!(shader->hasCaps(Bsdf::capsSpecular) || shader->hasCaps(Bsdf::capsGlossy)))
	{
		return XYZ();
	}

	const TVector3D dNormal_dI = prim::normalTransform(context.dNormal_dI(), context.localToWorld());
	const TVector3D dNormal_dJ = prim::normalTransform(context.dNormal_dJ(), context.localToWorld());
	const TVector3D dPoint_dI = prim::transform(context.dPoint_dI(), context.localToWorld());
	const TVector3D dPoint_dJ = prim::transform(context.dPoint_dJ(), context.localToWorld());

	XYZ result;
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
		const size_t n = singleSample ? 1 : bsdfSample.size();
		for (size_t i = 0; i < n; ++i)
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
			TScalar t, a;
			const XYZ reflected = castRay(sample, reflectedRay, t, a);
			result += out.value * reflected * (a * num::abs(out.omegaOut.z) / (n * out.pdf));
		}
	}

	if (shader->hasCaps(Bsdf::capsTransmission) && shader->idTransmissionSamples() != -1)
	{
		const MediumChanger mediumChanger(mediumStack(), context.interior(), context.solidEvent());
		const TPoint3D beginCentral = point - 10 * liar::tolerance * normal;

		const Sample::TSubSequence2D bsdfSample = sample.subSequence2D(shader->idTransmissionSamples());
		const Sample::TSubSequence1D compSample = sample.subSequence1D(shader->idTransmissionComponentSamples());
		const size_t n = singleSample ? 1 : bsdfSample.size();
		for (size_t i = 0; i < n; ++i)
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
			TScalar t, a;
			const XYZ transmitted = castRay(sample, transmittedRay, t, a);
			result += out.value * transmitted * (a * num::abs(out.omegaOut.z) / (n * out.pdf));
		}
	}

	return result;
}


const XYZ DirectLighting::traceSingleScattering(const Sample& sample, const kernel::BoundedRay& ray) const
{
	typedef Sample::TSubSequence1D::difference_type difference_type;

	const Medium* medium = mediumStack().medium();
	if (!medium)
	{
		return XYZ();
	}

	const Sample::TSubSequence1D stepSamples = sample.subSequence1D(medium->idStepSamples()); // these are unsorted!!!
	const Sample::TSubSequence1D lightSamples = sample.subSequence1D(medium->idLightSamples());
	const Sample::TSubSequence2D surfaceSamples = sample.subSequence2D(medium->idSurfaceSamples());

	XYZ result;
	const difference_type n = stepSamples.size();
	LASS_ASSERT(lightSamples.size() == n && surfaceSamples.size() == n);
	for (difference_type k = 0; k < n; ++k)
	{
		TScalar tScatter, tPdf;
		const XYZ transRay = medium->sampleScatterOut(stepSamples[k], ray, tScatter, tPdf);
		const TPoint3D point = ray.point(tScatter);
		TScalar lightPdf;
		const LightContext* light = lights().sample(lightSamples[k], lightPdf);
		if (!light || lightPdf <= 0)
		{
			continue;
		}
		BoundedRay shadowRay;
		TScalar surfacePdf;
		const XYZ radiance = light->sampleEmission(sample, surfaceSamples[k], point, shadowRay, surfacePdf);
		if (surfacePdf <= 0 || !radiance)
		{
			continue;
		}
		const XYZ phase = medium->phase(point, ray.direction(), shadowRay.direction());
		if (scene()->isIntersecting(sample, shadowRay))
		{
			continue;
		}
		const XYZ transShadow = medium->transmittance(shadowRay);
		result += transRay * transShadow * phase * radiance / (n * tPdf * lightPdf * surfacePdf);
	}

	return result;
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
