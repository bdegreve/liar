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

/** @class liar::tracers::PhotonMapper
 *  @brief a ray tracer that uses photon mapping for global illumination
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_TRACERS_PHOTON_MAPPER_H
#define LIAR_GUARDIAN_OF_INCLUSION_TRACERS_PHOTON_MAPPER_H

#include "tracers_common.h"
#include "../kernel/ray_tracer.h"
#include <lass/spat/kd_tree.h>
#include <lass/spat/aabp_tree.h>
#include <lass/num/random.h>
#include <lass/util/dictionary.h>
#include <lass/num/inverse_transform_sampling.h>

namespace liar
{
namespace tracers
{

class LIAR_TRACERS_DLL PhotonMapper: public RayTracer
{
	PY_HEADER(RayTracer)
public:

	PhotonMapper();

	size_t maxNumberOfPhotons() const;
	void setMaxNumberOfPhotons(size_t maxNumberOfPhotons);

	size_t globalMapSize() const;
	void setGlobalMapSize(size_t mapSize);

	TScalar causticsQuality() const;
	void setCausticsQuality(TScalar quality);

	size_t estimationSize(const std::string& mapType) const;
	void setEstimationSize(const std::string& mapType, size_t size);

	TScalar estimationRadius(const std::string& mapType) const;
	void setEstimationRadius(const std::string& mapType, TScalar radius);

	TScalar estimationTolerance(const std::string& mapType) const;
	void setEstimationTolerance(const std::string& mapType, TScalar radius);

	size_t numFinalGatherRays() const;
	void setNumFinalGatherRays(size_t numFinalGatherRays);

	size_t numSecondaryGatherRays() const;
	void setNumSecondaryGatherRays(size_t numFinalGatherRays);

	TScalar ratioPrecomputedIrradiance() const;
	void setRatioPrecomputedIrradiance(TScalar ratio);

	bool isVisualizingPhotonMap() const;
	void setVisualizePhotonMap(bool enabled = true);

	bool isRayTracingDirect() const;
	void setRayTracingDirect(bool enabled = true);

private:

	typedef std::vector<Medium*> TMediumStack;

	struct Photon
	{
		Photon(const TPoint3D& position, const TVector3D& omegaIn, const XYZ& power):
			position(position), omegaIn(omegaIn), power(power) {}
		TPoint3D position;
		TVector3D omegaIn;
		XYZ power;
	};
	typedef std::vector<Photon> TPhotonBuffer;

	struct Irradiance
	{
		Irradiance(const TPoint3D& position, const TVector3D& normal):
			position(position), normal(normal), irradiance(), squaredEstimationRadius(0) {}
		TPoint3D position;
		TVector3D normal;
		XYZ irradiance;
		TScalar squaredEstimationRadius;
	};
	typedef std::vector<Irradiance> TIrradianceBuffer;

#if 1
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
#else
	template <typename Buffer>
	struct ObjectTraits: spat::DefaultObjectTraits<typename Buffer::value_type, TAabb3D, meta::NullType, typename Buffer::const_iterator>
	{
		static const TAabb objectAabb(TObjectIterator it) 
		{ 
			return aabb(it->position);
		}
		static const TValue objectSquaredDistance(TObjectIterator it, const TPoint& point, const TInfo* /* info */)
		{
			return squaredDistance(it->position, point);
		}
	};
	typedef spat::AabpTree<Photon, ObjectTraits<TPhotonBuffer>, spat::DefaultSplitHeuristics<8> > TPhotonMap;
	typedef spat::AabpTree<Irradiance, ObjectTraits<TIrradianceBuffer>, spat::DefaultSplitHeuristics<8> > TIrradianceMap;
#endif

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
	const XYZ doCastRay(const Sample& sample, const DifferentialRay& primaryRay,
		TScalar& depth, TScalar& alpha, int generation) const;
	const TLightSamplesRange doSampleLights(const Sample& sample,
		const TPoint3D& target, const TVector3D& targetNormal) const;
	const TRayTracerPtr doClone() const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	void fillPhotonMap(const TSamplerPtr& sampler, const TimePeriod& period);
	void emitPhoton(const LightContext& light, TScalar lightPdf, const Sample& sample, 
		TRandomSecondary::TValue secondarySeed);
	void tracePhoton(const Sample& sample, const XYZ& power, const BoundedRay& ray,
		int geneneration, TUniformSecondary& uniform, bool isCaustic = false);
	void buildPhotonMap();
	void buildIrradianceMap();

	const XYZ traceDirect(const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& target, const TVector3D& targetNormal, const TVector3D& omegaOut) const;
	const XYZ gatherIndirect(const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& target, const TVector3D& omegaOut, const TPoint2D* firstSample, 
		const TPoint2D* lastSample, size_t gatherStage = 0) const;
	const XYZ gatherSecondary(const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& target, const TVector3D& omegaOut) const;

	const XYZ estimateIrradiance(const TPoint3D& point, const TVector3D& normal) const 
	{ 
		TScalar sqrRadius;
		size_t count;
		return estimateIrradiance(point, normal, sqrRadius, count);
	}
	const XYZ estimateIrradiance(const TPoint3D& point, const TVector3D& normal, TScalar& sqrEstimationRadius, size_t& estimationCount) const;
	const XYZ estimateRadiance(const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf, 
		const TPoint3D& point, const TVector3D& omegaOut, size_t gatherStage = 0) const;
	const XYZ estimateCaustics(const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf, 
		const TPoint3D& point, const TVector3D& omegaOut) const;
	void updateActualEstimationRadius(MapType mapType, TScalar radius) const;

	const XYZ inScattering(const Sample& sample, const kernel::BoundedRay& ray, TScalar tFar) const;

	TPhotonBuffer photonBuffer_[numMapTypes];
	TPhotonMap photonMap_[numMapTypes];
	TIrradianceBuffer irradianceBuffer_;
	TIrradianceMap irradianceMap_;
	TScalar estimationRadius_[numMapTypes];
	TScalar estimationTolerance_[numMapTypes];
	size_t estimationSize_[numMapTypes];
	mutable TScalar maxActualEstimationRadius_[numMapTypes]; /**< keeps track of actual maximum needed estimation radius, for post diagnostics */
	mutable std::vector<TPoint2D> secondaryGatherSamples_;
	mutable std::vector<TPoint2D> secondaryLightSamples_;
	mutable std::vector<TScalar> secondaryLightSelectorSamples_;
	mutable TRandomSecondary secondarySampler_;

	mutable std::vector<TScalar> grid_;
	mutable num::InverseTransformSampling2D<TScalar> gatherDistribution_;

	size_t maxNumberOfPhotons_;
	size_t globalMapSize_;
	TScalar causticsQuality_;
	size_t numFinalGatherRays_;
	size_t numSecondaryGatherRays_;
	TScalar ratioPrecomputedIrradiance_;
	int idFinalGatherSamples_;
	bool isVisualizingPhotonMap_;
	bool isRayTracingDirect_;

	// buffers
	mutable TPhotonNeighbourhood photonNeighbourhood_;

	static TMapTypeDictionary generateMapTypeDictionary();

	static TMapTypeDictionary mapTypeDictionary_;
};

}

}

#endif

// EOF
