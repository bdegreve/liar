/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
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
 *  http://liar.sourceforge.net
 */

/** @class liar::tracers::PhotonMapper
 *  @brief a ray tracer that uses photon mapping for global illumination
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_TRACERS_PHOTON_MAPPER_H
#define LIAR_GUARDIAN_OF_INCLUSION_TRACERS_PHOTON_MAPPER_H

#include "tracers_common.h"
#include "../kernel/ray_tracer.h"
#include <lass/spat/kd_tree.h>
#include <lass/num/random.h>
#include <lass/util/dictionary.h>

namespace liar
{
namespace tracers
{

class LIAR_TRACERS_DLL PhotonMapper: public RayTracer
{
	PY_HEADER(RayTracer)
public:

	PhotonMapper();

	const unsigned maxNumberOfPhotons() const;
	void setMaxNumberOfPhotons(unsigned maxNumberOfPhotons);

	const unsigned globalMapSize() const;
	void setGlobalMapSize(unsigned mapSize);

	const TScalar causticsQuality() const;
	void setCausticsQuality(TScalar quality);

	const unsigned estimationSize(const std::string& mapType) const;
	void setEstimationSize(const std::string& mapType, unsigned size);

	const TScalar estimationRadius(const std::string& mapType) const;
	void setEstimationRadius(const std::string& mapType, TScalar radius);

	const TScalar estimationTolerance(const std::string& mapType) const;
	void setEstimationTolerance(const std::string& mapType, TScalar radius);

	const unsigned numFinalGatherRays() const;
	void setNumFinalGatherRays(unsigned numFinalGatherRays);

	const TScalar ratioPrecomputedIrradiance() const;
	void setRatioPrecomputedIrradiance(TScalar ratio);

	const bool isVisualizingPhotonMap() const;
	void setVisualizePhotonMap(bool enabled = true);

	const bool isRayTracingDirect() const;
	void setRayTracingDirect(bool enabled = true);
    
private:

	typedef std::vector<Medium*> TMediumStack;

	struct Photon
	{
		Photon(const TPoint3D& position, const TVector3D& omegaIn, const Spectrum& power):
			position(position), omegaIn(omegaIn), power(power) {}
		TPoint3D position;
		TVector3D omegaIn;
		Spectrum power;
	};
	typedef std::vector<Photon> TPhotonBuffer;

	struct Irradiance
	{
		Irradiance(const TPoint3D& position, const TVector3D& normal):
			position(position), normal(normal), irradiance() {}
		TPoint3D position;
		TVector3D normal;
		Spectrum irradiance;
	};
	typedef std::vector<Irradiance> TIrradianceBuffer;

	template <typename Buffer>
	struct KdTreeTraits
	{
		typedef typename Buffer::const_iterator TObjectIterator;
		typedef typename Buffer::const_reference TObjectReference;
		typedef TPoint3D TPoint;
		typedef TPoint::TValue TValue;
		typedef TPoint::TParam TParam;
		typedef TPoint::TReference TReference;
		typedef TPoint::TConstReference TConstReference;
		enum { dimension = TPoint::dimension };

		static const TPoint& position(TObjectIterator object) { return object->position; }
	};
	typedef spat::KdTree<Photon, KdTreeTraits<TPhotonBuffer> > TPhotonMap;
	typedef spat::KdTree<Irradiance, KdTreeTraits<TIrradianceBuffer> > TIrradianceMap;

	typedef std::vector<TPhotonMap::Neighbour> TPhotonNeighbourhood;

	enum MapType
	{
		mtNone = -1,
		mtGlobal = 0,
		mtCaustics,
		mtVolume,
		numMapTypes
	};

	typedef util::Dictionary<std::string, MapType> TMapTypeDictionary;

	typedef std::vector<TScalar> TLightCdf;
	typedef num::RandomMT19937 TRandomPrimary;
	typedef num::RandomMT19937 TRandomSecondary;
	//typedef num::RandomParkMiller TRandomSecondary;
	typedef num::DistributionUniform<TScalar, TRandomPrimary> TUniformPrimary;
	typedef num::DistributionUniform<TScalar, TRandomSecondary> TUniformSecondary;

	enum
	{
		numGatherStages_ = 2,
	};

	void doRequestSamples(const TSamplerPtr& sampler);
	void doPreProcess(const TSamplerPtr& sampler, const TimePeriod& period);
	const Spectrum doCastRay(const Sample& sample, const DifferentialRay& primaryRay,
		TScalar& depth, TScalar& alpha, int generation) const;
	const TLightSamplesRange doSampleLights(const Sample& sample,
		const TPoint3D& target, const TVector3D& targetNormal) const;
	const TRayTracerPtr doClone() const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	void fillPhotonMap(const TLightCdf& iCumulativeLightPower, const TSamplerPtr& sampler, 
		const TimePeriod& period);
	void emitPhoton(const LightContext& light, TScalar lightPdf, const Sample& sample, 
		TRandomSecondary::TValue secondarySeed);
	void tracePhoton(const Sample& sample, const Spectrum& power, const BoundedRay& ray,
		int geneneration, TUniformSecondary& uniform, bool isCaustic = false);
	void buildPhotonMap();
	void buildIrradianceMap();

	const Spectrum traceDirect(const Sample& sample, const IntersectionContext& context,
		const TPoint3D& target, const TVector3D& targetNormal, const TVector3D& omegaOut) const;
	const Spectrum gatherIndirect(const Sample& sample, const IntersectionContext& context,
		const TPoint3D& target, const TVector3D& omegaOut, const TPoint2D* firstSample, 
		const TPoint2D* lastSample, size_t gatherStage = 0) const;

	const Spectrum estimateIrradiance(const TPoint3D& point, const TVector3D& normal) const;
	const Spectrum estimateRadiance(const Sample& sample, const IntersectionContext& context, 
		const TPoint3D& point, const TVector3D& omegaOut, size_t gatherStage = 0) const;
	const Spectrum estimateCaustics(const Sample& sample, const IntersectionContext& context, 
		const TPoint3D& point, const TVector3D& omegaOut) const;

	mutable MediumStack mediumStack_;
	TPhotonBuffer photonBuffer_[numMapTypes];
	TPhotonMap photonMap_[numMapTypes];
	TIrradianceBuffer irradianceBuffer_;
	TIrradianceMap irradianceMap_;
	TScalar estimationRadius_[numMapTypes];
	TScalar estimationTolerance_[numMapTypes];
	unsigned estimationSize_[numMapTypes];

	unsigned maxNumberOfPhotons_;
	unsigned globalMapSize_;
	TScalar causticsQuality_;
	unsigned numFinalGatherRays_;
	TScalar ratioPrecomputedIrradiance_;
	int idFinalGatherSamples_;
	bool isVisualizingPhotonMap_;
	bool isRayTracingDirect_;

	// buffers
	mutable TPhotonNeighbourhood photonNeighbourhood_;
	mutable std::vector<BsdfIn> bsdfIns_;
	mutable std::vector<BsdfOut> bsdfOuts_;
	mutable std::vector<SampleBsdfIn> sampleBsdfIns_[numGatherStages_];
	mutable std::vector<SampleBsdfOut> sampleBsdfOuts_[numGatherStages_];

	static TMapTypeDictionary generateMapTypeDictionary();

	static TMapTypeDictionary mapTypeDictionary_;
};

}

}

#endif

// EOF
