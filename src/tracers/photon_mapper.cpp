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
PY_CLASS_MEMBER_RW(PhotonMapper, "maxNumberOfPhotons", maxNumberOfPhotons, setMaxNumberOfPhotons)
PY_CLASS_METHOD(PhotonMapper, requestedMapSize)
PY_CLASS_METHOD(PhotonMapper, setRequestedMapSize)
PY_CLASS_METHOD(PhotonMapper, estimationRadius)
PY_CLASS_METHOD(PhotonMapper, setEstimationRadius)
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
	maxNumberOfPhotons_(1000000),
	numFinalGatherRays_(0),
	ratioPrecomputedIrradiance_(0.25f),
	isVisualizingPhotonMap_(false),
	isRayTracingDirect_(true),
	photonNeighbourhood_(1)
{
	for (int i = 0; i < numMapTypes; ++i)
	{
		estimationRadius_[i] = 1;
		requestedMapSize_[i] = 100000;
		estimationSize_[i] = 100;
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



const unsigned PhotonMapper::requestedMapSize(const std::string& mapType) const
{
	return requestedMapSize_[mapTypeDictionary_[mapType]];
}



void PhotonMapper::setRequestedMapSize(const std::string& mapType, unsigned mapSize)
{
	requestedMapSize_[mapTypeDictionary_[mapType]] = mapSize;
}



const TScalar PhotonMapper::estimationRadius(const std::string& mapType) const
{
	return estimationRadius_[mapTypeDictionary_[mapType]];
}



void PhotonMapper::setEstimationRadius(const std::string& mapType, TScalar radius)
{
	estimationRadius_[mapTypeDictionary_[mapType]] = radius;
}



const unsigned PhotonMapper::estimationSize(const std::string& mapType) const
{
	return estimationSize_[mapTypeDictionary_[mapType]];
}



void PhotonMapper::setEstimationSize(const std::string& mapType, unsigned size)
{
	estimationSize_[mapTypeDictionary_[mapType]] = size;
	
	size_t maxSize = 1;
	for (int i = 0; i < numMapTypes; ++i)
	{
		maxSize = std::max(maxSize, estimationSize_[i]);
	}
	photonNeighbourhood_.resize(maxSize + 1);
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
	TLightCdf lightCdf;
	for (TLightContexts::const_iterator i = lights().begin(); i != lights().end(); ++i)
	{
		lightCdf.push_back(i->totalPower().average());
	}
	std::partial_sum(lightCdf.begin(), lightCdf.end(), lightCdf.begin());
	std::transform(lightCdf.begin(), lightCdf.end(), lightCdf.begin(), 
		std::bind2nd(std::divides<TScalar>(), lightCdf.back()));

	buildPhotonMap(mtGlobal, lightCdf, sampler, period);
	buildIrradianceMap();
}

namespace temp
{
	typedef util::AllocatorSingleton<
		util::AllocatorPerThread<
			util::AllocatorVariableHybrid<
				util::AllocatorFreeList<>,
				128
			>
		>
	>
	CustomAllocator;

	template <typename T>
	class custom_stl_allocator: public stde::lass_allocator<T, CustomAllocator> {};
}

const Spectrum PhotonMapper::doCastRay(
		const kernel::Sample& sample, const kernel::DifferentialRay& primaryRay,
		unsigned generation) const
{
	Intersection intersection;
	scene()->intersect(sample, primaryRay, intersection);
	if (!intersection)
	{
		return Spectrum();
	}
	const TPoint3D target = primaryRay.point(intersection.t());

	IntersectionContext context(this);
	scene()->localContext(sample, primaryRay, intersection, context);
	context.flipTo(-primaryRay.direction());

	const Shader* const shader = context.shader();
	if (!shader)
	{
		// leaving or entering something
		const DifferentialRay continuedRay = bound(primaryRay, intersection.t() + tolerance);
		return this->castRay(sample, continuedRay);
	}

	const TVector3D targetNormal = context.shaderToWorld(TVector3D(0, 0, 1));
	const TVector3D omegaOut = context.worldToShader(-primaryRay.direction());
	LASS_ASSERT(omegaOut.z >= 0);

	if (isVisualizingPhotonMap_)
	{
		return estimateIrradiance(target, targetNormal);
	}

	Spectrum result = shader->emission(sample, context, omegaOut);

	if (isRayTracingDirect_)
	{
		result += traceDirect(sample, context, target, targetNormal, omegaOut);
	}

	if (shader->hasCaps(Shader::capsDiffuse))
	{
		if (isRayTracingDirect_ && numFinalGatherRays_ > 0)
		{
			LASS_ASSERT(idFinalGatherSamples_ >= 0);
			Sample::TSubSequence2D gatherSample = sample.subSequence2D(idFinalGatherSamples_);
			result += gatherIndirect(
				sample, context, target, omegaOut, gatherSample.begin(), gatherSample.end());
		}
		else
		{
			result += estimateRadiance(sample, context, target, omegaOut);
		}
	}

	if (shader->hasCaps(Shader::capsSpecular) || shader->hasCaps(Shader::capsGlossy))
	{
		if (shader->hasCaps(Shader::capsReflection) && shader->idReflectionSamples() != -1)
		{
			Sample::TSubSequence2D bsdfSample = sample.subSequence2D(shader->idReflectionSamples());
			const size_t n = generation == 0 ? bsdfSample.size() : 1;
			std::vector<TVector3D, temp::custom_stl_allocator<TVector3D> > omegaIn(n);
			std::vector<Spectrum, temp::custom_stl_allocator<Spectrum> > bsdf(n);
			std::vector<TScalar, temp::custom_stl_allocator<TScalar> > pdf(n);
			shader->sampleBsdf(sample, context, omegaOut, bsdfSample.begin(), bsdfSample.begin() + n, &omegaIn[0], &bsdf[0], &pdf[0],
				Shader::capsReflection | Shader::capsSpecular | Shader::capsGlossy);

			const TPoint3D beginCentral = target;
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
				if (pdf[i] > 0 && bsdf[i])
				{
					LASS_ASSERT(omegaIn[i].z > 0);
					const TVector3D directionCentral = context.shaderToWorld(omegaIn[i]);
					const TVector3D test = context.shaderToWorld(omegaOut);
					LASS_ASSERT(dot(normal, directionCentral) > 0);
					LASS_ASSERT(dot(normal, incident) < 0);
					const TVector3D directionI = directionCentral + dReflected_dI;
					const TVector3D directionJ = directionCentral + dReflected_dJ;

					const DifferentialRay reflectedRay(
						BoundedRay(beginCentral, directionCentral, tolerance),
						TRay3D(beginI, directionI),
						TRay3D(beginJ, directionJ));
					result += bsdf[i] * castRay(sample, reflectedRay) * (num::abs(omegaIn[i].z) / (n * pdf[i]));
				}
			}
		}
	}

	return result;
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
	std::vector<unsigned> reqMapSize(requestedMapSize_, requestedMapSize_ + numMapTypes);
	return python::makeTuple(reqMapSize, maxNumberOfPhotons_);
}



void PhotonMapper::doSetState(const TPyObjectPtr& state)
{
	std::vector<unsigned> reqMapSize;
	unsigned maxNumPhotons;

	python::decodeTuple(state, reqMapSize, maxNumPhotons);	
	LASS_ENFORCE(reqMapSize.size() == numMapTypes);
	
	std::copy(reqMapSize.begin(), reqMapSize.end(), requestedMapSize_);
	maxNumberOfPhotons_ = maxNumPhotons;
}



void PhotonMapper::buildPhotonMap(MapType type, const TLightCdf& lightCdf, 
		const TSamplerPtr& sampler, const TimePeriod& period)
{
	LASS_ASSERT(mapTypeDictionary_.isValue(type));
	if (lightCdf.empty())
	{
		return;
	}

	TRandomPrimary random;
	TUniformPrimary uniform(random);

	util::ProgressIndicator progress("filling " + mapTypeDictionary_.key(type) + " map with " +
		util::stringCast<std::string>(requestedMapSize_[type]) + " photons");

	size_t photonsShot = 0;
	while (photonBuffer_[type].size() < requestedMapSize_[type])
	{
		if (photonsShot >= maxNumberOfPhotons_)
		{
			LASS_CERR << "PhotonMapper: maximum number of " 
				<< maxNumberOfPhotons_	<< " photons reached before " 
				<< mapTypeDictionary_.key(type) << " photon map was sufficiently filled with " 
				<< requestedMapSize_[type] << " photons.";
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
            emitPhoton(type, lights()[light], lightPdf, sample, secondarySeed);
		}

		progress(std::min(1., 
			static_cast<double>(photonBuffer_[type].size()) / requestedMapSize_[type]));

		++photonsShot;
	}

    const TScalar invPhotonsShot = num::inv(static_cast<TScalar>(photonsShot));
	for (TPhotonBuffer::iterator i = photonBuffer_[type].begin(); i != photonBuffer_[type].end(); ++i)
	{
		i->power *= invPhotonsShot;
	}

#pragma LASS_NOTE("diagnostic code [Bramz]")
#if 1//ndef NDEBUG
	if (photonBuffer_[type].size() > 0)
	{
		std::vector<TScalar> powers;
		for (TPhotonBuffer::iterator i = photonBuffer_[type].begin(); i != photonBuffer_[type].end(); ++i)
		{
			powers.push_back(i->power.average());
		}
		std::sort(powers.begin(), powers.end());
		const TScalar min = powers.front();
		const TScalar max = powers.back();
		const TScalar median = powers[powers.size() / 2];
		LASS_COUT << "min, median, max: " << min << ", " << median << ", " << max << std::endl;
	}
#endif

	TScalar maxPower = 0;
	for (TPhotonBuffer::iterator i = photonBuffer_[type].begin(); i != photonBuffer_[type].end(); ++i)
	{
		maxPower = std::max(maxPower, i->power.average());
	}
	const TScalar threshold = 0.05f;
	estimationRadius_[type] = num::sqrt(estimationSize_[type] * maxPower / threshold) / TNumTraits::pi;
	std::cout << "estimation radius: " << estimationRadius_[type] << std::endl;

	photonMap_[type].reset(photonBuffer_[type].begin(), photonBuffer_[type].end());
}



void PhotonMapper::emitPhoton(MapType iType, const LightContext& light, TScalar lightPdf,
		const Sample& sample, TRandomSecondary::TValue secondarySeed)
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
		unsigned generation, TUniformSecondary& uniform)
{
	Intersection intersection;
	scene()->intersect(sample, ray, intersection);
	if (!intersection)
	{
		return;
	}

	const TPoint3D hitPoint = ray.point(intersection.t());

	IntersectionContext context(this);
	scene()->localContext(sample, ray, intersection, context);
	context.flipTo(-ray.direction());
	const Shader* const shader = context.shader();
	if (!shader)
	{
		// entering or leaving something ...
		if (generation < maxRayGeneration())
		{
			const BoundedRay newRay(
				ray.unboundedRay(), intersection.t() + tolerance, ray.farLimit());
			tracePhoton(sample, power, newRay, generation + 1, uniform);
		}
		return;
	}

	if (shader->hasCaps(Shader::capsDiffuse))
	{
		Photon photon(hitPoint, -ray.direction(), power);
		if (numFinalGatherRays_ > 0 || generation > 0 || !isRayTracingDirect_)
		{
			photonBuffer_[mtGlobal].push_back(photon);
			if (uniform() < ratioPrecomputedIrradiance_)
			{
				const TVector3D worldNormal = context.shaderToWorld(TVector3D(0, 0, 1));
				irradianceBuffer_.push_back(Irradiance(hitPoint, worldNormal));
			}
		}
	}

	// let's generate a new photon, unless ...
	if (generation == maxRayGeneration())
	{
		return;
	}
	LASS_ASSERT(generation < maxRayGeneration());

	const TVector3D omegaOut = context.worldToShader(-ray.direction());

	const TPoint2D bsdfSample(uniform(), uniform());
	TVector3D omegaIn;
	Spectrum spectrum;
	TScalar pdf;
	context.shader()->sampleBsdf(sample, context, omegaOut, &bsdfSample, &bsdfSample + 1,
		&omegaIn, &spectrum, &pdf, Shader::capsAll);
	if (pdf == 0 || !spectrum)
	{
		return;
	}

	const TScalar cos_thetaOut = omegaIn.z;
	Spectrum newPower = power * spectrum * (abs(cos_thetaOut) / pdf);
	const TScalar attenuation = newPower.average() / power.average();
	LASS_ASSERT(attenuation < 1.01);
	const TScalar scatterProbability = std::min(TNumTraits::one, attenuation);
	if (uniform() < scatterProbability)
	{
		newPower /= scatterProbability;
		BoundedRay newRay(hitPoint, context.shaderToWorld(omegaIn), tolerance);
		tracePhoton(sample, newPower, newRay, generation + 1, uniform);
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
		const TVector3D& omegaOut) const
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
				sample, *lightSample, target, targetNormal, shadowRay, lightPdf);
			if (lightPdf > 0 && radiance)
			{
				const TVector3D omegaIn = context.worldToShader(shadowRay.direction());
				Spectrum bsdf;
				TScalar bsdfPdf;
				shader->bsdf(sample, context, omegaOut, 
					&omegaIn, &omegaIn + 1, &bsdf, &bsdfPdf, caps);
				if (bsdf && !scene()->isIntersecting(sample, shadowRay))
				{
					const TScalar weight = isSingularLight ? 
						TNumTraits::one : temp::squaredHeuristic(lightPdf, bsdfPdf);
					result += bsdf * radiance * 
						(weight * abs(omegaIn.z) / (n * lightPdf));
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
		const TPoint3D& target, const TVector3D& omegaOut,
		const TPoint2D* firstSample, const TPoint2D* lastSample,
		size_t gatherStage) const
{
	static PerThreadBuffer<TVector3D> omegaInsPerStage[numGatherStages_];
	static PerThreadBuffer<Spectrum> bsdfsPerStage[numGatherStages_];
	static PerThreadBuffer<TScalar> pdfsPerStage[numGatherStages_];
	
	PerThreadBuffer<TVector3D>& omegaIns = omegaInsPerStage[gatherStage];
	PerThreadBuffer<Spectrum>& bsdfs = bsdfsPerStage[gatherStage];
	PerThreadBuffer<TScalar>& pdfs = pdfsPerStage[gatherStage];

	const int n = static_cast<int>(lastSample - firstSample);
	omegaIns.growTo(n);
	bsdfs.growTo(n);
	pdfs.growTo(n);

	LASS_ASSERT(context.shader());
	context.shader()->sampleBsdf(sample, context, omegaOut,
		firstSample, lastSample, omegaIns.begin(), bsdfs.begin(), pdfs.begin(), Shader::capsReflection | Shader::capsDiffuse);

	Spectrum result;
	for (int i = 0; i < n; ++i)
	{
		if (pdfs[i] > 0 && bsdfs[i])
		{
			const TVector3D direction = context.shaderToWorld(omegaIns[i]);
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
				result += bsdfs[i] * radiance * abs(omegaIns[i].z) / (n * pdfs[i]);
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
			const TVector3D omegaIrradiance = context.worldToShader(nearest.normal);
			TScalar pdf;
			Spectrum bsdf;
			shader->bsdf(sample, context, omegaOut, 
				&omegaIrradiance, &omegaIrradiance + 1, &bsdf, &pdf, Shader::capsAll);
			return pdf > 0 && bsdf ? bsdf * nearest.irradiance : Spectrum();
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
		TScalar pdf;
		Spectrum bsdf;
		shader->bsdf(sample, context, omegaOut, 
			&omegaPhoton, &omegaPhoton + 1, &bsdf, &pdf, Shader::capsAll);
		if (pdf > 0 && bsdf)
		{
			result += bsdf * i->object()->power;
		}
	}

	return result / (TNumTraits::pi * photonNeighbourhood_.front().squaredDistance());
}



PhotonMapper::TMapTypeDictionary PhotonMapper::generateMapTypeDictionary()
{
	TMapTypeDictionary dictionary;
	dictionary.enableSuggestions(true);
	dictionary.add("global", mtGlobal);
	dictionary.add("caustic", mtCaustic);
	dictionary.add("volume", mtVolume);
	return dictionary;
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
