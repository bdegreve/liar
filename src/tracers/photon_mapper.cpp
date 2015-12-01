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
#include "photon_mapper.h"
#include "../kernel/per_thread_buffer.h"
#include <lass/num/distribution.h>
#include <lass/num/distribution_transformations.h>
#include <lass/util/progress_indicator.h>
#include <lass/stde/range_algorithm.h>
#include <lass/stde/extended_iterator.h>
#include <lass/stde/extended_string.h>
#include <lass/stde/overwrite_insert_iterator.h>
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
PY_CLASS_MEMBER_RW_DOC(PhotonMapper, volumetricQuality, setVolumetricQuality,
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
PY_CLASS_MEMBER_RW_DOC(PhotonMapper, volumetricGatherQuality, setVolumetricGatherQuality,
	"- Value between 0 and 1: ratio of the final gather rays that also collect in scattering.")
PY_CLASS_MEMBER_RW_DOC(PhotonMapper, isVisualizingPhotonMap, setVisualizePhotonMap,
	"if True, the content of the photon map is directly visualized (without applying material properties)\n"
	"if False, a proper render is done =)\n")
PY_CLASS_MEMBER_RW_DOC(PhotonMapper, isRayTracingDirect, setRayTracingDirect,
	"if True, the direct lighting is estimated by sampling the scene lights\n"
	"if False, the direct lighting is estimated by a direct query of the photon map\n")
PY_CLASS_MEMBER_RW_DOC(PhotonMapper, isScatteringDirect, setScatteringDirect,
	"if True and isRayTracingDirect, single scattering is performed in the direct lighting step.\n"
	"Otherwise, all scattering is estimated using the volumetric photon map.\n")

PhotonMapper::TMapTypeDictionary PhotonMapper::mapTypeDictionary_ =
	PhotonMapper::generateMapTypeDictionary();


// --- public --------------------------------------------------------------------------------------

PhotonMapper::PhotonMapper():
	DirectLighting(),
	shared_(new SharedData),
	maxNumberOfPhotons_(100000000),
	globalMapSize_(10000),
	causticsQuality_(1),
	volumetricQuality_(1),
	numFinalGatherRays_(0),
	numSecondaryGatherRays_(0),
	ratioPrecomputedIrradiance_(0.25f),
	volumetricGatherQuality_(0.25f),
	isVisualizingPhotonMap_(false),
	isRayTracingDirect_(true),
	isScatteringDirect_(true),
	photonNeighbourhood_(1)
{
	for (int i = 0; i < numMapTypes; ++i)
	{
		estimationRadius_[i] = 0;
		estimationTolerance_[i] = 0.05f;
		estimationSize_[i] = 50;
		maxActualEstimationRadius_[i] = 0;
	}
	setNumSecondaryGatherRays(1);
	updateStorageProbabilities();
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
	causticsQuality_ = std::max(quality, TNumTraits::zero);
	updateStorageProbabilities();
}



TScalar PhotonMapper::volumetricQuality() const
{
	return volumetricQuality_;
}



void PhotonMapper::setVolumetricQuality(TScalar quality)
{
	volumetricQuality_ = std::max(quality, TNumTraits::zero);
	updateStorageProbabilities();
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
	secondaryGatherBsdfSamples_.resize(numSecondaryGatherRays);
	secondaryGatherComponentSamples_.resize(numSecondaryGatherRays);
	secondaryGatherVolumetricSamples_.resize(numSecondaryGatherRays);
}



TScalar PhotonMapper::ratioPrecomputedIrradiance() const
{
	return ratioPrecomputedIrradiance_;
}



void PhotonMapper::setRatioPrecomputedIrradiance(TScalar ratio)
{
	ratioPrecomputedIrradiance_ = num::clamp(ratio, TNumTraits::zero, TNumTraits::one);
}



TScalar PhotonMapper::volumetricGatherQuality() const
{
	return volumetricGatherQuality_;
}



void PhotonMapper::setVolumetricGatherQuality(TScalar quality)
{
	volumetricGatherQuality_ = num::clamp(quality, TNumTraits::zero, TNumTraits::one);
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



bool PhotonMapper::isScatteringDirect() const
{
	return isRayTracingDirect_ && isScatteringDirect_;
}



void PhotonMapper::setScatteringDirect(bool enabled)
{
	isScatteringDirect_ = enabled;
}


// --- protected -----------------------------------------------------------------------------------

// --- private -------------------------------------------------------------------------------------

void PhotonMapper::doRequestSamples(const kernel::TSamplerPtr& sampler)
{
	DirectLighting::doRequestSamples(sampler);

	idFinalGatherSamples_ = numFinalGatherRays_ > 0 ? sampler->requestSubSequence2D(numFinalGatherRays_) : -1;
	idFinalGatherComponentSamples_ = numFinalGatherRays_ > 0 ? sampler->requestSubSequence1D(numFinalGatherRays_) : -1;
	idFinalVolumetricGatherSamples_ = numFinalGatherRays_ > 0 ? sampler->requestSubSequence1D(numFinalGatherRays_) : -1;
}



void PhotonMapper::doPreProcess(const kernel::TSamplerPtr& sampler, const TimePeriod& period, size_t numberOfThreads)
{
	DirectLighting::doPreProcess(sampler, period, numberOfThreads);

	const size_t maxSize = *std::max_element(estimationSize_, estimationSize_ + numMapTypes);
	photonNeighbourhood_.resize(maxSize + 1);

	if (lights().size() == 0)
	{
		return;
	}

	const size_t photonsShot = fillPhotonMaps(sampler, period);
	const TScalar powerScale = num::inv(static_cast<TScalar>(photonsShot));
	buildPhotonMap(mtGlobal, shared_->globalBuffer_, shared_->globalMap_, powerScale);
	buildIrradianceMap(numberOfThreads);
	buildPhotonMap(mtCaustics, shared_->causticsBuffer_, shared_->causticsMap_, powerScale);
	TPreliminaryVolumetricPhotonMap preliminaryVolumetricMap;
	buildPhotonMap(mtVolume, shared_->volumetricBuffer_, preliminaryVolumetricMap, powerScale);
	buildVolumetricPhotonMap(preliminaryVolumetricMap, numberOfThreads);
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

	return python::makeTuple(DirectLighting::doGetState(),
		maxNumberOfPhotons_, globalMapSize_, causticsQuality_,
		numFinalGatherRays_, ratioPrecomputedIrradiance_, isVisualizingPhotonMap_,
		isRayTracingDirect_, isScatteringDirect_, radius, tolerance, size);
}



void PhotonMapper::doSetState(const TPyObjectPtr& state)
{
	TPyObjectPtr directLighting;
	std::vector<TScalar> radius;
	std::vector<TScalar> tolerance;
	std::vector<size_t> size;

	python::decodeTuple(state, directLighting, maxNumberOfPhotons_, globalMapSize_, causticsQuality_,
		numFinalGatherRays_, ratioPrecomputedIrradiance_, isVisualizingPhotonMap_,
		isRayTracingDirect_, isScatteringDirect_, radius, tolerance, size);

	DirectLighting::doSetState(directLighting);

	LASS_ENFORCE(radius.size() == numMapTypes);
	LASS_ENFORCE(tolerance.size() == numMapTypes);
	LASS_ENFORCE(size.size() == numMapTypes);

	std::copy(radius.begin(), radius.end(), estimationRadius_);
	std::copy(tolerance.begin(), tolerance.end(), estimationTolerance_);
	std::copy(size.begin(), size.end(), estimationSize_);
}



const Spectral PhotonMapper::doShadeMedium(const kernel::Sample& sample, const kernel::BoundedRay& ray, Spectral& transparency) const
{
	const bool doTraceSingleScattering = isScatteringDirect();

	transparency = mediumStack().transmittance(sample, ray);

	Spectral result = mediumStack().emission(sample, ray) + estimateVolumetric(sample, ray, doTraceSingleScattering);
	if (doTraceSingleScattering)
	{
		result += traceSingleScattering(sample, ray);
	}

	return result;
}



const Spectral PhotonMapper::doShadeSurface(
		const kernel::Sample& sample, const DifferentialRay& primaryRay, const IntersectionContext& context,
		const TPoint3D& point, const TVector3D& normal, const TVector3D& omega, bool highQuality) const
{
	if (isVisualizingPhotonMap_)
	{
		return Spectral::fromXYZ(estimateIrradiance(point, normal), sample, Illuminant);
	}

	Spectral result;
	if (context.rayGeneration() == 0 || !context.object().asLight())
	{
		// actually, we block this because currently this is our way to include area lights in the camera rays only.
		// we should always to the shader emission thing, but get the light emission seperately.
		// and generation == 0 isn't a good discriminator eighter.
		result += context.shader()->emission(sample, context, omega);
	}

	const TBsdfPtr bsdf = context.bsdf();
	if (!bsdf)
	{
		return result;
	}

	result += estimateCaustics(sample, context, bsdf, point, omega);

	if (isRayTracingDirect_)
	{
		result += traceDirect(sample, context, bsdf, point, normal, omega, highQuality);
	}

	//*
	if (bsdf->hasCaps(Bsdf::capsDiffuse))
	{
		if (highQuality && hasFinalGather())
		{
			LASS_ASSERT(idFinalGatherSamples_ >= 0);
			Sample::TSubSequence2D gatherSample = sample.subSequence2D(idFinalGatherSamples_);
			Sample::TSubSequence1D componentSample = sample.subSequence1D(idFinalGatherComponentSamples_);
			Sample::TSubSequence1D volumetricGatherSample = sample.subSequence1D(idFinalVolumetricGatherSamples_);
			result += gatherIndirect(sample, context, bsdf, point + 10 * liar::tolerance * normal, omega,
				gatherSample.begin(), gatherSample.end(), componentSample.begin(), volumetricGatherSample.begin());
		}
		else
		{
			result += estimateRadiance(sample, context, bsdf, point, omega);
		}
	}
	/**/

	result += traceSpecularAndGlossy(sample, primaryRay, context, bsdf, point, normal, omega, highQuality);

	return result;
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
		buffer << std::setprecision(3) << "min=" << container.front() << ", Q1=" << container[n / 4] << ", Q2=" << container[n / 2]
			<< ", Q3=" << container[3 * n / 4] << ", max=" << container.back();
		return buffer.str();
	}
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


size_t PhotonMapper::fillPhotonMaps(const TSamplerPtr& sampler, const TimePeriod& period)
{
	TRandomPrimary random;
	TUniformPrimary uniform(random);
	TPhotonBuffer& globalBuffer = shared_->globalBuffer_;

	util::ProgressIndicator progress("filling photon map with " + util::stringCast<std::string>(globalMapSize_) + " photons");

	size_t photonsShot = 0;
	while (globalBuffer.size() < globalMapSize_)
	{
		if (photonsShot >= maxNumberOfPhotons_)
		{
			LASS_CERR << "PhotonMapper: maximum number of " << maxNumberOfPhotons_
				<< " photons emited before global photon map was sufficiently filled. "
				<< "Only " << globalBuffer.size() << " of the requested "
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

		progress(std::min(1., static_cast<double>(globalBuffer.size()) / globalMapSize_));

		++photonsShot;
	}

	LASS_COUT << "  total number of emitted photons: " << photonsShot << std::endl;
	return photonsShot;
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
	Spectral spectrum = light.sampleEmission(sample, lightSampleA, lightSampleB, ray, pdf);
	if (pdf > 0 && spectrum)
	{
		spectrum /= lightPdf * pdf;
		tracePhoton(sample, spectrum, ray, 0, uniform);
	}
}



void PhotonMapper::tracePhoton(
		const Sample& sample, const Spectral& power, const BoundedRay& ray,
		size_t generation, TUniformSecondary& uniform, bool isCaustic)
{
	if (!power)
	{
		return;
	}

	Intersection intersection;
	scene()->intersect(sample, ray, intersection);

	const TPoint3D hitPoint = ray.point(intersection.t());
	TScalar tScatter, pdf;
	const Spectral transmittance = mediumStack().sampleScatterOutOrTransmittance(sample, uniform(), bound(ray, ray.nearLimit(), intersection.t()), tScatter, pdf);
	Spectral transmittedPower = power * transmittance / pdf;
	// RR to keep power constant (if possible)
	const TScalar transmittanceProbability = std::min(transmittedPower.absAverage() / power.absAverage(), TNumTraits::one);
	if (!russianRoulette(transmittedPower, transmittanceProbability, uniform()))
	{
		return;
	}
	if (tScatter < intersection.t())
	{
		// scattering event.
		// TransmittedPower is the incident power on the article, which is precisely what we need to store.
		const TPoint3D scatterPoint = ray.point(tScatter);
		const bool isDirect = generation == 0;
		bool mayStore = !isDirect || !isScatteringDirect_ || numFinalGatherRays_ > 0;
		if (mayStore)
		{
			VolumetricPhoton photon(Photon(scatterPoint, ray.direction(), transmittedPower, sample), isDirect);
			if (russianRoulette(photon.power, storageProbability_[mtVolume], uniform()))
			{
				shared_->volumetricBuffer_.push_back(photon);
			}
		}

		// now we need to do the actual scattering ...
		if (generation >= maxRayGeneration())
		{
			return;
		}
		TVector3D dirOut;
		TScalar pdfOut;
		const Spectral reflectance = mediumStack().samplePhase(sample, TPoint2D(uniform(), uniform()), scatterPoint, ray.direction(), dirOut, pdfOut);
		if (pdfOut <= 0 || !reflectance)
		{
			return;
		}
		Spectral scatteredPower = transmittedPower * reflectance / pdfOut;
		const TScalar scatteredProbability = std::min(scatteredPower.absAverage() / transmittedPower.absAverage(), TNumTraits::one);
		if (!russianRoulette(scatteredPower, scatteredProbability, uniform()))
		{
			return;
		}
		const BoundedRay scatteredRay(scatterPoint, ray.direction());
		return tracePhoton(sample, scatteredPower, scatteredRay, generation + 1, uniform, false);
	}

	if (!intersection)
	{
		return;
	}

	IntersectionContext context(*scene(), sample, ray, intersection, generation);
	const Shader* const shader = context.shader();
	if (!shader)
	{
		// entering or leaving something ...
		if (generation >= maxRayGeneration())
		{
			return;
		}
		MediumChanger mediumChanger(mediumStack(), context.interior(), context.solidEvent());
		const BoundedRay newRay = bound(ray, intersection.t() + liar::tolerance, ray.farLimit());
		return tracePhoton(sample, transmittedPower, newRay, generation + 1, uniform, isCaustic);
	}
	shader->shadeContext(sample, context);

	if (shader->hasCaps(Bsdf::capsDiffuse))
	{
		Photon photon(hitPoint, -ray.direction(), transmittedPower, sample);
		if (isCaustic)
		{
			if (russianRoulette(photon.power, storageProbability_[mtCaustics], uniform()))
			{
				shared_->causticsBuffer_.push_back(photon);
			}
		}
		const bool mayStorePhoton = (((generation > 0) || !isRayTracingDirect_) && !isCaustic) || hasFinalGather();
		if (mayStorePhoton && russianRoulette(photon.power, storageProbability_[mtGlobal], uniform()))
		{
			shared_->globalBuffer_.push_back(photon);
			if (ratioPrecomputedIrradiance_ > 0 && uniform() <= ratioPrecomputedIrradiance_)
			{
				const TVector3D worldNormal = context.bsdfToWorld(TVector3D(0, 0, 1));
				shared_->irradianceBuffer_.push_back(Irradiance(hitPoint, worldNormal));
			}
		}
	}

	// let's generate a new photon, unless ...
	if (generation == maxRayGeneration())
	{
		return;
	}
	LASS_ASSERT(generation < maxRayGeneration());

	const TBsdfPtr bsdf = shader->bsdf(sample, context);
	if (!bsdf)
	{
		return;
	}

	const TVector3D omegaIn = context.worldToBsdf(-ray.direction());
	LASS_ASSERT(omegaIn.z > 0);

	const TPoint2D sampleBsdf(uniform(), uniform());
	const TScalar sampleComponent = uniform();
	const SampleBsdfOut out = bsdf->sample(omegaIn, sampleBsdf, sampleComponent, Bsdf::capsAll);
	if (!out)
	{
		return;
	}

	const TScalar cos_theta = out.omegaOut.z;
	Spectral newPower = transmittedPower * out.value * (num::abs(cos_theta) / out.pdf);
	const TScalar attenuation = newPower.average() / transmittedPower.average();
	//LASS_ASSERT(attenuation < 1.1);
	const TScalar scatterProbability = std::min(TNumTraits::one, attenuation);
	if (!russianRoulette(newPower, scatterProbability, uniform()))
	{
		return;
	}
	const BoundedRay newRay(hitPoint, context.bsdfToWorld(out.omegaOut));
	const bool isSpecular = (out.usedCaps & (Bsdf::capsSpecular | Bsdf::capsGlossy)) > 0;
	const bool newIsCaustic = isSpecular && (isCaustic || generation == 0);
	MediumChanger mediumChanger(mediumStack(), context.interior(),
		out.omegaOut.z < 0 ? context.solidEvent() : seNoEvent);
	return tracePhoton(sample, newPower, newRay, generation + 1, uniform, newIsCaustic);
}



template <typename PhotonBuffer, typename PhotonMap>
void PhotonMapper::buildPhotonMap(MapType type, PhotonBuffer& buffer, PhotonMap& map, TScalar powerScale)
{
	LASS_COUT << mapTypeDictionary_.key(type) << " photon map:" << std::endl;
	LASS_COUT << "  number of photons: " << buffer.size() << std::endl;

	map.reset(buffer.begin(), buffer.end());

	if (!buffer.empty())
	{
		std::vector<TScalar> powers;
		powers.reserve(buffer.size());
		typename PhotonBuffer::iterator end = buffer.end();
		for (typename PhotonBuffer::iterator i = buffer.begin(); i != end; ++i)
		{
			i->power *= powerScale;
			powers.push_back(i->power.absTotal());
		}
		LASS_COUT << "  photon powers: " << temp::statistics(powers) << std::endl;

		if (estimationRadius_[type] == 0)
		{
			if (estimationTolerance_[type] <= 0)
			{
				estimationTolerance_[type] = 0.05f;
			}
			const TScalar medianPower = powers[powers.size() / 2];
			const TScalar estimationArea = estimationSize_[type] * medianPower / estimationTolerance_[type];
			if (type == mtVolume)
			{
				estimationRadius_[type] = num::pow(estimationArea * 3.f / 16.f, TNumTraits::one / 3) / TNumTraits::pi;
			}
			else
			{
				estimationRadius_[type] = num::sqrt(estimationArea) / TNumTraits::pi;
			}
			LASS_COUT << "  automatic estimation radius: " << estimationRadius_[type] << std::endl;
		}
	}
}




namespace experimental
{

class ConcurrentProgressIndicator: util::NonCopyable
{
public:
	ConcurrentProgressIndicator(size_t max, const std::string& description, int consoleWidth = 80):
		progress_(description, consoleWidth),
		max_(max),
		current_(0),
		isDone_(false)
	{
		loopThread_.reset(util::threadMemFun(this, &ConcurrentProgressIndicator::progressLoop, util::threadJoinable));
		loopThread_->run();
	}
	~ConcurrentProgressIndicator()
	{
		isDone_ = true;
		loopThread_->join();
		progress_(1.);
	}
	void operator++()
	{
		util::atomicIncrement(current_);
	}
private:
	void progressLoop()
	{
		size_t oldCurrent = 0;
		while (!isDone_)
		{
			size_t newCurrent = current_;
			if (newCurrent > oldCurrent)
			{
				progress_(newCurrent / static_cast<TScalar>(max_));
				oldCurrent = newCurrent;
			}
			util::Thread::sleep(100);
		}
	}
	util::ScopedPtr<util::Thread> loopThread_;
	util::ProgressIndicator progress_;
	size_t max_;
	volatile size_t current_;
	volatile bool isDone_;
};



class TaskRange
{
public:
	TaskRange(): begin_(0), end_(0), progress_(0) {}
	TaskRange(size_t begin, size_t end, ConcurrentProgressIndicator* progress): begin_(begin), end_(end), progress_(progress) {}
	TaskRange& operator++()
	{
		++begin_;
		++*progress_;
		return *this;
	}
	TaskRange operator++(int)
	{
		TaskRange temp(*this);
		++*this;
		return temp;
	}
	size_t operator*() const { return begin_; }
	operator num::SafeBool() const { return num::safeBool(begin_ < end_); }
private:
	size_t begin_;
	size_t end_;
	ConcurrentProgressIndicator* progress_;
};



template <typename WorkerType>
void runWorkers(WorkerType& worker, size_t size, size_t numberOfThreads, const std::string& description)
{
	typedef util::ThreadPool<TaskRange, WorkerType, util::Spinning, util::SelfParticipating> TThreadPool;

	ConcurrentProgressIndicator progress(size, description);

	if (numberOfThreads == 1)
	{
		worker(TaskRange(0, size, &progress));
		return;
	}

	TThreadPool pool(numberOfThreads, TThreadPool::unlimitedNumberOfTasks, worker);
	const size_t taskSize = std::max<size_t>(static_cast<size_t>(num::sqrt(static_cast<TScalar>(size))), 1);
	for (size_t begin = 0; begin < size; begin += taskSize)
	{
		const size_t end = std::min(begin + taskSize, size);
		pool.addTask(TaskRange(begin, end, &progress));
	}
}

}



class IrradianceWorker
{
public:
	typedef std::vector<TScalar> TRadii;
	typedef std::vector<size_t> TCounts;
	typedef std::vector<TScalar> TIrradiances;

	IrradianceWorker(PhotonMapper& photonMapper, TRadii& radii, TCounts& counts, TIrradiances& irradiances):
		neighbourhood_(photonMapper.estimationSize_[PhotonMapper::mtGlobal] + 1),
		photonMapper_(photonMapper),
		buffer_(photonMapper.shared_->irradianceBuffer_),
		radii_(radii),
		counts_(counts),
		irradiances_(irradiances)
	{
	}
	void operator()(experimental::TaskRange task)
	{
		while (task)
		{
			const size_t i = *task++;
			PhotonMapper::Irradiance& ir = buffer_[i];
			ir.irradiance = photonMapper_.estimateIrradianceImpl(neighbourhood_, ir.position, ir.normal, ir.squaredEstimationRadius, counts_[i]);
			radii_[i] = num::sqrt(ir.squaredEstimationRadius);
			irradiances_[i] = ir.irradiance.absTotal();
		}
	}
private:
	PhotonMapper::TPhotonNeighbourhood neighbourhood_;
	PhotonMapper& photonMapper_;
	PhotonMapper::TIrradianceBuffer& buffer_;
	TRadii& radii_;
	TCounts& counts_;
	TIrradiances& irradiances_;
};



void PhotonMapper::buildIrradianceMap(size_t numberOfThreads)
{
	shared_->irradianceMap_.reset();
	if (shared_->irradianceBuffer_.empty() || ratioPrecomputedIrradiance_ == 0 || numFinalGatherRays_ == 0 || !isRayTracingDirect_)
	{
		return;
	}

	const size_t size = shared_->irradianceBuffer_.size();
	IrradianceWorker::TRadii radii(size);
	IrradianceWorker::TCounts counts(size);
	IrradianceWorker::TIrradiances irradiances(size);
	IrradianceWorker worker(*this, radii, counts, irradiances);
	experimental::runWorkers(worker, size, numberOfThreads, "  precomputing irradiances");

	LASS_COUT << "  irradiance: " << temp::statistics(irradiances) << std::endl;
	LASS_COUT << "  eff. radii: " << temp::statistics(radii) << std::endl;
	LASS_COUT << "  eff. counts: " << temp::statistics(counts) << std::endl;

	shared_->irradianceMap_.reset(shared_->irradianceBuffer_.begin(), shared_->irradianceBuffer_.end());
}



class VolumetricWorker
{
public:
	typedef PhotonMapper::TVolumetricPhotonBuffer TVolumetricPhotonBuffer;
	typedef PhotonMapper::TPreliminaryVolumetricPhotonMap TPreliminaryVolumetricPhotonMap;
	typedef TPreliminaryVolumetricPhotonMap::TNeighbourhood TNeighbourhood;
	typedef std::vector<TScalar> TRadii;

	VolumetricWorker(TVolumetricPhotonBuffer& buffer, TRadii& radii, const TPreliminaryVolumetricPhotonMap& map, TScalar rMax, size_t nMax):
		buffer_(buffer),
		radii_(radii),
		map_(map),
		rMax_(rMax),
		mMax_(std::max<size_t>(static_cast<size_t>(num::ceil(num::sqrt(static_cast<TScalar>(nMax)))), 5))
	{
		neighbourhood_.resize(mMax_ + 1);
		mnScale_ = num::pow(static_cast<TScalar>(nMax) / static_cast<TScalar>(mMax_), TNumTraits::one / 3);
	}
	void operator()(experimental::TaskRange task)
	{
		while (task)
		{
			const size_t i = *task++;
			PhotonMapper::VolumetricPhoton& photon = buffer_[i];
			//*
			const TNeighbourhood::const_iterator last = map_.rangeSearch(photon.position, rMax_, mMax_, neighbourhood_.begin());
			size_t m = static_cast<size_t>(last - neighbourhood_.begin());
			LASS_ASSERT(m > 0);
			const TScalar dist = num::sqrt(neighbourhood_.front().squaredDistance());
			photon.radius = std::min(rMax_, mnScale_ * (m < mMax_ ? rMax_ : dist));
			/*/
			photon.radius = rMax;
			/**/
			radii_[i] = photon.radius;
		}
	}
private:
	TNeighbourhood neighbourhood_;
	TVolumetricPhotonBuffer& buffer_;
	TRadii& radii_;
	const TPreliminaryVolumetricPhotonMap& map_;
	TScalar rMax_;
	size_t mMax_;
	TScalar mnScale_;
};



/** Build a tree of spherical photons for the volumetric map.
 *  An estimation of the radius is made by searching for less photons than required by the settings (see article).
 *  @par ref: W. Jarosz, M. Zwicker, H.W.n Jensen. The Beam Radiance Estimate for Volumetric Photon Mapping (2008)
 */
void PhotonMapper::buildVolumetricPhotonMap(const TPreliminaryVolumetricPhotonMap& preliminaryVolumetricMap, size_t numberOfThreads)
{
	const size_t size = shared_->volumetricBuffer_.size();
	if (!size)
	{
		return;
	}
	VolumetricWorker::TRadii radii(size);
	VolumetricWorker worker(shared_->volumetricBuffer_, radii, preliminaryVolumetricMap, estimationRadius_[mtVolume], estimationSize_[mtVolume]);
	experimental::runWorkers(worker, size, numberOfThreads, "  precomputing radii");
	LASS_COUT << "  eff. radii: " << temp::statistics(radii) << std::endl;

	shared_->volumetricMap_.reset(shared_->volumetricBuffer_.begin(), shared_->volumetricBuffer_.end());
}



const Spectral PhotonMapper::gatherIndirect(
		const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& target, const TVector3D& omegaIn,
		const TPoint2D* firstSample, const TPoint2D* lastSample, const TScalar* firstComponentSample, const TScalar* firstVolumetricSample,
		size_t gatherStage) const
{
	LASS_ASSERT(gatherStage < numGatherStages_);

	if (!bsdf)
	{
		return Spectral();
	}

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
		const TPhotonNeighbourhood::const_iterator last = shared_->globalMap_.rangeSearch(
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

	Spectral result;
	const ptrdiff_t n = lastSample - firstSample;
	for (; firstSample != lastSample; ++firstSample, ++firstComponentSample, ++firstVolumetricSample)
	{
		SampleBsdfOut out;
		if (doImportanceGathering && gatherStage == 0)
		{
			prim::Point2D<size_t> i2;
			out.pdf = 1;
			const TPoint2D s = gatherDistribution_(*firstSample, out.pdf, i2);
			TScalar pdfHalfSphere;
			out.omegaOut = num::uniformHemisphere(s, pdfHalfSphere).position();
			out.pdf *= pdfHalfSphere;
			out.value = bsdf->evaluate(omegaIn, out.omegaOut, Bsdf::capsReflection | Bsdf::capsDiffuse).value;
		}
		else
		{
			out = bsdf->sample(omegaIn, *firstSample, *firstComponentSample, Bsdf::capsReflection | Bsdf::capsDiffuse);
		}
		if (!out)
		{
			continue;
		}
		const TVector3D direction = context.bsdfToWorld(out.omegaOut);
		const BoundedRay ray(target, direction, liar::tolerance);
		const bool gatherVolumetric = (volumetricGatherQuality_ > 0) && (*firstVolumetricSample <= volumetricGatherQuality_);
		const Spectral radiance = traceGatherRay(sample, ray, gatherVolumetric, gatherStage, context.rayGeneration() + 1);

		result += radiance * out.value * (num::abs(out.omegaOut.z) / (n * out.pdf));
	}
	return result;
}



const Spectral PhotonMapper::traceGatherRay(const Sample& sample, const BoundedRay& ray, bool gatherVolumetric, size_t gatherStage, size_t rayGeneration) const
{
	Intersection intersection;
	scene()->intersect(sample, ray, intersection);

	const Spectral inScatter = gatherVolumetric ? estimateVolumetric(sample, bound(ray, ray.nearLimit(), intersection.t())) / volumetricGatherQuality_ : Spectral();
	if (!intersection)
	{
		return inScatter;
	}
	const Spectral transmittance = mediumStack().transmittance(sample, ray, intersection.t());

	IntersectionContext context(*scene(), sample, ray, intersection, rayGeneration);
	const Shader* const shader = context.shader();
	Spectral radiance;
	if (shader)
	{
		shader->shadeContext(sample, context);
		const TBsdfPtr bsdf = shader->bsdf(sample, context);
		const TPoint3D point = ray.point(intersection.t());
		const TVector3D omega = context.worldToBsdf(-ray.direction());
		TScalar sqrRadius = TNumTraits::infinity;
		radiance = estimateRadiance(sample, context, bsdf, point, omega, sqrRadius);
		if (num::sqr(intersection.t()) < sqrRadius && gatherStage == 0 && numSecondaryGatherRays_ > 0)
		{
			radiance = gatherSecondary(sample, context, bsdf, point, omega);
		}
	}
	else
	{
		// leaving or entering something
		MediumChanger mediumChanger(mediumStack(), context.interior(), context.solidEvent());
		const BoundedRay continuedRay = bound(ray, intersection.t() + liar::tolerance);
		radiance = traceGatherRay(sample, continuedRay, gatherVolumetric, gatherStage, rayGeneration + 1);
	}


	return inScatter + transmittance * radiance;
}



const Spectral PhotonMapper::gatherSecondary(
		const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& point, const TVector3D& omega) const
{
	const size_t n = numSecondaryGatherRays_;
	if (n == 0)
	{
		return Spectral();
	}

	Spectral result;

	TPoint2D* bsdfSamples = &secondaryGatherBsdfSamples_[0];
	TScalar* componentSamples = &secondaryGatherComponentSamples_[0];
	TScalar* volumetricSamples = &secondaryGatherVolumetricSamples_[0];
	latinHypercube2D(bsdfSamples, bsdfSamples + n, secondarySampler());
	stratifier1D(componentSamples, componentSamples + n, secondarySampler());
	stratifier1D(volumetricSamples, volumetricSamples + n, secondarySampler());
	result += gatherIndirect(sample, context, bsdf, point, omega, bsdfSamples, bsdfSamples + n, componentSamples, volumetricSamples, 1);

	const TVector3D normal = context.bsdfToWorld(TVector3D(0, 0, 1));
	result += traceDirect(sample, context, bsdf, point, normal, omega, false);

	return result;
}


const XYZ PhotonMapper::estimateIrradiance(const TPoint3D& point, const TVector3D& normal, TScalar& sqrEstimationRadius, size_t& count) const
{
	if (!shared_->irradianceMap_.isEmpty())
	{
		TIrradianceMap::Neighbour nearest = shared_->irradianceMap_.nearestNeighbour(point, estimationRadius_[mtGlobal]);
		if (nearest.object() == shared_->irradianceMap_.end())
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

	return estimateIrradianceImpl(photonNeighbourhood_, point, normal, sqrEstimationRadius, count);
}



const XYZ PhotonMapper::estimateIrradianceImpl(TPhotonNeighbourhood& neighbourhood, const TPoint3D& point, const TVector3D& normal, TScalar& sqrEstimationRadius, size_t& count) const
{
	LASS_ASSERT(neighbourhood.size() > estimationSize_[mtGlobal]);
	const TPhotonNeighbourhood::const_iterator last = shared_->globalMap_.rangeSearch(
		point, estimationRadius_[mtGlobal], estimationSize_[mtGlobal], neighbourhood.begin());

	count = static_cast<size_t>(last - neighbourhood.begin());
	if (count == 0)
	{
		sqrEstimationRadius = 0;
		return XYZ();
	}

	XYZ result;
	for (TPhotonNeighbourhood::const_iterator i = neighbourhood.begin(); i != last; ++i)
	{
		if (dot(i->object()->omegaIn, normal) > 0)
		{
			result += i->object()->power;
		}
	}

	sqrEstimationRadius = neighbourhood.front().squaredDistance();
	return sqrEstimationRadius > 0 ? result / (TNumTraits::pi * sqrEstimationRadius) : XYZ();
}



const Spectral PhotonMapper::estimateRadiance(
		const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& point, const TVector3D& omegaOut, TScalar& sqrEstimationRadius) const
{
	const Shader* const shader = context.shader();
	if (!bsdf || !shader || !shader->hasCaps(Bsdf::capsDiffuse))
	{
		return Spectral();
	}

	//*
	/**/

	if (!shared_->irradianceMap_.isEmpty())
	{
		TIrradianceMap::Neighbour nearest = shared_->irradianceMap_.nearestNeighbour(point, estimationRadius_[mtGlobal]);
		if (nearest.object() == shared_->irradianceMap_.end())
		{
			sqrEstimationRadius = estimationRadius_[mtGlobal];
			return Spectral();
		}
		//if (dot(normal, nearest->normal) > 0.9)
		{
			sqrEstimationRadius = nearest->squaredEstimationRadius;
			const BsdfOut out = bsdf->evaluate(omegaOut, context.worldToBsdf(nearest->normal), Bsdf::capsAll);
			return out ? out.value * nearest->spectralIrradiance(sample) : Spectral();
		}
	}

	LASS_ASSERT(photonNeighbourhood_.size() > estimationSize_[mtGlobal]);
	const TPhotonNeighbourhood::const_iterator last = shared_->globalMap_.rangeSearch(
		point, estimationRadius_[mtGlobal], estimationSize_[mtGlobal], photonNeighbourhood_.begin());

	const TPhotonNeighbourhood::difference_type n = last - photonNeighbourhood_.begin();
	if (n < 2)
	{
		sqrEstimationRadius = estimationRadius_[mtGlobal];
		return Spectral();
	}

	Spectral result;
	for (TPhotonNeighbourhood::const_iterator i = photonNeighbourhood_.begin(); i != last; ++i)
	{
		const TVector3D omegaPhoton = context.worldToBsdf(i->object()->omegaIn);
		const BsdfOut out = bsdf->evaluate(omegaOut, omegaPhoton, Bsdf::capsAll & ~Bsdf::capsSpecular & ~Bsdf::capsGlossy);
		if (out.pdf > 0 && out.value)
		{
			result += out.value * i->object()->spectralPower(sample);
		}
	}

	sqrEstimationRadius = photonNeighbourhood_.front().squaredDistance();
	return result / (TNumTraits::pi * sqrEstimationRadius);
}



const Spectral PhotonMapper::estimateCaustics(
		const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& point, const TVector3D& omegaIn) const
{
	const Shader* const shader = context.shader();
	if (!shader || !shader->hasCaps(Bsdf::capsDiffuse))
	{
		return Spectral();
	}

	LASS_ASSERT(photonNeighbourhood_.size() > estimationSize_[mtCaustics]);
	const TPhotonNeighbourhood::const_iterator last = shared_->causticsMap_.rangeSearch(
		point, estimationRadius_[mtCaustics], estimationSize_[mtCaustics], photonNeighbourhood_.begin());
	const TPhotonNeighbourhood::difference_type n = last - photonNeighbourhood_.begin();
	LASS_ASSERT(n >= 0);
	if (n < 2)
	{
		return Spectral();
	}

	const TScalar sqrSize = photonNeighbourhood_[0].squaredDistance();
	const TScalar alpha = 0.918f;
	const TScalar beta = 1.953f;
	const TScalar b1 = -beta / (2 * sqrSize);
	const TScalar b2 = num::inv(1 - num::exp(-beta));

	Spectral result;
	for (int i = 0; i < n; ++i)
	{
		const TVector3D omegaPhoton = context.worldToBsdf(photonNeighbourhood_[i].object()->omegaIn);
		const BsdfOut out = bsdf->evaluate(omegaIn, omegaPhoton, Bsdf::capsAllDiffuse);
		if (out)
		{
			const TScalar sqrR = photonNeighbourhood_[i].squaredDistance();
			const TScalar w = alpha * (1 - b2 * (1 - num::exp(b1 * sqrR)));
			result += w * out.value * photonNeighbourhood_[i].object()->spectralPower(sample);
		}
	}

	return result / (TNumTraits::pi * sqrSize);
}

namespace temp
{
	/** 2D Epanechnikov density kernel.
	 *  @param h [in] kernel bandwidth
	 *  @par B. W. Silverman, Density estimation for statistics and data analysis (1986), page 76
	 */
 	TScalar kernelEpanechnikov2D(const TPoint3D& p, const TPoint3D& q, TScalar h)
	{
		const TScalar d2 = prim::squaredDistance(p, q);
		const TScalar h2 = num::sqr(h);
		return std::max(1 - (d2 / h2), TNumTraits::zero) * 2 / (TNumTraits::pi * h2);
	}

	/** 2D Silverman density kernel.
	 *  @param h [in] kernel bandwidth
	 *  @par B. W. Silverman, Density estimation for statistics and data analysis (1986), page 76
	 */
 	TScalar kernelSilverman2D(const TPoint3D& p, const TPoint3D& q, TScalar h)
	{
		const TScalar d2 = prim::squaredDistance(p, q);
		const TScalar h2 = num::sqr(h);
		return num::sqr(std::max(1 - (d2 / h2), TNumTraits::zero)) * 3 / (TNumTraits::pi * h2);
	}
}



const Spectral PhotonMapper::estimateVolumetric(const Sample& sample, const kernel::BoundedRay& ray, bool dropDirectPhotons) const
{
	typedef Sample::TSubSequence1D::difference_type difference_type;

	const Medium* medium = mediumStack().medium();
	if (shared_->volumetricMap_.isEmpty() || !medium || ray.isEmpty())
	{
		return Spectral();
	}

	Spectral result;

	const TRay3D& unboundedRay = ray.unboundedRay();
	const TScalar tNear = ray.nearLimit();
	const TScalar tFar = ray.farLimit();
	TVolumetricNeighbourhood::const_iterator last = shared_->volumetricMap_.find(
		unboundedRay, tNear, tFar, stde::overwrite_inserter(volumetricNeighbourhood_)).end();
	for (TVolumetricNeighbourhood::const_iterator i = volumetricNeighbourhood_.begin(); i != last; ++i)
	{
		const VolumetricPhoton& photon = **i;
		if (dropDirectPhotons && photon.isDirect)
		{
			continue;
		}
		const TScalar t = num::clamp(unboundedRay.t(photon.position), tNear, tFar);
		const TPoint3D pos = unboundedRay.point(t);
		const TScalar k = temp::kernelEpanechnikov2D(pos, photon.position, photon.radius);
		if (k <= 0)
		{
			continue;
		}
		const Spectral trans = medium->scatterOut(sample, bound(ray, tNear, t));
		const Spectral phase = medium->phase(sample, pos, ray.direction(), -photon.omegaIn);
		result += (k) * trans * phase * photon.spectralPower(sample);
	}

	return result;
}



void PhotonMapper::updateActualEstimationRadius(MapType mapType, TScalar radius) const
{
	maxActualEstimationRadius_[mapType] = std::max(maxActualEstimationRadius_[mapType], radius);
}



void PhotonMapper::updateStorageProbabilities()
{
	const TScalar maxQuality = std::max(std::max(causticsQuality_, volumetricQuality_), TNumTraits::one);
	storageProbability_[mtGlobal] = num::inv(maxQuality);
	storageProbability_[mtCaustics] = causticsQuality_ / maxQuality;
	storageProbability_[mtVolume] = volumetricQuality_ / maxQuality;
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
