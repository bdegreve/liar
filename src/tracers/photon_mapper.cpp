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
#include <lass/num/distribution.h>
#include <lass/util/progress_indicator.h>
#include <lass/stde/range_algorithm.h>

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
PY_CLASS_MEMBER_RW(PhotonMapper, "isVisualizingPhotonMap", isVisualizingPhotonMap, setVisualizePhotonMap)
PY_CLASS_MEMBER_RW(PhotonMapper, "isRayTracingDirect", isRayTracingDirect, setRayTracingDirect)

PhotonMapper::TMapTypeDictionary PhotonMapper::mapTypeDictionary_ = 
	PhotonMapper::generateMapTypeDictionary();

// --- public --------------------------------------------------------------------------------------

PhotonMapper::PhotonMapper():
	maxNumberOfPhotons_(1000000),
	isVisualizingPhotonMap_(false),
	isRayTracingDirect_(true),
	photonNeighbourhood_(1)
{
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

void PhotonMapper::doPreprocess()
{
	TLightCdf lightCdf;
	for (TLightContexts::const_iterator i = lights().begin(); i != lights().end(); ++i)
	{
		lightCdf.push_back(i->totalPower().average());
	}
	std::partial_sum(lightCdf.begin(), lightCdf.end(), lightCdf.begin());
	std::transform(lightCdf.begin(), lightCdf.end(), lightCdf.begin(), 
		std::bind2nd(std::divides<TScalar>(), lightCdf.back()));

	buildPhotonMap(mtGlobal, lightCdf);
}




void PhotonMapper::doRequestSamples(const kernel::TSamplerPtr& sampler)
{
}



const Spectrum PhotonMapper::doCastRay(const kernel::Sample& sample,
		const kernel::DifferentialRay& primaryRay) const
{
	Intersection intersection;
	scene()->intersect(sample, primaryRay, intersection);
	if (!intersection)
	{
		return Spectrum();
	}

	IntersectionContext context(this);
	scene()->localContext(sample, primaryRay, intersection, context);

	if (isVisualizingPhotonMap_)
	{
		return estimateIrradiance(context);
	}

	const Shader* const shader = context.shader();
	if (!shader)
	{
		return Spectrum();
	}

	const TVector3D dirOut = -primaryRay.direction();

	if (isRayTracingDirect_)
	{

	}

	return estimateRadiance(sample, context, dirOut);
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



void PhotonMapper::buildPhotonMap(MapType type, const TLightCdf& lightCdf)
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
			const TRandomSecondary::TValue seed = random();
            emitPhoton(type, lights()[light], lightPdf, seed);
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
	photonMap_[type].reset(photonBuffer_[type].begin(), photonBuffer_[type].end());
}



void PhotonMapper::emitPhoton(MapType iType, const LightContext& light, TScalar lightPdf,
		TRandomSecondary::TValue iSeed)
{
	Sample dummy;

	TRandomSecondary random(iSeed);
	TUniformSecondary uniform(random);
	TPoint2D sampleA(uniform(), uniform());
	TPoint2D sampleB(uniform(), uniform());

	TRay3D ray;
	TScalar pdf;
	Spectrum spectrum = light.sampleEmission(sampleA, sampleB, ray, pdf);
	if (pdf > 0 && spectrum)
	{
		spectrum /= lightPdf * pdf; 
		tracePhoton(dummy, spectrum, BoundedRay(ray), 0, uniform);
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
		if (generation > 0 || isVisualizingPhotonMap_ || !isRayTracingDirect_)
		{
			photonBuffer_[mtGlobal].push_back(photon);
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
		&omegaIn, &spectrum, &pdf);
	if (pdf == 0 || !spectrum)
	{
		return;
	}

	const TScalar cos_thetaOut = omegaOut.z;
	Spectrum newPower = power * spectrum * (abs(cos_thetaOut) / pdf);
	const TScalar attenuation = newPower.average() / power.average();
	const TScalar scatterProbability = std::min(TNumTraits::one, attenuation);
	if (uniform() < scatterProbability)
	{
		newPower /= scatterProbability;
		BoundedRay newRay(hitPoint, context.shaderToWorld(omegaOut));
		tracePhoton(sample, newPower, newRay, generation + 1, uniform);
	}
}



const Spectrum PhotonMapper::estimateIrradiance(const IntersectionContext& context) const
{
	const TPhotonNeighbourhood::const_iterator last = photonMap_[mtGlobal].rangeSearch(
		context.point(), estimationRadius_[mtGlobal], estimationSize_[mtGlobal], 
		photonNeighbourhood_.begin());
	if (last == photonNeighbourhood_.begin())
	{
		return Spectrum();
	}

	Spectrum result;
	const TVector3D worldNormal = context.shaderToWorld(TVector3D(0, 0, 1));
	for (TPhotonNeighbourhood::const_iterator i = photonNeighbourhood_.begin(); i != last; ++i)
	{
		if (dot(i->object()->omegaIn, worldNormal) > 0)
		{
			result += i->object()->power;
		}
	}

	return result / (TNumTraits::pi * photonNeighbourhood_.front().squaredDistance());
}



const Spectrum PhotonMapper::estimateRadiance(
		const Sample& sample,
		const IntersectionContext& context, 
		const TVector3D& omegaOut) const
{
	const Shader* const shader = context.shader();
	if (!shader || !shader->hasCaps(Shader::capsDiffuse))
	{
		return Spectrum();
	}
	
	const TPhotonNeighbourhood::const_iterator last = photonMap_[mtGlobal].rangeSearch(
		context.point(), estimationRadius_[mtGlobal], estimationSize_[mtGlobal], 
		photonNeighbourhood_.begin());
	if (last == photonNeighbourhood_.begin())
	{
		return Spectrum();
	}

	Spectrum result;
	for (TPhotonNeighbourhood::const_iterator i = photonNeighbourhood_.begin(); i != last; ++i)
	{
		if (dot(i->object()->omegaIn, context.normal()) > 0)
		{
			TScalar pdf;
			Spectrum bsdf;
			const TVector3D& omegaPhoton = i->object()->omegaIn;
			shader->bsdf(sample, context, omegaOut, 
				&omegaPhoton, &omegaPhoton + 1, &bsdf, &pdf);
			if (pdf > 0 && bsdf)
			{
				result += bsdf * i->object()->power;
			}
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
