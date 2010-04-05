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
#include "photon_mapper.h"
#include "../kernel/per_thread_buffer.h"
#include <lass/num/distribution.h>
#include <lass/num/distribution_transformations.h>
#include <lass/util/progress_indicator.h>
#include <lass/stde/range_algorithm.h>
#include <lass/stde/extended_iterator.h>
#include <lass/stde/extended_string.h>
#include <lass/util/thread.h>
#include <lass/util/callback_0.h>
#include <lass/util/thread_fun.h>
#include <lass/util/thread_pool.h>

#define EVAL(x) LASS_COUT << LASS_STRINGIFY(x) << ": " << (x) << std::endl

namespace liar
{
namespace tracers
{

PY_DECLARE_CLASS_DOC(PhotonMapper, "ray tracer with photon mapper")
PY_CLASS_CONSTRUCTOR_0(PhotonMapper)
PY_CLASS_MEMBER_RW_DOC(PhotonMapper, maxNumberOfPhotons, setMaxNumberOfPhotons,
	"The maximum number of photons being emitted by the light sources.\n"
	"Set this to a very high number (at least globalMapSize * causticsQuality) to prevent the "
	"first pass to end without sufficient photons in the global photon map.\n")
PY_CLASS_MEMBER_RW_DOC(PhotonMapper, globalMapSize, setGlobalMapSize,
	"the requested number of photons in the global photon map.\n")
PY_CLASS_MEMBER_RW_DOC(PhotonMapper, causticsQuality, setCausticsQuality,
	"the quality ratio of the caustics photon map versus global photon map. "
	"Increase if more photons in the caustics map are wanted for higher quality rendering.\n")
PY_CLASS_METHOD(PhotonMapper, estimationRadius)
PY_CLASS_METHOD(PhotonMapper, setEstimationRadius)
PY_CLASS_METHOD(PhotonMapper, estimationTolerance)
PY_CLASS_METHOD(PhotonMapper, setEstimationTolerance)
PY_CLASS_METHOD(PhotonMapper, estimationSize)
PY_CLASS_METHOD(PhotonMapper, setEstimationSize)
PY_CLASS_MEMBER_RW_DOC(PhotonMapper, numFinalGatherRays, setNumFinalGatherRays,
	"- if numFinalGatherRays > 0 and isRayTracingDirect == True, a final gather step with "
	"numFinalGatherRays gather rays is used to estimate the indirect lighting.\n"
	"- else, the indirect lighting is estimated by a direct query of the photon map.\n"
	"\n"
	"Set to zero to disable the final gather step\n")
PY_CLASS_MEMBER_RW_DOC(PhotonMapper, numSecondaryGatherRays, setNumSecondaryGatherRays,
	"- If final gather step is enabled (see numFinalGatherRays), secondary gather rays\n"
	"  can be shot.\n"
	"\n"
	"Set to zero to disable the secondary gather step\n")
PY_CLASS_MEMBER_RW_DOC(PhotonMapper, ratioPrecomputedIrradiance, setRatioPrecomputedIrradiance,
	"- if ratioPrecomputedIrradiance > 0 and numFinalGatherRays > 0 and isRayTracingDirect == True, "
	"the final gather step is optimized by precomputing irradiances. "
	"- else, a full radiance estimation is done for each gather ray.\n"
	"\n"
	"Set to zero to disable precomputed irradiance estimations.\n")
PY_CLASS_MEMBER_RW_DOC(PhotonMapper, isVisualizingPhotonMap, setVisualizePhotonMap,
	"if True, the content of the photon map is directly visualized (without applying material properties)\n"
	"if False, a proper render is done =)\n")
PY_CLASS_MEMBER_RW_DOC(PhotonMapper, isRayTracingDirect, setRayTracingDirect,
	"if True, the direct lighting is estimated by sampling the scene lights\n"
	"if False, the direct lighting is estimated by a direct query of the photon map\n")
	
PhotonMapper::TMapTypeDictionary PhotonMapper::mapTypeDictionary_ = 
	PhotonMapper::generateMapTypeDictionary();


// --- public --------------------------------------------------------------------------------------

PhotonMapper::PhotonMapper():
	maxNumberOfPhotons_(100000000),
	globalMapSize_(10000),
	causticsQuality_(5),
	numFinalGatherRays_(0),
	numSecondaryGatherRays_(0),
	ratioPrecomputedIrradiance_(0.25f),
	isVisualizingPhotonMap_(false),
	isRayTracingDirect_(true),
	photonNeighbourhood_(1)
{
	for (int i = 0; i < numMapTypes; ++i)
	{
		estimationRadius_[i] = 0;
		estimationTolerance_[i] = 0.05f;
		estimationSize_[i] = 50;
		maxActualEstimationRadius_[i] = 0;
	}
}




size_t PhotonMapper::maxNumberOfPhotons() const
{
	return maxNumberOfPhotons_;
}



void PhotonMapper::setMaxNumberOfPhotons(size_t maxNumberOfPhotons)
{
	maxNumberOfPhotons_ = maxNumberOfPhotons;
}



size_t PhotonMapper::globalMapSize() const
{
	return globalMapSize_;
}



void PhotonMapper::setGlobalMapSize(size_t mapSize)
{
	globalMapSize_ = mapSize;
}



TScalar PhotonMapper::causticsQuality() const
{
	return causticsQuality_;
}



void PhotonMapper::setCausticsQuality(TScalar quality)
{
	causticsQuality_ = std::max<TScalar>(quality, 1);
}



TScalar PhotonMapper::estimationRadius(const std::string& mapType) const
{
	return estimationRadius_[mapTypeDictionary_[mapType]];
}



void PhotonMapper::setEstimationRadius(const std::string& mapType, TScalar radius)
{
	estimationRadius_[mapTypeDictionary_[mapType]] = std::max(radius, TNumTraits::zero);
}



TScalar PhotonMapper::estimationTolerance(const std::string& mapType) const
{
	return estimationTolerance_[mapTypeDictionary_[mapType]];
}



void PhotonMapper::setEstimationTolerance(const std::string& mapType, TScalar tolerance)
{
	estimationTolerance_[mapTypeDictionary_[mapType]] = std::max(tolerance, TNumTraits::zero);
}



size_t PhotonMapper::estimationSize(const std::string& mapType) const
{
	return estimationSize_[mapTypeDictionary_[mapType]];
}



void PhotonMapper::setEstimationSize(const std::string& mapType, size_t size)
{
	estimationSize_[mapTypeDictionary_[mapType]] = std::max<size_t>(size, 1);
}



size_t PhotonMapper::numFinalGatherRays() const
{
	return numFinalGatherRays_;
}



void PhotonMapper::setNumFinalGatherRays(size_t numFinalGatherRays)
{
	numFinalGatherRays_ = numFinalGatherRays;
}



size_t PhotonMapper::numSecondaryGatherRays() const
{
	return numSecondaryGatherRays_;
}



void PhotonMapper::setNumSecondaryGatherRays(size_t numSecondaryGatherRays)
{
	numSecondaryGatherRays_ = numSecondaryGatherRays;
	secondaryGatherSamples_.resize(numSecondaryGatherRays);
}



TScalar PhotonMapper::ratioPrecomputedIrradiance() const
{
	return ratioPrecomputedIrradiance_;
}



void PhotonMapper::setRatioPrecomputedIrradiance(TScalar ratio)
{
	ratioPrecomputedIrradiance_ = num::clamp(ratio, TNumTraits::zero, TNumTraits::one);
}



bool PhotonMapper::isVisualizingPhotonMap() const
{
	return isVisualizingPhotonMap_;
}



void PhotonMapper::setVisualizePhotonMap(bool enabled)
{
	isVisualizingPhotonMap_ = enabled;
}



bool PhotonMapper::isRayTracingDirect() const
{
	return isRayTracingDirect_;
}



void PhotonMapper::setRayTracingDirect(bool enabled)
{
	isRayTracingDirect_ = enabled;
}


// --- protected -----------------------------------------------------------------------------------

// --- private -------------------------------------------------------------------------------------

void PhotonMapper::doRequestSamples(const kernel::TSamplerPtr& sampler)
{
	idFinalGatherSamples_ = numFinalGatherRays_ > 0 ?
		sampler->requestSubSequence2D(numFinalGatherRays_) : -1;
}



void PhotonMapper::doPreProcess(const kernel::TSamplerPtr& sampler, const TimePeriod& period)
{
	const size_t maxSize = *std::max_element(estimationSize_, estimationSize_ + numMapTypes);
	photonNeighbourhood_.resize(maxSize + 1);

	if (lights().size() == 0)
	{
		return;
	}
	fillPhotonMap(sampler, period);
	buildPhotonMap();
	buildIrradianceMap();
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

	template <typename Container>
	std::string statistics(Container& container)
	{
		std::sort(container.begin(), container.end());
		size_t n = container.size();
		std::ostringstream buffer;
		buffer << std::setprecision(4) << "min=" << container.front() << ", Q1=" << container[n / 4] << ", Q2=" << container[n / 2]
			<< ", Q3=" << container[3 * n / 4] << ", max=" << container.back();
		return buffer.str();
	}
}

const XYZ PhotonMapper::doCastRay(
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
	const TPoint3D target = primaryRay.point(intersection.t());
	const XYZ mediumTransparency = mediumStack().transparency(primaryRay, intersection.t());
	const XYZ mediumInScattering = inScattering(sample, primaryRay.centralRay(), intersection.t());

	IntersectionContext context(*scene(), sample, primaryRay, intersection);
	const Shader* const shader = context.shader();
	if (!shader)
	{
		// leaving or entering something
		MediumChanger mediumChanger(mediumStack(), context.interior(), context.solidEvent());
		const DifferentialRay continuedRay = bound(primaryRay, intersection.t() + liar::tolerance);
		return mediumInScattering + mediumTransparency * this->castRay(sample, continuedRay, tIntersection, alpha);
	}
	const TBsdfPtr bsdf = shader->bsdf(sample, context);

	const TVector3D targetNormal = context.bsdfToWorld(TVector3D(0, 0, 1));
	const TVector3D omegaIn = context.worldToBsdf(-primaryRay.direction());
	LASS_ASSERT(omegaIn.z >= 0);

	if (isVisualizingPhotonMap_)
	{
		XYZ result = estimateIrradiance(target, targetNormal);
		result *= mediumTransparency;
		return mediumInScattering + result;
	}

	XYZ result = estimateCaustics(sample, context, bsdf, target, omegaIn);
	result += shader->emission(sample, context, omegaIn);

	if (isRayTracingDirect_)
	{
		result += traceDirect(sample, context, bsdf, target, targetNormal, omegaIn);
	}

	//*
	if (shader->hasCaps(Shader::capsDiffuse))
	{
		if (isRayTracingDirect_ && numFinalGatherRays_ > 0)
		{
			LASS_ASSERT(idFinalGatherSamples_ >= 0);
			Sample::TSubSequence2D gatherSample = sample.subSequence2D(idFinalGatherSamples_);
			result += gatherIndirect(sample, context, bsdf, target + 10 * liar::tolerance * targetNormal, omegaIn, gatherSample.begin(), gatherSample.end());
		}
		else
		{
			result += estimateRadiance(sample, context, bsdf, target, omegaIn);
		}
	}
	/**/

	if (shader->hasCaps(Shader::capsSpecular) || shader->hasCaps(Shader::capsGlossy))
	{
		//*
		if (shader->hasCaps(Shader::capsReflection) && shader->idReflectionSamples() != -1)
		{
			const TPoint3D beginCentral = target + 10 * liar::tolerance * targetNormal;
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

			const Sample::TSubSequence2D bsdfSample = sample.subSequence2D(shader->idReflectionSamples());
			const size_t n = generation == 0 ? bsdfSample.size() : 1;
			for (size_t i = 0; i < n; ++i)
			{
				const SampleBsdfOut out = bsdf->sample(omegaIn, bsdfSample[i], Shader::capsReflection | Shader::capsSpecular | Shader::capsGlossy);
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
		/**/
		//*
		if (shader->hasCaps(Shader::capsTransmission) && shader->idTransmissionSamples() != -1)
		{
			const MediumChanger mediumChanger(mediumStack(), context.interior(), context.solidEvent());
			const TPoint3D beginCentral = target - 10 * liar::tolerance * targetNormal;

			const Sample::TSubSequence2D bsdfSample = sample.subSequence2D(shader->idTransmissionSamples());
			const size_t n = generation == 0 ? bsdfSample.size() : 1;
			for (size_t i = 0; i < n; ++i)
			{
				const SampleBsdfOut out = bsdf->sample(omegaIn, bsdfSample[i], Shader::capsReflection | Shader::capsSpecular | Shader::capsGlossy);
				if (!out)
				{
					continue;
				}

				LASS_ASSERT(out.omegaOut.z < 0);
				const TVector3D directionCentral = context.bsdfToWorld(out.omegaOut);
				const DifferentialRay transmittedRay(
					BoundedRay(beginCentral, directionCentral, liar::tolerance),
					TRay3D(beginCentral, directionCentral),
					TRay3D(beginCentral, directionCentral));
				TScalar t, a;
				const XYZ transmitted = castRay(sample, transmittedRay, t, a);
				result += out.value * transmitted * (a * num::abs(out.omegaOut.z) / (n * out.pdf));
			}
		}
		/**/
	}

	return mediumInScattering + mediumTransparency * result;
}



const TLightSamplesRange
PhotonMapper::doSampleLights(const Sample&, const TPoint3D&, const TVector3D&) const
{
	return TLightSamplesRange();
}



const TRayTracerPtr PhotonMapper::doClone() const
{
	return TRayTracerPtr(new PhotonMapper(*this));
}



const TPyObjectPtr PhotonMapper::doGetState() const
{
	std::vector<TScalar> radius(estimationRadius_, estimationRadius_ + numMapTypes);
	std::vector<TScalar> tolerance(estimationTolerance_, estimationTolerance_ + numMapTypes);
	std::vector<size_t> size(estimationSize_, estimationSize_ + numMapTypes);
	
	return python::makeTuple(maxNumberOfPhotons_, globalMapSize_, causticsQuality_,
		numFinalGatherRays_, ratioPrecomputedIrradiance_, isVisualizingPhotonMap_,
		isRayTracingDirect_, radius, tolerance, size);
}



void PhotonMapper::doSetState(const TPyObjectPtr& state)
{
	std::vector<TScalar> radius;
	std::vector<TScalar> tolerance;
	std::vector<size_t> size;

	python::decodeTuple(state, maxNumberOfPhotons_, globalMapSize_, causticsQuality_,
		numFinalGatherRays_, ratioPrecomputedIrradiance_, isVisualizingPhotonMap_,
		isRayTracingDirect_, radius, tolerance, size);	

	LASS_ENFORCE(radius.size() == numMapTypes);
	LASS_ENFORCE(tolerance.size() == numMapTypes);
	LASS_ENFORCE(size.size() == numMapTypes);
	
	std::copy(radius.begin(), radius.end(), estimationRadius_);
	std::copy(tolerance.begin(), tolerance.end(), estimationTolerance_);
	std::copy(size.begin(), size.end(), estimationSize_);
}


namespace experimental
{
	template <typename Function>
	void runWorkers(Function worker, size_t numThreads = 0, util::ThreadKind threadKind = util::threadJoinable)
	{
		if (numThreads == 0)
		{
			numThreads = util::numberOfAvailableProcessors();
		}
		std::vector<util::Thread*> workers;
		for (size_t i = 0; i < numThreads; ++i)
		{
			workers.push_back(threadFun(worker, threadKind));
			workers.back()->run();
		}
		if (threadKind == util::threadDetached)
		{
			return;
		}
		for (size_t i = 0; i < workers.size(); ++i)
		{
			workers[i]->join();
			delete workers[i];
		}
	}
}


void PhotonMapper::fillPhotonMap(const TSamplerPtr& sampler, const TimePeriod& period)
{
	TRandomPrimary random;
	TUniformPrimary uniform(random);

	util::ProgressIndicator progress(
		"filling photon map with " + util::stringCast<std::string>(globalMapSize_) + " photons");

	size_t photonsShot = 0;
	while (photonBuffer_[mtGlobal].size() < globalMapSize_)
	{
		if (photonsShot >= maxNumberOfPhotons_)
		{
			LASS_CERR << "PhotonMapper: maximum number of " << maxNumberOfPhotons_	
				<< " photons emited before global photon map was sufficiently filled. "
				<< "Only " << photonBuffer_[mtGlobal].size() << " of the requested " 
				<< globalMapSize_ << " photons have reached the global photon map. "
				<< "Will continue with smaller map\n";
			break;
		}

		TScalar pdf;
		const LightContext* const light = lights().sample(uniform(), pdf);
		if (light && pdf > 0)
		{
			Sample sample;
			sampler->sample(period, sample);
			const TRandomSecondary::TValue secondarySeed = random();
			emitPhoton(*light, pdf, sample, secondarySeed);
		}

		progress(std::min(1., static_cast<double>(photonBuffer_[mtGlobal].size()) / globalMapSize_));

		++photonsShot;
	}

	LASS_COUT << "  total number of emitted photons: " << photonsShot << std::endl;
	const TScalar invPhotonsShot = num::inv(static_cast<TScalar>(photonsShot));
	for (int map = 0; map < numMapTypes; ++map)
	{
		for (TPhotonBuffer::iterator i = photonBuffer_[map].begin(); 
			i != photonBuffer_[map].end(); ++i)
		{
			i->power *= invPhotonsShot;
		}
	}
}



void PhotonMapper::buildPhotonMap()
{
	for (int map = 0; map < numMapTypes; ++map)
	{
		LASS_COUT << mapTypeDictionary_.key(static_cast<MapType>(map)) << " photon map:" << std::endl;
		LASS_COUT << "  number of photons: " << photonBuffer_[map].size() << std::endl;
		if (!photonBuffer_[map].empty())
		{
			std::vector<TScalar> powers;
			for (TPhotonBuffer::iterator i = photonBuffer_[map].begin(); i != photonBuffer_[map].end(); ++i)
			{
				powers.push_back(i->power.absTotal());
			}
			LASS_COUT << "  photon powers: " << temp::statistics(powers) << std::endl;
			const TScalar medianPower = powers[powers.size() / 2];
			if (estimationRadius_[map] == 0 && estimationTolerance_[map] == 0)
			{
				estimationTolerance_[map] = 0.05f;
			}
			if (estimationTolerance_[map] > 0)
			{
				const TScalar estimationArea = estimationSize_[map] * medianPower / estimationTolerance_[map];
				estimationRadius_[map] = num::sqrt(estimationArea) / TNumTraits::pi;
				LASS_COUT << "  automatic estimation radius: " << estimationRadius_[map] << std::endl;
			}
		}
		photonMap_[map].reset(photonBuffer_[map].begin(), photonBuffer_[map].end());
	}
}



void PhotonMapper::emitPhoton(
		const LightContext& light, TScalar lightPdf, const Sample& sample, 
		TRandomSecondary::TValue secondarySeed)
{
	TRandomSecondary random(secondarySeed);
	TUniformSecondary uniform(random);
	TPoint2D lightSampleA(uniform(), uniform());
	TPoint2D lightSampleB(uniform(), uniform());

	BoundedRay ray;
	TScalar pdf;
	XYZ spectrum = light.sampleEmission(sample, lightSampleA, lightSampleB, ray, pdf);
	if (pdf > 0 && spectrum)
	{
		spectrum /= lightPdf * pdf; 
		tracePhoton(sample, spectrum, ray, 0, uniform);
	}
}



void PhotonMapper::tracePhoton(
		const Sample& sample, const XYZ& power, const BoundedRay& ray, 
		int generation, TUniformSecondary& uniform, bool isCaustic)
{
	Intersection intersection;
	scene()->intersect(sample, ray, intersection);
	if (!intersection)
	{
		return;
	}

	const TPoint3D hitPoint = ray.point(intersection.t());

	const XYZ mediumTransparency = mediumStack().transparency(ray, intersection.t());
	XYZ mediumPower = mediumTransparency * power;
	const TScalar mediumAttenuation = mediumPower.average() / power.average();
	LASS_ASSERT(mediumAttenuation < 1.1);
	const TScalar mediumProbability = std::min(TNumTraits::one, mediumAttenuation);
	if (uniform() >= mediumProbability)
	{
		return;
	}
	mediumPower /= mediumProbability;

	IntersectionContext context(*scene(), sample, ray, intersection);
	const Shader* const shader = context.shader();
	if (!shader)
	{
		// entering or leaving something ...
		if (generation < maxRayGeneration())
		{
			MediumChanger mediumChanger(mediumStack(), context.interior(), context.solidEvent());
			const BoundedRay newRay = bound(ray, intersection.t() + liar::tolerance, ray.farLimit());
			tracePhoton(sample, mediumPower, newRay, generation + 1, uniform, isCaustic);
		}
		return;
	}
	shader->shadeContext(sample, context);
	const TBsdfPtr bsdf = shader->bsdf(sample, context);

	if (shader->hasCaps(Shader::capsDiffuse))
	{
		Photon photon(hitPoint, -ray.direction(), mediumPower);
		if (isCaustic)
		{
			photonBuffer_[mtCaustics].push_back(photon);
		}
		else
		{
			const bool mayStorePhoton = !isRayTracingDirect_ || (isRayTracingDirect_ && numFinalGatherRays_ > 0) || generation > 0;
			if (mayStorePhoton && uniform() < num::inv(causticsQuality_))
			{
				photon.power *= causticsQuality_;
				photonBuffer_[mtGlobal].push_back(photon);
				if (uniform() < ratioPrecomputedIrradiance_)
				{
					const TVector3D worldNormal = context.bsdfToWorld(TVector3D(0, 0, 1));
					irradianceBuffer_.push_back(Irradiance(hitPoint, worldNormal));
				}
			}
		}
	}

	// let's generate a new photon, unless ...
	if (generation == maxRayGeneration())
	{
		return;
	}
	LASS_ASSERT(generation < maxRayGeneration());

	const TVector3D omegaIn = context.worldToBsdf(-ray.direction());
	if (omegaIn.z < 0)
	{
		return;
	}

	const TPoint2D sampleBsdf(uniform(), uniform());
	const SampleBsdfOut out = bsdf->sample(omegaIn, sampleBsdf, Shader::capsAll);
	if (!out)
	{
		return;
	}

	const TScalar cos_theta = out.omegaOut.z;
	XYZ newPower = mediumPower * out.value * (num::abs(cos_theta) / out.pdf);
	const TScalar attenuation = newPower.average() / mediumPower.average();
	//LASS_ASSERT(attenuation < 1.1);
	const TScalar scatterProbability = std::min(TNumTraits::one, attenuation);
	if (uniform() < scatterProbability)
	{
		newPower /= scatterProbability;
		const BoundedRay newRay(hitPoint, context.bsdfToWorld(out.omegaOut));
		const bool isSpecular = (out.usedCaps & (Shader::capsSpecular | Shader::capsGlossy)) > 0;
		const bool newIsCaustic = isSpecular && (isCaustic || generation == 0);
		MediumChanger mediumChanger(mediumStack(), context.interior(), 
			out.omegaOut.z < 0 ? context.solidEvent() : seNoEvent);
		tracePhoton(sample, newPower, newRay, generation + 1, uniform, newIsCaustic);
	}
}



void PhotonMapper::buildIrradianceMap()
{
	irradianceMap_.reset();
	if (ratioPrecomputedIrradiance_ > 0 && numFinalGatherRays_ > 0 && isRayTracingDirect_)
	{
		const size_t size = irradianceBuffer_.size();
		const TScalar invSize = num::inv(static_cast<TScalar>(size));
		std::vector<TScalar> radii(size);
		std::vector<size_t> counts(size);
		{
			util::ProgressIndicator	progress("precomputing irradiances");
			for (size_t i = 0; i < size; ++i)
			{
				Irradiance& ir = irradianceBuffer_[i];
				ir.irradiance = estimateIrradiance(ir.position, ir.normal, ir.squaredEstimationRadius, counts[i]);
				radii[i] = num::sqrt(ir.squaredEstimationRadius);
				progress(i * invSize);
			}
		}
		LASS_COUT << "  eff. radii: " << temp::statistics(radii) << std::endl;
		LASS_COUT << "  eff. counts: " << temp::statistics(counts) << std::endl;

		irradianceMap_.reset(irradianceBuffer_.begin(), irradianceBuffer_.end());
	}
}

namespace temp
{
	inline TScalar squaredHeuristic(TScalar pdfA, TScalar pdfB)
	{
		return num::sqr(pdfA) / (num::sqr(pdfA) + num::sqr(pdfB));
	}
}

const XYZ PhotonMapper::traceDirect(
		const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& target, const TVector3D& targetNormal, 
		const TVector3D& omegaIn) const
{
	const Shader* const shader = context.shader();
	LASS_ASSERT(shader);

	XYZ result;
	const LightContexts::TIterator end = lights().end();
	for (LightContexts::TIterator light = lights().begin(); light != end; ++light)
	{
		Sample::TSubSequence2D lightSamples = sample.subSequence2D(light->idLightSamples());
		Sample::TSubSequence2D bsdfSamples = sample.subSequence2D(light->idBsdfSamples());
		/*
		Sample::TSubSequence1D componentSample = sample.subSequence1D(light->idBsdfComponentSamples());
		LASS_ASSERT(bsdfSample.size() == lightSample.size() && componentSample.size() == lightSample.size());
		*/
		const TScalar nl = static_cast<TScalar>(lightSamples.size());
		const TScalar nb = static_cast<TScalar>(bsdfSamples.size());
		const bool isMultipleImportanceSampling = nb > 0 && !light->isSingular();
		const TScalar n = isMultipleImportanceSampling ? nl + nb : nl;

		unsigned caps = Shader::capsAll & ~Shader::capsSpecular;
		if (!light->isSingular())
		{
			caps &= ~Shader::capsGlossy;
		}

		const TPoint3D start = target + 10 * liar::tolerance * targetNormal;
		for (Sample::TSubSequence2D::iterator ls = lightSamples.begin(); ls != lightSamples.end(); ++ls)
		{
			BoundedRay shadowRay;
			TScalar lightPdf;
			const XYZ radiance = light->sampleEmission(sample, *ls, start, targetNormal, shadowRay, lightPdf);
			if (lightPdf <= 0 || !radiance)
			{
				continue;
			}
			const TVector3D& omegaOut = context.worldToBsdf(shadowRay.direction());
			const BsdfOut out = bsdf->call(omegaIn, omegaOut, caps);
			if (!out || scene()->isIntersecting(sample, shadowRay))
			{
				continue;
			}
			const TScalar weight = isMultipleImportanceSampling ? temp::squaredHeuristic(nl * lightPdf, nb * out.pdf) : TNumTraits::one;
			const TScalar cosTheta = omegaOut.z;
			result += out.value * radiance * (weight * num::abs(cosTheta) / (n * lightPdf));
		}
		if (isMultipleImportanceSampling)
		{
			for (Sample::TSubSequence2D::iterator bs = lightSamples.begin(); bs != lightSamples.end(); ++bs)
			{
				const SampleBsdfOut out = bsdf->sample(omegaIn, *bs, Shader::capsAll & ~Shader::capsSpecular);
				if (!out)
				{
					continue;
				}
				const TRay3D ray(start, context.bsdfToWorld(out.omegaOut).normal());
				BoundedRay shadowRay;
				TScalar lightPdf;
				const XYZ radiance = light->emission(sample, ray, shadowRay, lightPdf);
				if (lightPdf <= 0 || !radiance || scene()->isIntersecting(sample, shadowRay))
				{
					continue;
				}
				const TScalar weight = temp::squaredHeuristic(nb * out.pdf, nl * lightPdf);
				const TScalar cosTheta = out.omegaOut.z;
				result += out.value * radiance * (weight * abs(cosTheta) / (n * out.pdf));
			}
		}
	}

	return result;
}



const XYZ PhotonMapper::gatherIndirect(
		const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& target, const TVector3D& omegaIn,
		const TPoint2D* firstSample, const TPoint2D* lastSample,
		size_t gatherStage) const
{		
	LASS_ASSERT(gatherStage < numGatherStages_);

	const bool doImportanceGathering = false;
	if (doImportanceGathering && gatherStage == 0)
	{
		const size_t gridSize = 8;
		const TScalar alpha = 0.05;
		grid_.resize(num::sqr(gridSize));
		std::fill(grid_.begin(), grid_.end(), 0);
		TScalar pTotal = 0;
		//*
		LASS_ASSERT(photonNeighbourhood_.size() > estimationSize_[mtGlobal]);
		const TPhotonNeighbourhood::const_iterator last = photonMap_[mtGlobal].rangeSearch(
			target, estimationRadius_[mtGlobal], estimationSize_[mtGlobal], photonNeighbourhood_.begin());
		for (TPhotonNeighbourhood::const_iterator i = photonNeighbourhood_.begin(); i != last; ++i)
		{
			const TVector3D omega = context.worldToBsdf(i->object()->omegaIn);
			const TScalar p = i->object()->power.absTotal();
			if (omega.z < 0)
			{
				continue;
			}
			pTotal += p;
			const TScalar u = omega.z;
			const TScalar phi = (num::abs(omega.x) < 1e-6 && num::abs(omega.y) < 1e-6) ? 0 : num::atan2(omega.y, omega.x);
			const TScalar v = (phi < 0 ? phi + 2 * TNumTraits::pi : phi) / (2 * TNumTraits::pi);
			const size_t ii = num::clamp<size_t>(static_cast<size_t>(num::floor(u * gridSize)), 0, gridSize - 1);
			const size_t jj = num::clamp<size_t>(static_cast<size_t>(num::floor(v * gridSize)), 0, gridSize - 1);
			grid_[jj * gridSize + ii] += (1 - alpha) * p;
		}
		/*/
		pTotal = 1;
		/**/
		LASS_ENFORCE(pTotal > 0);
		std::transform(grid_.begin(), grid_.end(), grid_.begin(), std::bind2nd(std::plus<TScalar>(), alpha * pTotal / num::sqr(gridSize)));
		gatherDistribution_.reset(grid_.begin(), grid_.end(), gridSize, gridSize);
	}

	XYZ result;
	const ptrdiff_t n = lastSample - firstSample;
	for (const TPoint2D* s = firstSample; s != lastSample; ++s)
	{
		SampleBsdfOut out;
		if (doImportanceGathering && gatherStage == 0)
		{
			prim::Point2D<size_t> i2;
			out.pdf = 1;
			const TPoint2D s2 = gatherDistribution_(*s, out.pdf, i2);
			TScalar pdfHalfSphere;
			out.omegaOut = num::uniformHemisphere(s2, pdfHalfSphere).position();
			out.pdf *= pdfHalfSphere;
			out.value = bsdf->call(omegaIn, out.omegaOut, Shader::capsReflection | Shader::capsDiffuse).value;
		}
		else
		{
			out = bsdf->sample(omegaIn, *s,  Shader::capsReflection | Shader::capsDiffuse);
		}
		if (!out)
		{
			continue;
		}
		const TVector3D direction = context.bsdfToWorld(out.omegaOut);
		const BoundedRay ray(target, direction, liar::tolerance);
		Intersection intersection;
		scene()->intersect(sample, ray, intersection);
		if (!intersection)
		{
			continue;
		}
		const TPoint3D hitPoint = ray.point(intersection.t());
		IntersectionContext hitContext(*scene(), sample, ray, intersection);
		const Shader* const hitShader = hitContext.shader();
		if (!hitShader)
		{
			// leaving or entering something
			//MediumChanger mediumChanger(mediumStack_, hitContext.interior(), hitContext.solidEvent());
			//const DifferentialRay continuedRay = bound(ray, intersection.t() + liar::tolerance);
			//return mediumTransparency * this->castRay(sample, continuedRay, tIntersection, alpha);
			continue;
		}
		hitShader->shadeContext(sample, hitContext);
		const TBsdfPtr hitBsdf = hitShader->bsdf(sample, hitContext);
		const TVector3D hitOmega = hitContext.worldToBsdf(-direction);
		const XYZ radiance = estimateRadiance(sample, hitContext, hitBsdf, hitPoint, hitOmega, gatherStage);
		result += out.value * radiance * num::abs(out.omegaOut.z) / (n * out.pdf);
	}
	return result;
}


namespace temp
{
	template <typename Generator>
	void stratifier(TScalar* first, TScalar* last, Generator& generator)
	{
		num::DistributionUniform<TScalar, num::RandomMT19937> uniform(generator);
		const ptrdiff_t n = last - first;
		const TScalar scale = num::inv(static_cast<TScalar>(n));
		for (ptrdiff_t k = 0; k < n; ++k)
		{
			first[k] = scale * (k + uniform());
		};
		std::random_shuffle(first, last, generator);
	}

	template <typename Generator>
	void latinHypercube(TPoint2D* first, TPoint2D* last, Generator& generator)
	{
		num::DistributionUniform<TScalar, num::RandomMT19937> uniform(generator);
		const ptrdiff_t n = last - first;
		const TPoint2D::TValue scale = num::inv(static_cast<TPoint2D::TValue>(n));
		for (ptrdiff_t k = 0; k < n; ++k)
		{
			first[k].x = scale * (k + uniform());
			first[k].y = scale * (k + uniform());
		};
		std::random_shuffle(stde::member_iterator(first, &TPoint2D::x), stde::member_iterator(last, &TPoint2D::x), generator);
		std::random_shuffle(stde::member_iterator(first, &TPoint2D::y), stde::member_iterator(last, &TPoint2D::y), generator);
	}
}



const XYZ PhotonMapper::gatherSecondary(
		const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& target, const TVector3D& omegaOut) const
{
	const size_t n = numSecondaryGatherRays_;
	TPoint2D* gatherSamples = &secondaryGatherSamples_[0];
	temp::latinHypercube(gatherSamples, gatherSamples + n, secondarySampler_);
	XYZ result = gatherIndirect(sample, context, bsdf, target, omegaOut, gatherSamples, gatherSamples + n, 1);

	//TPoint2D* lightSamples = &secondaryLightSamples_[0];
	//TScalar* lightSelectors = &secondaryLightSelectorSamples_[0];
	//temp::latinHypercube(lightSamples, lightSamples + n, secondarySampler_);
	//temp::stratifier(lightSelectors, lightSelectors + n, secondarySampler_);
	//for (size_t k = 0; k < n; ++k)
	//{
	//	TScalar pdf;
	//	const LightContext* light = lights().sample(lightSelectors[k], pdf);
	//	if (!light || pdf <= 0)
	//	{
	//		continue;
	//	}
	//}

	return result;
}


const XYZ PhotonMapper::estimateIrradiance(const TPoint3D& point, const TVector3D& normal, TScalar& sqrEstimationRadius, size_t& count) const
{
	if (!irradianceMap_.isEmpty())
	{
		TIrradianceMap::Neighbour nearest = irradianceMap_.nearestNeighbour(point, estimationRadius_[mtGlobal]);
		if (nearest.object() == irradianceMap_.end())
		{
			sqrEstimationRadius = 0;
			count = 0;
			return XYZ();
		}
		if (dot(normal, nearest->normal) > 0.9)
		{
			sqrEstimationRadius = nearest->squaredEstimationRadius;
			count = 1;
			return nearest->irradiance;
		}
	}

	LASS_ASSERT(photonNeighbourhood_.size() > estimationSize_[mtGlobal]);
	const TPhotonNeighbourhood::const_iterator last = photonMap_[mtGlobal].rangeSearch(
		point, estimationRadius_[mtGlobal], estimationSize_[mtGlobal], 
		photonNeighbourhood_.begin());

	count = static_cast<size_t>(last - photonNeighbourhood_.begin());
	if (count == 0)
	{
		sqrEstimationRadius = 0;
		return XYZ();
	}

	XYZ result;
	for (TPhotonNeighbourhood::const_iterator i = photonNeighbourhood_.begin(); i != last; ++i)
	{
		if (dot(i->object()->omegaIn, normal) > 0)
		{
			result += i->object()->power;
		}
	}

	sqrEstimationRadius = photonNeighbourhood_.front().squaredDistance();
	return sqrEstimationRadius > 0 ? result / (TNumTraits::pi * sqrEstimationRadius) : XYZ();
}



const XYZ PhotonMapper::estimateRadiance(
		const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf, 
		const TPoint3D& point, const TVector3D& omegaOut, size_t gatherStage) const
{
	const Shader* const shader = context.shader();
	if (!shader || !shader->hasCaps(Shader::capsDiffuse))
	{
		return XYZ();
	}

	//*
	/**/

	if (!irradianceMap_.isEmpty())
	{
		TIrradianceMap::Neighbour nearest = irradianceMap_.nearestNeighbour(point, estimationRadius_[mtGlobal]);
		if (nearest.object() == irradianceMap_.end())
		{
			return XYZ();
		}
		if (num::sqr(context.t()) < nearest->squaredEstimationRadius && gatherStage == 0 && numSecondaryGatherRays_ > 0)
		{
			return gatherSecondary(sample, context, bsdf, point, omegaOut);
		}
		//if (dot(normal, nearest.normal) > 0.9)
		{
			const BsdfOut out = bsdf->call(omegaOut, context.worldToBsdf(nearest->normal), Shader::capsAll);
			return out ? out.value * nearest->irradiance : XYZ();
		}
	}

	LASS_ASSERT(photonNeighbourhood_.size() > estimationSize_[mtGlobal]);
	const TPhotonNeighbourhood::const_iterator last = photonMap_[mtGlobal].rangeSearch(
		point, estimationRadius_[mtGlobal], estimationSize_[mtGlobal], 
		photonNeighbourhood_.begin());
	
	const TPhotonNeighbourhood::difference_type n = last - photonNeighbourhood_.begin();
	if (n < 2)
	{
		return XYZ();
	}

	if (num::sqr(context.t()) < photonNeighbourhood_.front().squaredDistance() && gatherStage == 0 && numSecondaryGatherRays_ > 0)
	{
		return gatherSecondary(sample, context, bsdf, point, omegaOut);
	}

	XYZ result;
	for (TPhotonNeighbourhood::const_iterator i = photonNeighbourhood_.begin(); i != last; ++i)
	{
		const TVector3D omegaPhoton = context.worldToBsdf(i->object()->omegaIn);
		const BsdfOut out = bsdf->call(omegaOut, omegaPhoton, Shader::capsAll & ~Shader::capsSpecular & ~Shader::capsGlossy);
		if (out.pdf > 0 && out.value)
		{
			result += out.value * i->object()->power;
		}
	}

	return result / (TNumTraits::pi * photonNeighbourhood_.front().squaredDistance());
}



const XYZ PhotonMapper::estimateCaustics(
		const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf, 
		const TPoint3D& point, const TVector3D& omegaIn) const
{
	const Shader* const shader = context.shader();
	if (!shader || !shader->hasCaps(Shader::capsDiffuse))
	{
		return XYZ();
	}

	LASS_ASSERT(photonNeighbourhood_.size() > estimationSize_[mtGlobal]);
	const TPhotonNeighbourhood::const_iterator last = photonMap_[mtCaustics].rangeSearch(
		point, estimationRadius_[mtCaustics], estimationSize_[mtCaustics],
		photonNeighbourhood_.begin());
	const TPhotonNeighbourhood::difference_type n = last - photonNeighbourhood_.begin();
	LASS_ASSERT(n >= 0);
	if (n < 2)
	{
		return XYZ();
	}

	const TScalar sqrSize = photonNeighbourhood_[0].squaredDistance();
	const TScalar alpha = 0.918f;
	const TScalar beta = 1.953f;
	const TScalar b1 = -beta / 2 * sqrSize;
	const TScalar b2 = num::inv(1 - num::exp(-beta));

	XYZ result;
	for (int i = 0; i < n; ++i)
	{
		const TVector3D omegaPhoton = context.worldToBsdf(photonNeighbourhood_[i].object()->omegaIn);
		const BsdfOut out = bsdf->call(omegaIn, omegaPhoton, Shader::capsAllDiffuse);
		if (out)
		{
			const TScalar sqrR = photonNeighbourhood_[i].squaredDistance();
			const TScalar w = alpha * (1 - b2 * (1 - num::exp(b1 * sqrR)));
			result += w * out.value * photonNeighbourhood_[i].object()->power;
		}
	}

	return result / (TNumTraits::pi * sqrSize);
}



const XYZ PhotonMapper::inScattering(const Sample& sample, const kernel::BoundedRay& ray, TScalar tFar) const
{
	typedef Sample::TSubSequence1D::difference_type difference_type;

	const Medium* medium = mediumStack().medium();
	const TScalar tNear = ray.nearLimit();
	if (!medium || medium->numScatterSamples() == 0 || (tNear > tFar))
	{
		return XYZ();
	}

	const Sample::TSubSequence1D stepSamples = sample.subSequence1D(medium->idStepSamples()); // these are unsorted!!!
	const Sample::TSubSequence1D lightSamples = sample.subSequence1D(medium->idLightSamples());
	const Sample::TSubSequence2D surfaceSamples = sample.subSequence2D(medium->idSurfaceSamples());

	const difference_type n = stepSamples.size();
	LASS_ASSERT(lightSamples.size() == n && surfaceSamples.size() == n);

	XYZ result;	
	for (difference_type k = 0; k < n; ++k)
	{
		const TScalar t = num::lerp(tNear, tFar, stepSamples[k]);
		const TPoint3D point = ray.point(t);
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
		const XYZ trans = medium->transparency(bound(ray, tNear, t));
		result += trans * phase * radiance / (n * lightPdf * surfacePdf);
	}

	return result;
}



void PhotonMapper::updateActualEstimationRadius(MapType mapType, TScalar radius) const
{
	maxActualEstimationRadius_[mapType] = std::max(maxActualEstimationRadius_[mapType], radius);
}



PhotonMapper::TMapTypeDictionary PhotonMapper::generateMapTypeDictionary()
{
	TMapTypeDictionary dictionary;
	dictionary.enableSuggestions(true);
	dictionary.add("global", mtGlobal);
	dictionary.add("caustic", mtCaustics);
	dictionary.add("volume", mtVolume);
	return dictionary;
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
