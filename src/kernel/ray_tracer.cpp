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
#include "ray_tracer.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(RayTracer, "Abstract base class of ray tracers")
PY_CLASS_MEMBER_RW(RayTracer, maxRayGeneration, setMaxRayGeneration)
PY_CLASS_METHOD_NAME(RayTracer, reduce, "__reduce__")
PY_CLASS_METHOD_NAME(RayTracer, getState, "__getstate__")
PY_CLASS_METHOD_NAME(RayTracer, setState, "__setstate__")

// --- impl ----------------------------------------------------------------------------------------

namespace impl
{
	class RequestShaderSamples
	{
	public:
		RequestShaderSamples(const TSamplerPtr& sampler): sampler_(sampler) {};
		void operator()(const SceneObject& object)
		{
			Shader* shader = object.shader().get();
			if (shader && visited_.count(shader) == 0)
			{
				shader->requestSamples(sampler_);
				visited_.insert(shader);
			}
		}
	private:
		std::set<Shader*> visited_;
		TSamplerPtr sampler_;
	};
}



// --- public --------------------------------------------------------------------------------------

RayTracer::~RayTracer()
{
}



const TSceneObjectPtr& RayTracer::scene() const
{
    return scene_;
}



const int RayTracer::maxRayGeneration() const
{
	return maxRayGeneration_;
}



void RayTracer::setScene(const TSceneObjectPtr& scene)
{
    scene_ = scene;
	lights_ = gatherLightContexts(scene);
}



void RayTracer::setMaxRayGeneration(int iMaxRayGeneration)
{
	maxRayGeneration_ = std::max<int>(iMaxRayGeneration, 0);
}



void RayTracer::requestSamples(const TSamplerPtr& sampler) 
{
	if (scene_ && sampler)
	{
		sampler->clearSubSequenceRequests();

		for (TLightContexts::iterator i = lights_.begin(); i != lights_.end(); ++i)
		{
			i->requestSamples(sampler);
		}

		forAllObjects(scene_, impl::RequestShaderSamples(sampler));

		doRequestSamples(sampler); 
	}
}



void RayTracer::preProcess(const TSamplerPtr& sampler, const TimePeriod& period)
{
	const TAabb3D sceneBound = scene_->boundingBox();
	for (TLightContexts::iterator i = lights_.begin(); i != lights_.end(); ++i)
	{
		i->setSceneBound(sceneBound, period);
	}
	doPreProcess(sampler, period);
}



const TRayTracerPtr RayTracer::clone() const
{
	const TRayTracerPtr result = doClone();
	LASS_ASSERT(typeid(*result) == typeid(*this));
	return result;
}



const TPyObjectPtr RayTracer::reduce() const
{
	return python::makeTuple(
		python::fromNakedToSharedPtrCast<PyObject>(reinterpret_cast<PyObject*>(this->_lassPyGetClassDef()->type())), 
		python::makeTuple(), this->getState());
}



const TPyObjectPtr RayTracer::getState() const
{
	return doGetState();
}



void RayTracer::setState(const TPyObjectPtr& state)
{
	doSetState(state);
}



// --- protected -----------------------------------------------------------------------------------

RayTracer::RayTracer():
	maxRayGeneration_(8),
	rayGeneration_(-1)
{
}



// --- private -------------------------------------------------------------------------------------



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
