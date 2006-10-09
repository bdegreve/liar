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

/** @class liar::tracers::PhotonMapper
 *  @brief a ray tracer that uses photon mapping for global illumination
 *  @author Bram de Greve [Bramz]
 */

#pragma once
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

	const unsigned requestedMapSize(const std::string& mapType) const;
	void setRequestedMapSize(const std::string& mapType, unsigned mapSize);

	const unsigned estimationSize(const std::string& mapType) const;
	void setEstimationSize(const std::string& mapType, unsigned size);

	const TScalar estimationRadius(const std::string& mapType) const;
	void setEstimationRadius(const std::string& mapType, TScalar radius);

	const unsigned maxNumberOfPhotons() const;
	void setMaxNumberOfPhotons(unsigned maxNumberOfPhotons);

	const unsigned numFinalGatherRays() const;
	void setNumFinalGatherRays(unsigned numFinalGatherRays);

	const bool isVisualizingPhotonMap() const;
	void setVisualizePhotonMap(bool enabled = true);

	const bool isRayTracingDirect() const;
	void setRayTracingDirect(bool enabled = true);
    
private:

	struct Photon
	{
		Photon(const TPoint3D& position, const TVector3D& omegaIn, const Spectrum& power):
			position(position), omegaIn(omegaIn), power(power) {}
		TPoint3D position;
		TVector3D omegaIn;
		Spectrum power;
	};

	typedef std::vector<Photon> TPhotonBuffer;

	struct PhotonTraits
	{
		typedef TPhotonBuffer::const_iterator TObjectIterator;
		typedef const Photon& TObjectReference;
		typedef TPoint3D TPoint;
		typedef TPoint::TValue TValue;
		typedef TPoint::TParam TParam;
		typedef TPoint::TReference TReference;
		typedef TPoint::TConstReference TConstReference;
		enum { dimension = TPoint::dimension };

		static const TPoint& position(TObjectIterator object) { return object->position; }
	};

	typedef spat::KdTree<Photon, PhotonTraits> TPhotonMap;
	typedef std::vector<TPhotonMap::Neighbour> TPhotonNeighbourhood;

	enum MapType
	{
		mtGlobal,
		mtCaustic,
		mtVolume,
		numMapTypes
	};

	typedef util::Dictionary<std::string, MapType> TMapTypeDictionary;

	typedef std::vector<TScalar> TLightCdf;
	typedef num::RandomMT19937 TRandomPrimary;
	typedef num::RandomParkMiller TRandomSecondary;
	typedef num::DistributionUniform<TScalar, TRandomPrimary> TUniformPrimary;
	typedef num::DistributionUniform<TScalar, TRandomSecondary> TUniformSecondary;

	void doPreprocess();
	void doRequestSamples(const TSamplerPtr& sampler);
	const Spectrum doCastRay(const Sample& sample, const DifferentialRay& primaryRay) const;
	const TLightSamplesRange doSampleLights(const Sample& sample,
		const TPoint3D& target, const TVector3D& targetNormal) const;
	const TRayTracerPtr doClone() const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	void buildPhotonMap(MapType iType, const TLightCdf& iCumulativeLightPower);
	void emitPhoton(MapType iType, const LightContext& light, TScalar lightPdf, 
		TRandomSecondary::TValue iSeed);
	void tracePhoton(const Sample& sample, const Spectrum& power, const BoundedRay& ray,
		unsigned geneneration, TUniformSecondary& uniform);
	const Spectrum estimateIrradiance(const TPoint3D& point, const TVector3D& normal) const;
	const Spectrum estimateRadiance(const Sample& sample, const IntersectionContext& context, 
		const TPoint3D& point, const TVector3D& omegaOut) const;

	TPhotonBuffer photonBuffer_[numMapTypes];
	TPhotonMap photonMap_[numMapTypes];
	mutable TPhotonNeighbourhood photonNeighbourhood_;
	TScalar estimationRadius_[numMapTypes];
	unsigned requestedMapSize_[numMapTypes];
	unsigned estimationSize_[numMapTypes];
	unsigned maxNumberOfPhotons_;
	unsigned numFinalGatherRays_;
	int idFinalGatherSamples_;
	bool isVisualizingPhotonMap_;
	bool isRayTracingDirect_;

	static TMapTypeDictionary generateMapTypeDictionary();

	static TMapTypeDictionary mapTypeDictionary_;
};

}

}

#endif

// EOF
