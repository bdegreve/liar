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

#include "kernel_common.h"
#include "light_context.h"

namespace liar
{
namespace kernel
{

util::CriticalSection LightContext::lock_;

// --- public --------------------------------------------------------------------------------------

LightContext::LightContext(const TObjectPath& objectPathToLight, const SceneLight& light):
	localToWorld_(),
	timeOfTransformation_(num::NumTraits<TTime>::qNaN),
	objectPath_(objectPathToLight),
	light_(&light),
	idLightSamples_(-1),
	idBsdfSamples_(-1),
	idBsdfComponentSamples_(-1),
	hasMotion_(false)
{
	LASS_ASSERT(!objectPath_.empty());
	LASS_ASSERT(objectPath_.back().get() == light_);

	for (TObjectPath::const_iterator i = objectPath_.begin(); i != objectPath_.end(); ++i)
	{
		hasMotion_ |= (*i)->hasMotion();
	}
}



void LightContext::setSceneBound(const TAabb3D& bound, const TimePeriod& period)
{
	setTime(period.begin());
	const TAabb3D beginBound = transform(bound, localToWorld_.inverse());
	setTime(period.end());
	const TAabb3D endBound = transform(bound, localToWorld_.inverse());
	localBound_ = beginBound + endBound;
}



void LightContext::requestSamples(const TSamplerPtr& sampler)
{
	const size_t n = light_->numberOfEmissionSamples();
	idLightSamples_ = sampler->requestSubSequence2D(n);
	idBsdfSamples_ = sampler->requestSubSequence2D(n);
	idBsdfComponentSamples_ = sampler->requestSubSequence1D(n);
}



const XYZ LightContext::sampleEmission(
		const Sample& cameraSample, const TPoint2D& lightSample, const TPoint3D& target,
		BoundedRay& shadowRay, TScalar& pdf) const
{
	setTime(cameraSample.time());

	const TPoint3D localTarget = transform(target, localToWorld_.inverse());
	
	BoundedRay localRay;
	const XYZ radiance = light_->sampleEmission(cameraSample, lightSample, localTarget, localRay, pdf);

	// we must transform back to world space.  But we already do know the starting point, so don't rely
	// on recalculating that one
	shadowRay = transform(localRay, localToWorld_);
	shadowRay.support() = target;

	return radiance;
}



const XYZ LightContext::sampleEmission(
		const Sample& cameraSample, const TPoint2D& lightSample, 
		const TPoint3D& target,	const TVector3D& targetNormal,		
		BoundedRay& shadowRay, TScalar& pdf) const
{
	setTime(cameraSample.time());

	const TPoint3D localTarget = transform(target, localToWorld_.inverse());
	const TVector3D localNormal = normalTransform(targetNormal, localToWorld_.inverse()).normal();
	
	BoundedRay localRay;
	const XYZ radiance = light_->sampleEmission(
		cameraSample, lightSample, localTarget, localNormal, localRay, pdf);

	// we must transform back to world space.  But we already do know the starting point, so don't rely
	// on recalculating that one
	shadowRay = transform(localRay, localToWorld_);
	shadowRay.support() = target;

	return radiance;
}



const XYZ LightContext::sampleEmission(
		const Sample& cameraSample, const TPoint2D& lightSampleA, const TPoint2D& lightSampleB, 
		BoundedRay& emissionRay, TScalar& pdf) const
{
	setTime(cameraSample.time());

	BoundedRay localRay;
	const XYZ radiance = light_->sampleEmission(
		cameraSample, lightSampleA, lightSampleB, localBound_, localRay, pdf);
	
	emissionRay = transform(localRay, localToWorld_);
	return radiance;
}



const XYZ LightContext::emission(
		const Sample& cameraSample, const TRay3D& ray,
		BoundedRay& shadowRay, TScalar& pdf) const
{
	setTime(cameraSample.time());
	TScalar scale = 1;
	const TRay3D localRay = prim::transform(ray, localToWorld_.inverse(), scale);

	BoundedRay localShadowRay;
	const XYZ radiance = light_->emission(cameraSample, localRay, localShadowRay, pdf);

	shadowRay = BoundedRay(
		ray, localShadowRay.nearLimit() / scale, localShadowRay.farLimit() / scale);
	return radiance;
}



const XYZ LightContext::totalPower() const
{
	return light_->totalPower(localBound_);
}



bool LightContext::isSingular() const
{
	return light_->isSingular();
}




// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void LightContext::setTime(TTime time) const
{
	if ((hasMotion_ && time != timeOfTransformation_) || num::isNaN(timeOfTransformation_))
	{
		LASS_LOCK(lock_)
		{
			TTransformation3D temp;
			for (TObjectPath::const_iterator i = objectPath_.begin(); i != objectPath_.end(); ++i)
			{
				(*i)->localSpace(time, temp);
			}
			localToWorld_.swap(temp);
			timeOfTransformation_ = time;
		}
	}
}


// --- LightContexts -------------------------------------------------------------------------------

namespace impl
{
	class LightContextGatherer: 
		public util::VisitorBase,
		public util::Visitor<SceneObject>,
		public util::Visitor<SceneLight>
	{
	public:
		LightContextGatherer(LightContexts& contexts):
			lights_(contexts)
		{
		}
		void operator()(const TSceneObjectPtr& scene)
		{
			lights_.clear();
			objectPath_.clear();
			scene->accept(*this);
		}
	private:
	  
		void doPreVisit(SceneObject& object)
		{
			objectPath_.push_back(python::fromNakedToSharedPtrCast<SceneObject>(&object));
		}
		void doPreVisit(SceneLight& light)
		{
			objectPath_.push_back(python::fromNakedToSharedPtrCast<SceneObject>(&light));
			lights_.add(LightContext(objectPath_, light));
		}
		void doPostVisit(SceneObject&)
		{
			objectPath_.pop_back();
		}
		void doPostVisit(SceneLight&)
		{
			objectPath_.pop_back();
		}

		LightContexts& lights_;
		LightContext::TObjectPath objectPath_;
	};
}



void LightContexts::clear()
{
	contexts_.clear();
	cdf_.clear();
	totalPower_ = 0;
}



void LightContexts::add(const LightContext& context)
{
	contexts_.push_back(context);
}



void LightContexts::gatherContexts(const TSceneObjectPtr& scene)
{
	impl::LightContextGatherer gatherer(*this);
	gatherer(scene);
}



void LightContexts::setSceneBound(const TAabb3D& bound, const TimePeriod& period)
{
	size_t n = contexts_.size();
	if (n == 0)
	{
		return;
	}
	cdf_.resize(n);
	totalPower_ = 0;
	for (size_t k = 0; k < n; ++k)
	{
		contexts_[k].setSceneBound(bound, period);
		const XYZ power = contexts_[k].totalPower();
		totalPower_ += power;
		cdf_[k] = positiveAverage(power);
	}
	std::partial_sum(cdf_.begin(), cdf_.end(), cdf_.begin());
	std::transform(cdf_.begin(), cdf_.end(), cdf_.begin(), std::bind2nd(std::divides<TScalar>(), cdf_.back()));
}


void LightContexts::requestSamples(const TSamplerPtr& sampler)
{
	size_t n = contexts_.size();
	for (size_t k = 0; k < n; ++k)
	{
		contexts_[k].requestSamples(sampler);
	}
}



const LightContext& LightContexts::operator[](size_t i) const
{
	return contexts_[i];
}



const LightContext* LightContexts::sample(TScalar x, TScalar& pdf) const
{
	if (contexts_.empty())
	{
		return 0;
	}
	const std::vector<TScalar>::const_iterator i = std::lower_bound(cdf_.begin(), cdf_.end(), x);
	const size_t k = std::min(static_cast<size_t>(i - cdf_.begin()), cdf_.size() - 1);
	pdf = *i - (k > 0 ? cdf_[k - 1] : TNumTraits::zero);
	return &contexts_[k];
}



LightContexts::TIterator LightContexts::begin() const
{
	return contexts_.begin();
}



LightContexts::TIterator LightContexts::end() const
{
	return contexts_.end();
}



size_t LightContexts::size() const
{
	return contexts_.size();
}



const XYZ LightContexts::totalPower() const
{
	return totalPower_;
}






// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
