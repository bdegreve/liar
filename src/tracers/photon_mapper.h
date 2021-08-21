/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2021  Bram de Greve (bramz@users.sourceforge.net)
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
#include "direct_lighting.h"
#include "../kernel/sampler_progressive.h"
#include <lass/prim/sphere_3d.h>
#include <lass/spat/kd_tree.h>
#include <lass/spat/aabp_tree.h>
#include <lass/spat/aabb_tree.h>
#include <lass/num/random.h>
#include <lass/util/dictionary.h>
#include <lass/num/inverse_transform_sampling.h>

namespace liar
{
namespace tracers
{

class LIAR_TRACERS_DLL PhotonMapper: public DirectLighting
{
	PY_HEADER(DirectLighting)
public:

	PhotonMapper();

	size_t maxNumberOfPhotons() const;
	void setMaxNumberOfPhotons(size_t maxNumberOfPhotons);

	size_t globalMapSize() const;
	void setGlobalMapSize(size_t mapSize);

	TScalar causticsQuality() const;
	void setCausticsQuality(TScalar quality);

	TScalar volumetricQuality() const;
	void setVolumetricQuality(TScalar quality);

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

	TScalar volumetricGatherQuality() const;
	void setVolumetricGatherQuality(TScalar quality);

	bool isVisualizingPhotonMap() const;
	void setVisualizePhotonMap(bool enabled = true);

	bool isRayTracingDirect() const;
	void setRayTracingDirect(bool enabled = true);

	bool isScatteringDirect() const;
	void setScatteringDirect(bool enabled = true);

	const TSamplerProgressivePtr& photonSampler() const;
	void setPhotonSampler(const TSamplerProgressivePtr& photonSampler);

private:

	typedef std::vector<Medium*> TMediumStack;

	struct Photon
	{
	public:
		Photon(const TPoint3D& position, const TVector3D& omegaIn, const Spectral& power, const Sample& sample) :
			position(position), omegaIn(omegaIn), power(power.xyz(sample)) {}
		TPoint3D position;
		TVector3D omegaIn;
		XYZ power;
		const Spectral spectralPower(const Sample& sample) const 
		{
			return Spectral::fromXYZ(power, sample, SpectralType::Illuminant);
		}
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
		const Spectral spectralIrradiance(const Sample& sample) const
		{
			return Spectral::fromXYZ(irradiance, sample, SpectralType::Illuminant);
		}
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

	struct VolumetricPhoton: Photon
	{
		VolumetricPhoton(const Photon& photon, bool isDirect): Photon(photon), radius(0), isDirect(isDirect) {}
		TScalar radius;
		bool isDirect;
	};
	typedef std::vector<VolumetricPhoton> TVolumetricPhotonBuffer;

	struct VolumetricPhotonTraits: spat::DefaultObjectTraits<VolumetricPhoton, TAabb3D, TRay3D, TVolumetricPhotonBuffer::const_iterator>
	{
		static const TAabb objectAabb(TObjectIterator it)
		{
			const TPoint3D center = it->position;
			const TVector3D halfExtent = TVector3D(it->radius, it->radius, it->radius);
			return TAabb(center - halfExtent, center + halfExtent);
		}
		static bool objectIntersects(TObjectIterator it, const TRay& ray, TParam tMin, TParam tMax, const TInfo* /*iInfo*/)
		{
			// ok, we're using a bit of a hack here. we're not intersecting the sphere surface, but it's volume.
			/*
			const TValue t = num::clamp(ray.t(it->position), tMin, tMax);
			return prim::squaredDistance(it->position, ray.point(t)) < num::sqr(it->radius);
			/*/
			const VolumetricPhoton& photon = *it;
			TVector3D v = photon.position - ray.support();
			const TVector3D& d = ray.direction();
			const TValue t = num::clamp(dot(v, d), tMin, tMax);
			v.x -= t * d.x;
			v.y -= t * d.y;
			v.z -= t * d.z;
			return v.squaredNorm() < num::sqr(photon.radius);
			/**/
		}
	};
	typedef spat::KdTree< VolumetricPhoton, KdTreeTraits<TVolumetricPhotonBuffer> > TPreliminaryVolumetricPhotonMap;
	typedef spat::AabpTree< VolumetricPhoton, VolumetricPhotonTraits, spat::DefaultSplitHeuristics > TVolumetricPhotonMap;
	typedef TVolumetricPhotonMap::TObjectIterators TVolumetricNeighbourhood;

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
	typedef num::DistributionUniform<TScalar, TRandomPrimary> TUniformPrimary;
	typedef num::DistributionUniform<TScalar, TRandomSecondary> TUniformSecondary;

	enum
	{
		numGatherStages_ = 2,
	};

	friend class IrradianceWorker;
	friend class VolumetricWorker;

	// RayTracer
	void doRequestSamples(const TSamplerPtr& sampler);
	void doPreProcess(const TSamplerPtr& sampler, const TimePeriod& period, size_t numberOfThreads);
	const TRayTracerPtr doClone() const;
	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	// DirectLighting
	const Spectral doShadeMedium(const kernel::Sample& sample, const kernel::BoundedRay& ray, Spectral& transparency) const;
	const Spectral doShadeSurface(const kernel::Sample& sample, const DifferentialRay& primaryRay, const IntersectionContext& context,
		const TPoint3D& point, const TVector3D& normal, const TVector3D& omega, bool highQuality) const;

	bool hasFinalGather() const { return isRayTracingDirect_ && (numFinalGatherRays_ > 0); }
	bool hasSecondaryGather() const { return hasFinalGather() && (numSecondaryGatherRays_ > 0); }

	size_t fillPhotonMaps(const TSamplerProgressivePtr& sampler, const TimePeriod& period);
	void emitPhoton(const LightContext& light, TScalar lightPdf, const Sample& sample, TRandomSecondary::TValue secondarySeed);
	void tracePhoton(const Sample& sample, const Spectral& power, const BoundedRay& ray, size_t geneneration, TUniformSecondary& uniform, bool isCaustic = false);
	template <typename PhotonBuffer, typename PhotonMap> void buildPhotonMap(MapType mapType, PhotonBuffer& buffer, PhotonMap& map, TScalar powerScale);
	void buildIrradianceMap(size_t numberOfThreads);
	void buildVolumetricPhotonMap(const TPreliminaryVolumetricPhotonMap& preliminaryVolumetricMap, size_t numberOfThreads);

	const Spectral gatherIndirect(const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& target, const TVector3D& omegaOut, const TPoint2D* firstSample, const TPoint2D* lastSample,
		const TScalar* firstComponentSample, const TScalar* firstVolumetricSample, size_t gatherStage = 0) const;
	const Spectral gatherSecondary(const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& target, const TVector3D& omegaOut) const;
	const Spectral traceGatherRay(const Sample& sample, const BoundedRay& ray, bool gatherVolumetric, size_t gatherStage, size_t rayGeneration) const;

	const XYZ estimateIrradiance(const TPoint3D& point, const TVector3D& normal, TScalar& sqrEstimationRadius, size_t& estimationCount) const;
	const XYZ estimateIrradiance(const TPoint3D& point, const TVector3D& normal) const
	{
		TScalar sqrRadius;
		size_t count;
		return estimateIrradiance(point, normal, sqrRadius, count);
	}
	const XYZ estimateIrradianceImpl(TPhotonNeighbourhood& neighbourhood, const TPoint3D& point, const TVector3D& normal, TScalar& sqrEstimationRadius, size_t& estimationCount) const;

	const Spectral estimateRadiance(const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& point, const TVector3D& omegaOut, TScalar& sqrEstimationRadius) const;
	const Spectral estimateRadiance(const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& point, const TVector3D& omegaOut) const
	{
		TScalar sqrRadius;
		return estimateRadiance(sample, context, bsdf, point, omegaOut, sqrRadius);
	}

	const Spectral estimateCaustics(const Sample& sample, const IntersectionContext& context, const TBsdfPtr& bsdf,
		const TPoint3D& point, const TVector3D& omegaOut) const;
	void updateActualEstimationRadius(MapType mapType, TScalar radius) const;
	void updateStorageProbabilities();

	const Spectral estimateVolumetric(const Sample& sample, const kernel::BoundedRay& ray, bool dropDirectPhotons = false) const;

	struct SharedData
	{
		TPhotonBuffer globalBuffer_;
		TIrradianceBuffer irradianceBuffer_;
		TPhotonBuffer causticsBuffer_;
		TVolumetricPhotonBuffer volumetricBuffer_;
		TPhotonMap globalMap_;
		TIrradianceMap irradianceMap_;
		TPhotonMap causticsMap_;
		TVolumetricPhotonMap volumetricMap_;
	};
	util::SharedPtr<SharedData> shared_;

	TScalar estimationRadius_[numMapTypes];
	TScalar estimationTolerance_[numMapTypes];
	size_t estimationSize_[numMapTypes];
	mutable TScalar maxActualEstimationRadius_[numMapTypes]; /**< keeps track of actual maximum needed estimation radius, for post diagnostics */
	mutable std::vector<TPoint2D> secondaryGatherBsdfSamples_;
	mutable std::vector<TScalar> secondaryGatherComponentSamples_;
	mutable std::vector<TScalar> secondaryGatherVolumetricSamples_;

	mutable std::vector<TScalar> grid_;
	mutable num::InverseTransformSampling2D<TScalar> gatherDistribution_;

	size_t maxNumberOfPhotons_;
	size_t globalMapSize_;
	TScalar causticsQuality_;
	TScalar volumetricQuality_;
	TScalar storageProbability_[numMapTypes];
	size_t numFinalGatherRays_;
	size_t numSecondaryGatherRays_;
	TScalar ratioPrecomputedIrradiance_;
	TScalar volumetricGatherQuality_;
	int idFinalGatherSamples_;
	int idFinalGatherComponentSamples_;
	int idFinalVolumetricGatherSamples_;
	bool isVisualizingPhotonMap_;
	bool isRayTracingDirect_;
	bool isScatteringDirect_;

	TSamplerProgressivePtr photonSampler_;
	int idLightSelector_;

	// buffers
	mutable TPhotonNeighbourhood photonNeighbourhood_;
	mutable TVolumetricNeighbourhood volumetricNeighbourhood_;

	static TMapTypeDictionary generateMapTypeDictionary();

	static TMapTypeDictionary mapTypeDictionary_;
};

}

}

#endif

// EOF
