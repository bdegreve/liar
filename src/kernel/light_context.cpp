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

LightContext::LightContext(const TSceneLightPtr& iLight, const TTransformation3D& iLocalToWorld):
	light_(iLight),
	localToWorld_(iLocalToWorld),
	idLightSamples_(-1),
	idBsdfSamples_(-1),
	idBsdfComponentSamples_(-1)
{
}



void LightContext::requestSamples(const TSamplerPtr& iSampler)
{
	const int n = light_->numberOfRadianceSamples();
	idLightSamples_ = iSampler->requestSubSequence2D(n);
	idBsdfSamples_ = iSampler->requestSubSequence2D(n);
	idBsdfComponentSamples_ = iSampler->requestSubSequence1D(n);
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
			savedSpaces_.clear();
			localToWorld_ = TTransformation3D();
			iScene->accept(*this);
			return lights_;
		}
	private:
	  
		void doVisit(SceneObject& iSceneObject)
		{
			savedSpaces_.push_back(localToWorld_);
			iSceneObject.localSpace(localToWorld_);
		}
		void doVisit(SceneLight& iSceneLight)
		{
			savedSpaces_.push_back(localToWorld_);
			iSceneLight.localSpace(localToWorld_);
			TSceneLightPtr lightPtr(python::PyPlus_INCREF(&iSceneLight));
			lights_.push_back(LightContext(lightPtr, localToWorld_));
		}
		void doVisitOnExit(SceneObject& iSceneObject)
		{
			localToWorld_ = savedSpaces_.back();
			savedSpaces_.pop_back();
		}
		void doVisitOnExit(SceneLight& iSceneLight)
		{
			localToWorld_ = savedSpaces_.back();
			savedSpaces_.pop_back();
		}

		TLightContexts lights_;
		TTransformation3D localToWorld_;
		std::vector<TTransformation3D> savedSpaces_;
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