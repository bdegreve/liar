/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
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

void DirectLighting::doRequestSamples(const kernel::TSamplerPtr& sampler)
{
}



void DirectLighting::doPreProcess(const kernel::TSamplerPtr& sampler, const TimePeriod& period)
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
		return 0;
	}
	tIntersection = intersection.t();
	alpha = 1;
	const TPoint3D target = primaryRay.point(intersection.t());
	const XYZ mediumTransparency = mediumStack_.transparency(BoundedRay(
		primaryRay.centralRay().unboundedRay(), primaryRay.centralRay().nearLimit(),
		intersection.t()));

	IntersectionContext context(this);
	scene()->localContext(sample, primaryRay, intersection, context);
	context.flipTo(-primaryRay.direction());

	const Shader* const shader = context.shader();
	if (!shader)
	{
		// leaving or entering something
		MediumChanger mediumChanger(mediumStack_, context.interior(), context.solidEvent());
		const DifferentialRay continuedRay = bound(primaryRay, intersection.t() + tolerance);
		return mediumTransparency * this->castRay(sample, continuedRay, tIntersection, alpha);
	}
	shader->shadeContext(sample, context);

	const TVector3D targetNormal = context.bsdfToWorld(TVector3D(0, 0, 1));
	const TVector3D omegaIn = context.worldToBsdf(-primaryRay.direction());
	//LASS_ASSERT(omegaIn.z >= 0);

	XYZ result = shader->emission(sample, context, omegaIn);

	result += traceDirect(sample, context, target, targetNormal, omegaIn);

	if (shader->hasCaps(Shader::capsSpecular) || shader->hasCaps(Shader::capsGlossy))
	{
		//*
		if (shader->hasCaps(Shader::capsReflection) && shader->idReflectionSamples() != -1)
		{
			Sample::TSubSequence2D bsdfSample =
				sample.subSequence2D(shader->idReflectionSamples());
			const size_t n = generation == 0 ? bsdfSample.size() : 1;
			std::vector<SampleBsdfIn, temp::custom_stl_allocator<SampleBsdfIn> > in(n);
			std::vector<SampleBsdfOut, temp::custom_stl_allocator<SampleBsdfOut> > out(n);
			for (size_t i = 0; i < n; ++i)
			{
				in[i].sample = bsdfSample[i];
				in[i].allowedCaps = Shader::capsReflection | Shader::capsSpecular | Shader::capsGlossy;
			}
			shader->sampleBsdf(sample, context, omegaIn, &in[0], &in[0] + n, &out[0]);

			const TPoint3D beginCentral = target + 2 * tolerance * targetNormal;
			const TPoint3D beginI = beginCentral + context.dPoint_dI();
			const TPoint3D beginJ = beginCentral + context.dPoint_dJ();

			const TVector3D incident = primaryRay.centralRay().direction();
			const TVector3D normal = context.normal();
			const TScalar cosTheta = -dot(incident, normal);

			const TVector3D dIncident_dI = primaryRay.differentialI().direction() - incident;
			const TScalar dCosTheta_dI = -dot(dIncident_dI, normal) - 
				dot(incident, context.dNormal_dI());
			const TVector3D dReflected_dI = dIncident_dI + 
				2 * (dCosTheta_dI * normal + cosTheta * context.dNormal_dI());

			const TVector3D dIncident_dJ = primaryRay.differentialJ().direction() - incident;
			const TScalar dCosTheta_dJ = -dot(dIncident_dJ, normal) - 
				dot(incident, context.dNormal_dJ());
			const TVector3D dReflected_dJ = dIncident_dJ +
				2 * (dCosTheta_dJ * normal + cosTheta * context.dNormal_dJ());

			for (size_t i = 0; i < n; ++i)
			{
				if (out[i].pdf > 0 && out[i].value)
				{
					LASS_ASSERT(out[i].omegaOut.z > 0);
					const TVector3D directionCentral = context.bsdfToWorld(out[i].omegaOut);
					LASS_ASSERT(dot(normal, directionCentral) > 0);
					LASS_ASSERT(dot(normal, incident) < 0);
					const TVector3D directionI = directionCentral + dReflected_dI;
					const TVector3D directionJ = directionCentral + dReflected_dJ;

					const DifferentialRay reflectedRay(
						BoundedRay(beginCentral, directionCentral, tolerance),
						TRay3D(beginI, directionI),
						TRay3D(beginJ, directionJ));
					TScalar t, a;
					const XYZ reflected = castRay(sample, reflectedRay, t, a);
					result += out[i].value * reflected * 
						(a * num::abs(out[i].omegaOut.z) / (n * out[i].pdf));
				}
			}
		}
	}

	return mediumTransparency * result;
}



const TLightSamplesRange
DirectLighting::doSampleLights(const Sample& iSample, const TPoint3D& iTarget, 
		const TVector3D& iTargetNormal) const
{
	return TLightSamplesRange();
}



const TRayTracerPtr DirectLighting::doClone() const
{
	return TRayTracerPtr(new DirectLighting(*this));
}



const TPyObjectPtr DirectLighting::doGetState() const
{
	return python::makeTuple();
}



void DirectLighting::doSetState(const TPyObjectPtr& state)
{
}

namespace temp
{
	inline TScalar squaredHeuristic(TScalar pdfA, TScalar pdfB)
	{
		return num::sqr(pdfA) / (num::sqr(pdfA) + num::sqr(pdfB));
	}
}

const XYZ DirectLighting::traceDirect(
		const Sample& sample, const IntersectionContext& context,
		const TPoint3D& target, const TVector3D& targetNormal, 
		const TVector3D& omegaIn) const
{
	const Shader* const shader = context.shader();
	LASS_ASSERT(shader);
	LASS_ASSERT(lass::num::almostEqual<TScalar>(targetNormal.norm(), 1, 1e-5));
	//LASS_ASSERT(lass::num::almostEqual<TScalar>(omegaIn.norm(), 1, 1e-5));

	XYZ result;
	const TLightContexts::const_iterator end = lights().end();
	for (TLightContexts::const_iterator light = lights().begin(); light != end; ++light)
	{
		Sample::TSubSequence2D lightSample = sample.subSequence2D(light->idLightSamples());
		/*
		Sample::TSubSequence2D bsdfSample = sample.subSequence2D(light->idBsdfSamples());
		Sample::TSubSequence1D componentSample = sample.subSequence1D(light->idBsdfComponentSamples());
		LASS_ASSERT(bsdfSample.size() == lightSample.size() && componentSample.size() == lightSample.size());
		*/
		const TScalar n = static_cast<TScalar>(lightSample.size());
		const bool isSingularLight = light->isSingular();
		unsigned caps = Shader::capsAll & ~Shader::capsSpecular;
		if (!isSingularLight)
		{
			caps &= ~Shader::capsGlossy;
		}

		while (lightSample)
		{
			BoundedRay shadowRay;
			TScalar lightPdf;
			const XYZ radiance = light->sampleEmission(
				sample, *lightSample, target + 2 * tolerance * targetNormal,
				targetNormal, shadowRay, lightPdf);
			if (lightPdf > 0 && radiance)
			{
				const TVector3D& omegaOut = context.worldToBsdf(shadowRay.direction());
				BsdfIn in(omegaOut, caps);
				BsdfOut out;
				shader->bsdf(sample, context, omegaIn, &in, &in + 1, &out);
				if (out.value && !scene()->isIntersecting(sample, shadowRay))
				{
					//const TScalar weight = isSingularLight ? 
					//	TNumTraits::one : temp::squaredHeuristic(lightPdf, bsdfPdf);
					const TScalar cosTheta = omegaOut.z;
					const TScalar weight = 1;
					result += out.value * radiance * 
						(weight * num::abs(cosTheta) / (n * lightPdf));
				}
			}
			/*
			if (!isSingularLight)
			{
				TVector3D omegaIn;
				XYZ bsdf;
				TScalar bsdfPdf;
				shader->sampleBsdf(sample, context, omegaOut, bsdfSample.begin(), bsdfSample.begin() + 1, 
					&omegaIn, &bsdf, &bsdfPdf, Shader::capsAll & ~Shader::capsSpecular);
				if (bsdfPdf > 0 && bsdf)
				{
					TRay3D ray(target, context.shaderToWorld(omegaIn)).normal();
					BoundedRay shadowRay;
					TScalar lightPdf;
					const XYZ radiance = light->emission(sample, ray, shadowRay, lightPdf);
					if (lightPdf > 0 && !scene()->isIntersecting(sample, shadowRay))
					{
						const TScalar weight = temp::squaredHeuristic(bsdfPdf, lightPdf);
						result += bsdf * radiance * (weight * abs(omegaIn.z) / (n * bsdfPdf));
					}
				}
			}
			*/
			++lightSample;
			/*
			++bsdfSample;
			++componentSample;
			*/
		}
	}

	return result;
}



const XYZ DirectLighting::traceSpecularAndGlossy(
		const Sample& sample, const IntersectionContext& context, const kernel::DifferentialRay& primaryRay,
		const TPoint3D& target, const TVector3D& targetNormal, const TVector3D& omegaIn, bool singleSample) const
{
	const Shader* const shader = context.shader();
	LASS_ASSERT(shader);

	if (!(shader->hasCaps(Shader::capsSpecular) || shader->hasCaps(Shader::capsGlossy)))
	{
		return XYZ();
	}

	XYZ result;
	if (shader->hasCaps(Shader::capsReflection) && shader->idReflectionSamples() != -1)
	{
		Sample::TSubSequence2D bsdfSample = sample.subSequence2D(shader->idReflectionSamples());
		const size_t n = singleSample ? 1 : bsdfSample.size();
		std::vector<SampleBsdfIn, temp::custom_stl_allocator<SampleBsdfIn> > in(n);
		std::vector<SampleBsdfOut, temp::custom_stl_allocator<SampleBsdfOut> > out(n);
		for (size_t i = 0; i < n; ++i)
		{
			in[i].sample = bsdfSample[i];
			in[i].allowedCaps = Shader::capsReflection | Shader::capsSpecular | Shader::capsGlossy;
		}
		shader->sampleBsdf(sample, context, omegaIn, &in[0], &in[0] + n, &out[0]);

		const TPoint3D beginCentral = target + 2 * tolerance * targetNormal;
		const TPoint3D beginI = beginCentral + context.dPoint_dI();
		const TPoint3D beginJ = beginCentral + context.dPoint_dJ();

		const TVector3D incident = primaryRay.centralRay().direction();
		const TVector3D normal = context.normal();
		const TScalar cosTheta = -dot(incident, normal);

		const TVector3D dIncident_dI = primaryRay.differentialI().direction() - incident;
		const TScalar dCosTheta_dI = -dot(dIncident_dI, normal) - dot(incident, context.dNormal_dI());
		const TVector3D dReflected_dI = dIncident_dI + 2 * (dCosTheta_dI * normal + cosTheta * context.dNormal_dI());

		const TVector3D dIncident_dJ = primaryRay.differentialJ().direction() - incident;
		const TScalar dCosTheta_dJ = -dot(dIncident_dJ, normal) - dot(incident, context.dNormal_dJ());
		const TVector3D dReflected_dJ = dIncident_dJ + 2 * (dCosTheta_dJ * normal + cosTheta * context.dNormal_dJ());

		for (size_t i = 0; i < n; ++i)
		{
			if (out[i].pdf > 0 && out[i].value)
			{
				LASS_ASSERT(out[i].omegaOut.z > 0);
				const TVector3D directionCentral = context.bsdfToWorld(out[i].omegaOut);
				LASS_ASSERT(dot(normal, directionCentral) > 0);
				LASS_ASSERT(dot(normal, incident) < 0);
				const TVector3D directionI = directionCentral + dReflected_dI;
				const TVector3D directionJ = directionCentral + dReflected_dJ;

				const DifferentialRay reflectedRay(
					BoundedRay(beginCentral, directionCentral, tolerance),
					TRay3D(beginI, directionI),
					TRay3D(beginJ, directionJ));
				TScalar t, a;
				const XYZ reflected = castRay(sample, reflectedRay, t, a);
				result += out[i].value * reflected * (a * num::abs(out[i].omegaOut.z) / (n * out[i].pdf));
			}
		}
	}

	if (shader->hasCaps(Shader::capsTransmission) && shader->idTransmissionSamples() != -1)
	{
		MediumChanger mediumChanger(mediumStack_, context.interior(), context.solidEvent());

		Sample::TSubSequence2D bsdfSample = sample.subSequence2D(shader->idTransmissionSamples());
		const size_t n = singleSample ? 1 : bsdfSample.size();
		std::vector<SampleBsdfIn, temp::custom_stl_allocator<SampleBsdfIn> > in(n);
		std::vector<SampleBsdfOut, temp::custom_stl_allocator<SampleBsdfOut> > out(n);
		for (size_t i = 0; i < n; ++i)
		{
			in[i].sample = bsdfSample[i];
			in[i].allowedCaps = Shader::capsTransmission | Shader::capsSpecular | Shader::capsGlossy;
		}
		shader->sampleBsdf(sample, context, omegaIn, &in[0], &in[0] + n, &out[0]);

		const TPoint3D beginCentral = target - 2 * tolerance * targetNormal;

		for (size_t i = 0; i < n; ++i)
		{
			if (out[i].pdf > 0 && out[i].value)
			{
				LASS_ASSERT(out[i].omegaOut.z < 0);
				const TVector3D directionCentral = context.bsdfToWorld(out[i].omegaOut);

#pragma LASS_TODO("preserve ray differentials! [Bramz]")
				const DifferentialRay transmittedRay(
					BoundedRay(beginCentral, directionCentral, tolerance),
					TRay3D(beginCentral, directionCentral),
					TRay3D(beginCentral, directionCentral));
				TScalar t, a;
				const XYZ transmitted = castRay(sample, transmittedRay, t, a);
				result += out[i].value * transmitted * 
					(a * num::abs(out[i].omegaOut.z) / (n * out[i].pdf));
			}
		}
	}

	return result;
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
