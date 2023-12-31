/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2023  Bram de Greve (bramz@users.sourceforge.net)
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



void LightContext::setSceneBound(const TSphere3D& bounds)
{
	if (auto* light = dynamic_cast<SceneLightGlobal*>(const_cast<SceneLight*>(light_)))
	{
#pragma LASS_TODO("Verify that global lights don't have transformations")
		light->setSceneBound(bounds);
	}
}



void LightContext::requestSamples(const TSamplerPtr& sampler)
{
	const size_t n = light_->numberOfEmissionSamples();
	idLightSamples_ = sampler->requestSubSequence2D(n);
	idBsdfSamples_ = sampler->requestSubSequence2D(n);
	idBsdfComponentSamples_ = sampler->requestSubSequence1D(sampler->subSequenceSize2D(idBsdfSamples_));
}



const Spectral LightContext::sampleEmission(
		const Sample& cameraSample, const TPoint2D& lightSample, const TPoint3D& target,
		BoundedRay& shadowRay, TScalar& pdf) const
{
	setTime(cameraSample.time());

	const TPoint3D localTarget = transform(target, worldToLocal_);

	BoundedRay localRay;
	const Spectral radiance = light_->sampleEmission(cameraSample, lightSample, localTarget, localRay, pdf);

	// we must transform back to world space.  But we already do know the starting point, so don't rely
	// on recalculating that one
	shadowRay = transform(localRay, localToWorld_);
	shadowRay.support() = target;

	return radiance;
}



const Spectral LightContext::sampleEmission(
		const Sample& cameraSample, const TPoint2D& lightSample,
		const TPoint3D& target,	const TVector3D& targetNormal,
		BoundedRay& shadowRay, TScalar& pdf) const
{
	setTime(cameraSample.time());

	const TPoint3D localTarget = transform(target, worldToLocal_);
	const TVector3D localNormal = normalTransform(targetNormal, worldToLocal_).normal();

	BoundedRay localRay;
	const Spectral radiance = light_->sampleEmission(
		cameraSample, lightSample, localTarget, localNormal, localRay, pdf);

	if (pdf > 0)
	{
		// we must transform back to world space.  But we already do know the starting point, so don't rely
		// on recalculating that one
		shadowRay = transform(localRay, localToWorld_);
		shadowRay.support() = target;
	}

	return radiance;
}



const Spectral LightContext::sampleEmission(
		const Sample& cameraSample, const TPoint2D& lightSampleA, const TPoint2D& lightSampleB,
		BoundedRay& emissionRay, TScalar& pdf) const
{
	setTime(cameraSample.time());

	BoundedRay localRay;
	const Spectral radiance = light_->sampleEmission(cameraSample, lightSampleA, lightSampleB, localRay, pdf);

	if (pdf > 0)
	{
		emissionRay = transform(localRay, localToWorld_);
	}
	return radiance;
}



const Spectral LightContext::emission(
	const Sample& cameraSample, const TRay3D& ray,
	BoundedRay& shadowRay, TScalar& pdf) const
{
	setTime(cameraSample.time());
	TScalar scale = 1;
	const TRay3D localRay = prim::transform(ray, worldToLocal_, scale);

	BoundedRay localShadowRay;
	const Spectral radiance = light_->emission(cameraSample, localRay, localShadowRay, pdf);

	if (pdf > 0)
	{
		shadowRay = BoundedRay(ray, localShadowRay.nearLimit() / scale, localShadowRay.farLimit() / scale);
	}
	return radiance;
}



TScalar LightContext::totalPower() const
{
	return light_->totalPower();
}



bool LightContext::isSingular() const
{
	return light_->isSingular();
}




// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

/** THREAD UNSAFE
 */
void LightContext::setTime(TTime time) const
{
	if ((hasMotion_ && time != timeOfTransformation_) || num::isNaN(timeOfTransformation_))
	{
		TTransformation3D temp;
		for (TObjectPath::const_iterator i = objectPath_.begin(); i != objectPath_.end(); ++i)
		{
			(*i)->localSpace(time, temp);
		}
		localToWorld_.swap(temp);
		worldToLocal_ = localToWorld_.inverse();
		timeOfTransformation_ = time;
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



void LightContexts::setSceneBound(const TSphere3D& bounds)
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
		contexts_[k].setSceneBound(bounds);
		totalPower_ += contexts_[k].totalPower();
		cdf_[k] = totalPower_;
	}
	std::transform(cdf_.begin(), cdf_.end(), cdf_.begin(), [this](TScalar cdf) { return cdf / totalPower_; });
}


void LightContexts::requestSamples(const TSamplerPtr& sampler)
{
	size_t n = contexts_.size();
	for (size_t k = 0; k < n; ++k)
	{
		contexts_[k].requestSamples(sampler);
	}
}



const LightContext* LightContexts::operator[](size_t i) const
{
	return &contexts_[i];
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



TScalar LightContexts::pdf(const LightContext* light) const
{
	LASS_ASSERT(!contexts_.empty());
	const size_t k = static_cast<size_t>(light - &contexts_[0]);
	LASS_ASSERT(k < contexts_.size());
	LASS_ASSERT(&contexts_[k] == light);

	return k > 0 ? (cdf_[k] - cdf_[k - 1]) : cdf_[k];
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



TScalar LightContexts::totalPower() const
{
	return totalPower_;
}






// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
