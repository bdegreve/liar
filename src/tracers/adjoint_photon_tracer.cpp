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

void AdjointPhotonTracer::doRequestSamples(const kernel::TSamplerPtr&)
{
}



void AdjointPhotonTracer::doPreProcess(const kernel::TSamplerPtr&, const TimePeriod&, size_t)
{
}



const XYZ AdjointPhotonTracer::doCastRay(
		const kernel::Sample& sample, const kernel::DifferentialRay& primaryRay,
		TScalar& tIntersection, TScalar& alpha, int generation) const
{
/*	TScalar weight = 1;

	Intersection intersection;
	scene()->intersect(sample, primaryRay, intersection);
	if (!intersection)
	{
		tIntersection = TNumTraits::infinity;
		alpha = 0;
		return 0;
	}
	
	const TPoint3D hitPoint = ray.point(tIntersection);
	TScalar tScatter, pdf;
	const XYZ transmittance = mediumStack().sampleScatterOutOrTransmittance(uniform(), bound(ray, ray.nearLimit(), tIntersection), tScatter, pdf);
	XYZ transmittedPower = power * transmittance / pdf;
	const TScalar transmittanceProbability = std::min(transmittedPower.absTotal() / power.absTotal(), TNumTraits::one);
	if (!russianRoulette(transmittedPower, transmittanceProbability, uniform()))
	{
		return;
	}

	if (tScatter < tIntersection)
	{
		// scattering event.
		if (generation >= maxRayGeneration())
		{
			return;
		}

		const TPoint3D scatterPoint = ray.point(tScatter);
		TVector3D dirOut;
		TScalar pdfOut;
		const XYZ reflectance = mediumStack().samplePhase(TPoint2D(uniform(), uniform()), scatterPoint, ray.direction(), dirOut, pdfOut);
		if (pdfOut <= 0 || !reflectance)
		{
			return;
		}
		XYZ scatteredPower = transmittedPower * reflectance / pdfOut;
		const TScalar scatteredProbability = std::min(scatteredPower.absTotal() / transmittedPower.absTotal(), TNumTraits::one);
		if (!russianRoulette(scatteredPower, scatteredProbability, uniform()))
		{
			return;
		}
		const BoundedRay scatteredRay(scatterPoint, ray.direction());
		return tracePhoton(sample, scatteredPower, scatteredRay, generation + 1, uniform, false);
	}

	const IntersectionContext context(*scene(), sample, primaryRay, intersection);
	const Shader* const shader = context.shader();
	if (!shader)
	{
		// leaving or entering something
		MediumChanger mediumChanger(mediumStack(), context.interior(), context.solidEvent());
		const DifferentialRay continuedRay = bound(primaryRay, intersection.t() + tolerance);
		return weight * this->castRay(sample, continuedRay, tIntersection, alpha);
	}

	const TBsdfPtr bsdf = shader->bsdf(sample, context);
	const TVector3D omegaIn = context.worldToBsdf(-primaryRay.direction());
	SampleBsdfOut out = bsdf->sample(omegaIn, sample, Bsdf::capsAll);
	if (!out)
	{
		return XYZ();
	}
	
	we*/
	return XYZ();
}



const TRayTracerPtr AdjointPhotonTracer::doClone() const
{
	return TRayTracerPtr(new AdjointPhotonTracer(*this));
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
