/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2005  Bram de Greve
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
 *
 *  http://liar.sourceforge.net
 */

#include "kernel_common.h"
#include "light_context.h"

namespace liar
{
namespace kernel
{

// --- public --------------------------------------------------------------------------------------

LightContext::LightContext(const TObjectPath& iObjectPathToLight, const SceneLight& iLight):
	localToWorld_(),
	timeOfTransformation_(num::NumTraits<TTime>::qNaN),
	objectPath_(iObjectPathToLight),
	light_(&iLight),
	hasMotion_(false),
	idLightSamples_(-1),
	idBsdfSamples_(-1),
	idBsdfComponentSamples_(-1)
{
	LASS_ASSERT(!objectPath_.empty());
	LASS_ASSERT(objectPath_.back().get() == light_);

	for (TObjectPath::const_iterator i = objectPath_.begin(); i != objectPath_.end(); ++i)
	{
		hasMotion_ |= (*i)->hasMotion();
	}
}



void LightContext::requestSamples(const TSamplerPtr& iSampler)
{
	const int n = light_->numberOfEmissionSamples();
	idLightSamples_ = iSampler->requestSubSequence2D(n);
	idBsdfSamples_ = iSampler->requestSubSequence2D(n);
	idBsdfComponentSamples_ = iSampler->requestSubSequence1D(n);
}



const Spectrum LightContext::sampleEmission(const Sample& iCameraSample,
											const TVector2D& iLightSample, 
											const TPoint3D& iTarget,
											const TVector3D& iTargetNormal,
											BoundedRay& oShadowRay,
											TScalar& oPdf) const
{
	const TTime time = iCameraSample.time();
	if ((hasMotion_ && time != timeOfTransformation_) || num::isNaN(timeOfTransformation_))
	{
		TTransformation3D temp;
		for (TObjectPath::const_iterator i = objectPath_.begin(); i != objectPath_.end(); ++i)
		{
			(*i)->localSpace(time, temp);
		}
		localToWorld_.swap(temp);
		timeOfTransformation_ = time;
	}

	const TPoint3D localTarget = transform(iTarget, localToWorld_.inverse());
	const TVector3D localNormal = normalTransform(iTargetNormal, localToWorld_.inverse()).normal();
	
	BoundedRay localRay;
	const Spectrum radiance = light_->sampleEmission(
		iCameraSample, iLightSample, localTarget, localNormal, localRay, oPdf);

	// we must transform back to world space.  But we already do know the starting point, so don't rely
	// on recalculating that one
	oShadowRay = transform(localRay, localToWorld_);
	oShadowRay.support() = iTarget;

	return radiance;
}



// --- protected -----------------------------------------------------------------------------------

// --- private -------------------------------------------------------------------------------------

// --- free ----------------------------------------------------------------------------------------

namespace impl
{
	class LightContextGatherer: 
		public util::VisitorBase,
		public util::Visitor<SceneObject>,
		public util::Visitor<SceneLight>
	{
	public:
		const TLightContexts& operator()(const TSceneObjectPtr& iScene)
		{
			lights_.clear();
			objectPath_.clear();
			iScene->accept(*this);
			return lights_;
		}
	private:
	  
		void doVisit(SceneObject& iSceneObject)
		{
			TSceneObjectPtr objectPtr(python::PyPlus_INCREF(&iSceneObject));
			objectPath_.push_back(objectPtr);
		}
		void doVisit(SceneLight& iSceneLight)
		{
			TSceneObjectPtr objectPtr(python::PyPlus_INCREF(&iSceneLight));
			objectPath_.push_back(objectPtr);
			lights_.push_back(LightContext(objectPath_, iSceneLight));
		}
		void doVisitOnExit(SceneObject& iSceneObject)
		{
			objectPath_.pop_back();
		}
		void doVisitOnExit(SceneLight& iSceneLight)
		{
			objectPath_.pop_back();
		}

		TLightContexts lights_;
		LightContext::TObjectPath objectPath_;
	};
}

/** gather light contexts of all light in a scene, and return as vector
 */
TLightContexts gatherLightContexts(const TSceneObjectPtr& iScene)
{
	impl::LightContextGatherer gatherer;
	return gatherer(iScene);
}

}

}

// EOF