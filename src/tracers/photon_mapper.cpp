/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2006  Bram de Greve
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
#include "photon_mapper.h"
#include "../kernel/per_thread_buffer.h"
#include <lass/num/distribution.h>
#include <lass/util/progress_indicator.h>
#include <lass/stde/range_algorithm.h>
#include <lass/stde/extended_iterator.h>
#include <lass/util/thread.h>

#define EVAL(x) LASS_COUT << LASS_STRINGIFY(x) << ": " << (x) << std::endl

namespace liar
{
namespace tracers
{

PY_DECLARE_CLASS(PhotonMapper)
PY_CLASS_CONSTRUCTOR_0(PhotonMapper)
PY_CLASS_MEMBER_RW_DOC(PhotonMapper, "maxNumberOfPhotons", maxNumberOfPhotons, setMaxNumberOfPhotons,
	"The maximum number of photons being emitted by the light sources.\n"
	"Set this to a very high number (at least globalMapSize * causticsQuality) to prevent the "
	"first pass to end without sufficient photons in the global photon map.\n")
PY_CLASS_MEMBER_RW_DOC(PhotonMapper, "globalMapSize", globalMapSize, setGlobalMapSize,
	"the requested number of photons in the global photon map.\n")
PY_CLASS_MEMBER_RW_DOC(PhotonMapper, "causticsQuality", causticsQuality, setCausticsQuality,
	"the quality ratio of the caustics photon map versus global photon map. "
	"Increase if more photons in the caustics map are wanted for higher quality rendering.\n")
PY_CLASS_METHOD(PhotonMapper, estimationRadius)
PY_CLASS_METHOD(PhotonMapper, setEstimationRadius)
PY_CLASS_METHOD(PhotonMapper, estimationTolerance)
PY_CLASS_METHOD(PhotonMapper, setEstimationTolerance)
PY_CLASS_METHOD(PhotonMapper, estimationSize)
PY_CLASS_METHOD(PhotonMapper, setEstimationSize)
PY_CLASS_MEMBER_RW_DOC(PhotonMapper, "numFinalGatherRays", numFinalGatherRays, setNumFinalGatherRays,
	"- if numFinalGatherRays > 0 and isRayTracingDirect == True, a final gather step with "
	"numFinalGatherRays gather rays is used to estimate the indirect lighting.\n"
	"- else, the indirect lighting is estimated by a direct query of the photon map.\n"
	"\n"
	"Set to zero to disable the final gather step\n")
PY_CLASS_MEMBER_RW_DOC(PhotonMapper, "ratioPrecomputedIrradiance", ratioPrecomputedIrradiance, setRatioPrecomputedIrradiance,
	"- if ratioPrecomputedIrradiance > 0 and numFinalGatherRays > 0 and isRayTracingDirect == True, "
	"the final gather step is optimized by precomputing irradiances. "
	"- else, a full radiance estimation is done for each gather ray.\n"
	"\n"
	"Set to zero to disable precomputed irradiance estimations.\n")
PY_CLASS_MEMBER_RW_DOC(PhotonMapper, "isVisualizingPhotonMap", isVisualizingPhotonMap, setVisualizePhotonMap,
	"if True, the content of the photon map is directly visualized (without applying material properties)\n"
	"if False, a proper render is done =)\n")
PY_CLASS_MEMBER_RW_DOC(PhotonMapper, "isRayTracingDirect", isRayTracingDirect, setRayTracingDirect,
	"if True, the direct lighting is estimated by sampling the scene lights\n"
	"if False, the direct lighting is estimated by a direct query of the photon map\n")
	
PhotonMapper::TMapTypeDictionary PhotonMapper::mapTypeDictionary_ = 
	PhotonMapper::generateMapTypeDictionary();


// --- public --------------------------------------------------------------------------------------

PhotonMapper::PhotonMapper():
	mediumStack_(),
	globalMapSize_(10000),
	causticsQuality_(5),
	maxNumberOfPhotons_(100000000),
	numFinalGatherRays_(0),
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
	}
}



const unsigned PhotonMapper::maxNumberOfPhotons() const
{
	return maxNumberOfPhotons_;
}



void PhotonMapper::setMaxNumberOfPhotons(unsigned maxNumberOfPhotons)
{
	maxNumberOfPhotons_ = maxNumberOfPhotons;
}



const unsigned PhotonMapper::globalMapSize() const
{
	return globalMapSize_;
}



void PhotonMapper::setGlobalMapSize(unsigned mapSize)
{
	globalMapSize_ = mapSize;
}



const TScalar PhotonMapper::causticsQuality() const
{
	return causticsQuality_;
}



void PhotonMapper::setCausticsQuality(TScalar quality)
{
	causticsQuality_ = std::max<TScalar>(quality, 1);
}



const TScalar PhotonMapper::estimationRadius(const std::string& mapType) const
{
	return estimationRadius_[mapTypeDictionary_[mapType]];
}



void PhotonMapper::setEstimationRadius(const std::string& mapType, TScalar radius)
{
	estimationRadius_[mapTypeDictionary_[mapType]] = std::max(radius, TNumTraits::zero);
}



const TScalar PhotonMapper::estimationTolerance(const std::string& mapType) const
{
	return estimationTolerance_[mapTypeDictionary_[mapType]];
}



void PhotonMapper::setEstimationTolerance(const std::string& mapType, TScalar tolerance)
{
	estimationTolerance_[mapTypeDictionary_[mapType]] = std::max(tolerance, TNumTraits::zero);
}



const unsigned PhotonMapper::estimationSize(const std::string& mapType) const
{
	return estimationSize_[mapTypeDictionary_[mapType]];
}



void PhotonMapper::setEstimationSize(const std::string& mapType, unsigned size)
{
	estimationSize_[mapTypeDictionary_[mapType]] = std::max<unsigned>(size, 1);
}



const unsigned PhotonMapper::numFinalGatherRays() const
{
	return numFinalGatherRays_;
}



void PhotonMapper::setNumFinalGatherRays(unsigned numFinalGatherRays)
{
	numFinalGatherRays_ = numFinalGatherRays;
}



const TScalar PhotonMapper::ratioPrecomputedIrradiance() const
{
	return ratioPrecomputedIrradiance_;
}



void PhotonMapper::setRatioPrecomputedIrradiance(TScalar ratio)
{
	ratioPrecomputedIrradiance_ = num::clamp(ratio, TNumTraits::zero, TNumTraits::one);
}



const bool PhotonMapper::isVisualizingPhotonMap() const
{
	return isVisualizingPhotonMap_;
}



void PhotonMapper::setVisualizePhotonMap(bool enabled)
{
	isVisualizingPhotonMap_ = enabled;
}



const bool PhotonMapper::isRayTracingDirect() const
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
	idFinalGatherSamples_ = numFinalGatherRays_ > 0 ? sampler->requestSubSequence2D(numFinalGatherRays_) : -1;
}



void PhotonMapper::doPreProcess(const kernel::TSamplerPtr& sampler, const TimePeriod& period)
{
	const unsigned maxSize = *std::max_element(estimationSize_, estimationSize_ + numMapTypes);
	photonNeighbourhood_.resize(maxSize + 1);

	TLightCdf lightCdf;
	for (TLightContexts::const_iterator i = lights().begin(); i != lights().end(); ++i)
	{
		lightCdf.push_back(i->totalPower().average());
	}
	std::partial_sum(lightCdf.begin(), lightCdf.end(), lightCdf.begin());
	std::transform(lightCdf.begin(), lightCdf.end(), lightCdf.begin(), 
		std::bind2nd(std::divides<TScalar>(), lightCdf.back()));

	fillPhotonMap(lightCdf, sampler, period);
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
}

const Spectrum PhotonMapper::doCastRay(
		const kernel::Sample& sample, const kernel::DifferentialRay& primaryRay,
		TScalar& alpha, unsigned generation) const
{
	Intersection intersection;
	scene()->intersect(sample, primaryRay, intersection);
	if (!intersection)
	{
		alpha = 0;
		return Spectrum();
	}
	alpha = 1;
	const TPoint3D target = primaryRay.point(intersection.t());
	const Spectrum mediumTransparency = mediumStack_.transparency(BoundedRay(primaryRay.centralRay().unboundedRay(), primaryRay.centralRay().nearLimit(), intersection.t()));

	IntersectionContext context(this);
	scene()->localContext(sample, primaryRay, intersection, context);
	context.flipTo(-primaryRay.direction());

	const Shader* const shader = context.shader();
	if (!shader)
	{
		// leaving or entering something
		MediumChanger mediumChanger(mediumStack_, context.interior(), context.solidEvent());
		const DifferentialRay continuedRay = bound(primaryRay, intersection.t() + tolerance);
		return mediumTransparency * this->castRay(sample, continuedRay, alpha);
	}
	shader->shadeContext(sample, context);

	const TVector3D targetNormal = context.shaderToWorld(TVector3D(0, 0, 1));
	const TVector3D omegaIn = context.worldToShader(-primaryRay.direction());
	LASS_ASSERT(omegaIn.z >= 0);

	if (isVisualizingPhotonMap_)
	{
		Spectrum result = estimateIrradiance(target, targetNormal);
		result *= mediumTransparency;
		return result;
	}

	Spectrum result = estimateCaustics(sample, context, target, omegaIn);
	result += shader->emission(sample, context, omegaIn);

	if (isRayTracingDirect_)
	{
		result += traceDirect(sample, context, target, targetNormal, omegaIn);
	}

	//*
	if (shader->hasCaps(Shader::capsDiffuse))
	{
		if (isRayTracingDirect_ && numFinalGatherRays_ > 0)
		{
			LASS_ASSERT(idFinalGatherSamples_ >= 0);
			Sample::TSubSequence2D gatherSample = sample.subSequence2D(idFinalGatherSamples_);
			result += gatherIndirect(
				sample, context, target, omegaIn, gatherSample.begin(), gatherSample.end());
		}
		else
		{
			result += estimateRadiance(sample, context, target, omegaIn);
		}
	}
	/**/

	if (shader->hasCaps(Shader::capsSpecular) || shader->hasCaps(Shader::capsGlossy))
	{
		//*
		if (shader->hasCaps(Shader::capsReflection) && shader->idReflectionSamples() != -1)
		{
			Sample::TSubSequence2D bsdfSample = sample.subSequence2D(shader->idReflectionSamples());
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
					const TVector3D directionCentral = context.shaderToWorld(out[i].omegaOut);
					LASS_ASSERT(dot(normal, directionCentral) > 0);
					LASS_ASSERT(dot(normal, incident) < 0);
					const TVector3D directionI = directionCentral + dReflected_dI;
					const TVector3D directionJ = directionCentral + dReflected_dJ;

					const DifferentialRay reflectedRay(
						BoundedRay(beginCentral, directionCentral, tolerance),
						TRay3D(beginI, directionI),
						TRay3D(beginJ, directionJ));
					TScalar a;
					const Spectrum reflected = castRay(sample, reflectedRay, a);
					result +=out[i].value * reflected * (a * num::abs(out[i].omegaOut.z) / (n * out[i].pdf));
				}
			}
		}
		/**/
		//*
		if (shader->hasCaps(Shader::capsTransmission) && shader->idTransmissionSamples() != -1)
		{
			MediumChanger mediumChanger(mediumStack_, context.interior(), context.solidEvent());

			Sample::TSubSequence2D bsdfSample = sample.subSequence2D(shader->idTransmissionSamples());
			const size_t n = generation == 0 ? bsdfSample.size() : 1;
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
					const TVector3D directionCentral = context.shaderToWorld(out[i].omegaOut);

					const DifferentialRay transmittedRay(
						BoundedRay(beginCentral, directionCentral, tolerance),
						TRay3D(beginCentral, directionCentral),
						TRay3D(beginCentral, directionCentral));
					TScalar a;
					const Spectrum transmitted = castRay(sample, transmittedRay, a);
					result +=out[i].value * transmitted * (a * num::abs(out[i].omegaOut.z) / (n * out[i].pdf));
				}
			}
		}
		/**/
	}

	alpha = 1;
	return mediumTransparency * result;
}



const TLightSamplesRange
PhotonMapper::doSampleLights(const Sample& iSample, const TPoint3D& iTarget, 
		const TVector3D& iTargetNormal) const
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
	std::vector<unsigned> size(estimationSize_, estimationSize_ + numMapTypes);
	
	return python::makeTuple(maxNumberOfPhotons_, globalMapSize_, causticsQuality_, numFinalGatherRays_,
		ratioPrecomputedIrradiance_, isVisualizingPhotonMap_, isRayTracingDirect_,
		radius, tolerance, size);
}



void PhotonMapper::doSetState(const TPyObjectPtr& state)
{
	std::vector<TScalar> radius;
	std::vector<TScalar> tolerance;
	std::vector<unsigned> size;

	python::decodeTuple(state, maxNumberOfPhotons_, globalMapSize_, causticsQuality_, numFinalGatherRays_,
		ratioPrecomputedIrradiance_, isVisualizingPhotonMap_, isRayTracingDirect_,
		radius, tolerance, size);	

	LASS_ENFORCE(radius.size() == numMapTypes);
	LASS_ENFORCE(tolerance.size() == numMapTypes);
	LASS_ENFORCE(size.size() == numMapTypes);
	
	std::copy(radius.begin(), radius.end(), estimationRadius_);
	std::copy(tolerance.begin(), tolerance.end(), estimationTolerance_);
	std::copy(size.begin(), size.end(), estimationSize_);
}



void PhotonMapper::fillPhotonMap(
		const TLightCdf& lightCdf, const TSamplerPtr& sampler, const TimePeriod& period)
{
	if (lightCdf.empty())
	{
		return;
	}

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
				<< " photons emited before global photon map was sufficiently filled. Only "
				<< photonBuffer_[mtGlobal].size() << " of the requested " << globalMapSize_
				<< " photons have reached the global photon map.  Will continue with smaller map\n";
			break;
		}

		const int light = static_cast<int>(
			std::lower_bound(lightCdf.begin(), lightCdf.end(), uniform()) - lightCdf.begin());
		const TScalar lightPdf = 
			lightCdf[light] - (light > 0 ? lightCdf[light - 1] : TNumTraits::zero);

		if (lightPdf > 0)
		{
			Sample sample;
			sampler->sample(period, sample);
			const TRandomSecondary::TValue secondarySeed = random();
            emitPhoton(lights()[light], lightPdf, sample, secondarySeed);
		}

		progress(std::min(1., static_cast<double>(photonBuffer_[mtGlobal].size()) / globalMapSize_));

		++photonsShot;
	}

    const TScalar invPhotonsShot = num::inv(static_cast<TScalar>(photonsShot));
	for (int map = 0; map < numMapTypes; ++map)
	{
		for (TPhotonBuffer::iterator i = photonBuffer_[map].begin(); i != photonBuffer_[map].end(); ++i)
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

			std::sort(powers.begin(), powers.end());
			const TScalar minPower = powers.front();
			const TScalar maxPower = powers.back();
			const TScalar medianPower = powers[powers.size() / 2];
			LASS_COUT << "  photon power: min " << minPower << ", median " << medianPower 
				<< ", max " << maxPower << std::endl;
			if (estimationRadius_[map] == 0 && estimationTolerance_[map] == 0)
			{
				estimationTolerance_[map] = 0.05f;
			}
			if (estimationTolerance_[map] > 0)
			{
				estimationRadius_[map] = 
					num::sqrt(estimationSize_[map] * medianPower / estimationTolerance_[map]) 
					/ TNumTraits::pi;
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
	Spectrum spectrum = light.sampleEmission(sample, lightSampleA, lightSampleB, ray, pdf);
	if (pdf > 0 && spectrum)
	{
		spectrum /= lightPdf * pdf; 
		tracePhoton(sample, spectrum, ray, 0, uniform);
	}
}



void PhotonMapper::tracePhoton(
		const Sample& sample, const Spectrum& power, const BoundedRay& ray, 
		unsigned generation, TUniformSecondary& uniform, bool isCaustic)
{
	Intersection intersection;
	scene()->intersect(sample, ray, intersection);
	if (!intersection)
	{
		return;
	}

	const TPoint3D hitPoint = ray.point(intersection.t());

	const Spectrum mediumTransparency = mediumStack_.transparency(BoundedRay(ray.unboundedRay(), ray.nearLimit(), intersection.t()));
	Spectrum mediumPower = mediumTransparency * power;
	const TScalar mediumAttenuation = mediumPower.average() / power.average();
	LASS_ASSERT(mediumAttenuation < 1.1);
	const TScalar mediumProbability = std::min(TNumTraits::one, mediumAttenuation);
	if (uniform() >= mediumProbability)
	{
		return;
	}
	mediumPower /= mediumProbability;

	IntersectionContext context(this);
	scene()->localContext(sample, ray, intersection, context);
	context.flipTo(-ray.direction());
	const Shader* const shader = context.shader();
	if (!shader)
	{
		// entering or leaving something ...
		if (generation < maxRayGeneration())
		{
			MediumChanger mediumChanger(mediumStack_, context.interior(), context.solidEvent());
			const BoundedRay newRay(
				ray.unboundedRay(), intersection.t() + tolerance, ray.farLimit());
			tracePhoton(sample, mediumPower, newRay, generation + 1, uniform, isCaustic);
		}
		return;
	}
	shader->shadeContext(sample, context);

	if (shader->hasCaps(Shader::capsDiffuse))
	{
		Photon photon(hitPoint, -ray.direction(), mediumPower);
		if (isCaustic)
		{
			photonBuffer_[mtCaustics].push_back(photon);
		}
		else
		{
			const bool mayStorePhoton = !isRayTracingDirect_ || 
				(isRayTracingDirect_ && numFinalGatherRays_ > 0) || generation > 0;
			if (mayStorePhoton && uniform() < num::inv(causticsQuality_))
			{
				photon.power *= causticsQuality_;
				photonBuffer_[mtGlobal].push_back(photon);
				if (uniform() < ratioPrecomputedIrradiance_)
				{
					const TVector3D worldNormal = context.shaderToWorld(TVector3D(0, 0, 1));
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

	const TVector3D omegaIn = context.worldToShader(-ray.direction());
	const SampleBsdfIn in(TPoint2D(uniform(), uniform()), Shader::capsAll);
	SampleBsdfOut out;
	context.shader()->sampleBsdf(sample, context, omegaIn, &in, &in + 1, &out);
	if (out.pdf == 0 || !out.value)
	{
		return;
	}

	const TScalar cos_theta = out.omegaOut.z;
	Spectrum newPower = mediumPower * out.value * (abs(cos_theta) / out.pdf);
	const TScalar attenuation = newPower.average() / mediumPower.average();
	//LASS_ASSERT(attenuation < 1.1);
	const TScalar scatterProbability = std::min(TNumTraits::one, attenuation);
	if (uniform() < scatterProbability)
	{
		newPower /= scatterProbability;
		const BoundedRay newRay(hitPoint, context.shaderToWorld(out.omegaOut), tolerance);
		const bool newIsCaustic = (out.usedCaps & (Shader::capsSpecular | Shader::capsGlossy)) > 0;
		MediumChanger mediumChanger(mediumStack_, context.interior(), out.omegaOut.z < 0 ? context.solidEvent() : seNoEvent);
		tracePhoton(sample, newPower, newRay, generation + 1, uniform, newIsCaustic);
	}
}



void PhotonMapper::buildIrradianceMap()
{
	irradianceMap_.reset();
	if (ratioPrecomputedIrradiance_ > 0 && numFinalGatherRays_ > 0 && isRayTracingDirect_)
	{
		util::ProgressIndicator	progress("precomputing irradiances");
		const size_t size = irradianceBuffer_.size();
		const TScalar invSize = num::inv(static_cast<TScalar>(size));
		for (size_t i = 0; i < size; ++i)
		{
			Irradiance& ir = irradianceBuffer_[i];
			ir.irradiance = estimateIrradiance(ir.position, ir.normal);
			progress(i * invSize);
		}
		irradianceMap_.reset(irradianceBuffer_.begin(), irradianceBuffer_.end());
	}
}

namespace temp
{
	TScalar squaredHeuristic(TScalar pdfA, TScalar pdfB)
	{
		return num::sqr(pdfA) / (num::sqr(pdfA) + num::sqr(pdfB));
	}
}

const Spectrum PhotonMapper::traceDirect(
		const Sample& sample, const IntersectionContext& context,
		const TPoint3D& target, const TVector3D& targetNormal, 
		const TVector3D& omegaIn) const
{
	const Shader* const shader = context.shader();
	LASS_ASSERT(shader);

	Spectrum result;
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
			const Spectrum radiance = light->sampleEmission(
				sample, *lightSample, target + 2 * tolerance * targetNormal, targetNormal, shadowRay, lightPdf);
			if (lightPdf > 0 && radiance)
			{
				const TVector3D& omegaOut = context.worldToShader(shadowRay.direction());
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
						(weight * abs(cosTheta) / (n * lightPdf));
				}
			}
			/*
			if (!isSingularLight)
			{
				TVector3D omegaIn;
				Spectrum bsdf;
				TScalar bsdfPdf;
				shader->sampleBsdf(sample, context, omegaOut, bsdfSample.begin(), bsdfSample.begin() + 1, 
					&omegaIn, &bsdf, &bsdfPdf, Shader::capsAll & ~Shader::capsSpecular);
				if (bsdfPdf > 0 && bsdf)
				{
					TRay3D ray(target, context.shaderToWorld(omegaIn));
					BoundedRay shadowRay;
					TScalar lightPdf;
					const Spectrum radiance = light->emission(sample, ray, shadowRay, lightPdf);
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



const Spectrum PhotonMapper::gatherIndirect(
		const Sample& sample, const IntersectionContext& context,
		const TPoint3D& target, const TVector3D& omegaIn,
		const TPoint2D* firstSample, const TPoint2D* lastSample,
		size_t gatherStage) const
{		
	LASS_ASSERT(gatherStage < numGatherStages_);
	std::vector<SampleBsdfIn>& in = sampleBsdfIns_[gatherStage];
	std::vector<SampleBsdfOut>& out = sampleBsdfOuts_[gatherStage];

	const int n = static_cast<int>(lastSample - firstSample);
	if (n > in.size())
	{
		in.resize(n);
		out.resize(n);
	}

	for (int i = 0; i < n; ++i)
	{
		in[i].sample = firstSample[i];
		in[i].allowedCaps = Shader::capsReflection | Shader::capsDiffuse;
	}

	LASS_ASSERT(context.shader());
	context.shader()->sampleBsdf(sample, context, omegaIn, &in[0], &in[0] + n, &out[0]);

	Spectrum result;
	for (int i = 0; i < n; ++i)
	{
		if (out[i].pdf > 0 && out[i].value)
		{
			const TVector3D direction = context.shaderToWorld(out[i].omegaOut);
			const BoundedRay ray(target, direction, tolerance);
			Intersection intersection;
			scene()->intersect(sample, ray, intersection);
			if (intersection)
			{
				const TPoint3D hitPoint = ray.point(intersection.t());
				IntersectionContext hitContext(this);
				scene()->localContext(sample, ray, intersection, hitContext);
				hitContext.flipTo(-ray.direction());
				const TVector3D hitOmega = hitContext.worldToShader(-direction);
				const Spectrum radiance = estimateRadiance(
					sample, hitContext, hitPoint, hitOmega, gatherStage);
				result += out[i].value * radiance * abs(out[i].omegaOut.z) / (n * out[i].pdf);
			}
		}
	}
	return result;
}



const Spectrum PhotonMapper::estimateIrradiance(
		const TPoint3D& point, const TVector3D& normal) const
{
	if (!irradianceMap_.isEmpty())
	{
		const Irradiance& nearest = *irradianceMap_.nearestNeighbour(point);
		if (dot(normal, nearest.normal) > 0.9)
		{
			return nearest.irradiance;
		}
	}

	const TPhotonNeighbourhood::const_iterator last = photonMap_[mtGlobal].rangeSearch(
		point, estimationRadius_[mtGlobal], estimationSize_[mtGlobal], 
		photonNeighbourhood_.begin());
	if (last == photonNeighbourhood_.begin())
	{
		return Spectrum();
	}

	Spectrum result;
	for (TPhotonNeighbourhood::const_iterator i = photonNeighbourhood_.begin(); i != last; ++i)
	{
		if (dot(i->object()->omegaIn, normal) > 0)
		{
			result += i->object()->power;
		}
	}

	return result / (TNumTraits::pi * photonNeighbourhood_.front().squaredDistance());
}



const Spectrum PhotonMapper::estimateRadiance(
		const Sample& sample, const IntersectionContext& context, 
		const TPoint3D& point, const TVector3D& omegaOut,
		size_t gatherStage) const
{
	const Shader* const shader = context.shader();
	if (!shader || !shader->hasCaps(Shader::capsDiffuse))
	{
		return Spectrum();
	}

	if (context.t() < estimationRadius_[mtGlobal] && gatherStage + 1 < numGatherStages_)
	{
		static util::ThreadLocalVariable< num::RandomMT19937 > random2nd;
		static util::ThreadLocalVariable< num::DistributionUniform<TScalar, num::RandomMT19937> > uniform2nd(*random2nd);
		static PerThreadBuffer<TPoint2D> samples2nd;
		const size_t n = 8;
		samples2nd.growTo(n);
		for (size_t i = 0; i < n; ++i)
		{
			samples2nd[i] = TPoint2D((*uniform2nd)(), (*uniform2nd)());
		}
		return gatherIndirect(sample, context, point, omegaOut, 
			samples2nd.begin(), samples2nd.begin() + n, gatherStage + 1);
	}

	if (!irradianceMap_.isEmpty())
	{
		const Irradiance& nearest = *irradianceMap_.nearestNeighbour(point);
		//if (dot(normal, nearest.normal) > 0.9)
		{
			BsdfIn in(context.worldToShader(nearest.normal), Shader::capsAll);
			BsdfOut out;
			shader->bsdf(sample, context, omegaOut, &in, &in + 1, &out);
			return out.pdf > 0 && out.value ? out.value * nearest.irradiance : Spectrum();
		}
	}

	const TPhotonNeighbourhood::const_iterator last = photonMap_[mtGlobal].rangeSearch(
		point, estimationRadius_[mtGlobal], estimationSize_[mtGlobal], 
		photonNeighbourhood_.begin());
	if (last == photonNeighbourhood_.begin())
	{
		return Spectrum();
	}

	Spectrum result;
	for (TPhotonNeighbourhood::const_iterator i = photonNeighbourhood_.begin(); i != last; ++i)
	{
		const TVector3D omegaPhoton = context.worldToShader(i->object()->omegaIn);
		BsdfIn in(omegaPhoton, Shader::capsAll & ~Shader::capsSpecular & ~Shader::capsGlossy);
		BsdfOut out;
		shader->bsdf(sample, context, omegaOut, &in, &in + 1, &out);
		if (out.pdf > 0 && out.value)
		{
			result += out.value * i->object()->power;
		}
	}

	return result / (TNumTraits::pi * photonNeighbourhood_.front().squaredDistance());
}



const Spectrum PhotonMapper::estimateCaustics(
		const Sample& sample, const IntersectionContext& context, 
		const TPoint3D& point, const TVector3D& omegaIn) const
{
	const Shader* const shader = context.shader();
	if (!shader || !shader->hasCaps(Shader::capsDiffuse))
	{
		return Spectrum();
	}

	const TPhotonNeighbourhood::const_iterator last = photonMap_[mtCaustics].rangeSearch(
		point, estimationRadius_[mtCaustics], estimationSize_[mtCaustics], photonNeighbourhood_.begin());
	const int n = last - photonNeighbourhood_.begin();
	if (n < 2)
	{
		return Spectrum();
	}
	
	if (n > bsdfIns_.size())
	{
		bsdfIns_.resize(n);
		bsdfOuts_.resize(n);
	}
	for (int i = 0; i < n; ++i)
	{
		const TVector3D omegaPhoton = 
			context.worldToShader(photonNeighbourhood_[i].object()->omegaIn);
		bsdfIns_[i] = BsdfIn(omegaPhoton, Shader::capsAllDiffuse);
	};

	shader->bsdf(sample, context, omegaIn, &bsdfIns_[0], &bsdfIns_[0] + n, &bsdfOuts_[0]);

	const TScalar sqrSize = photonNeighbourhood_[0].squaredDistance();
	const TScalar alpha = 0.918f;
	const TScalar beta = 1.953f;
	const TScalar b1 = -beta / 2 * sqrSize;
	const TScalar b2 = num::inv(1 - num::exp(-beta));

	Spectrum result;
	for (int i = 0; i < n; ++i)
	{
		if (bsdfOuts_[i].pdf > 0)
		{
			const TScalar sqrR = photonNeighbourhood_[i].squaredDistance();
			const TScalar w = alpha * (1 - b2 * (1 - num::exp(b1 * sqrR)));
			result += w * bsdfOuts_[i].value * photonNeighbourhood_[i].object()->power;
		}
	}

	return result / (TNumTraits::pi * sqrSize);
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
