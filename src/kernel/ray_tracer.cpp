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
#include "ray_tracer.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS(RayTracer)
PY_CLASS_MEMBER_RW(RayTracer, "maxRayGeneration", maxRayGeneration, setMaxRayGeneration)
PY_CLASS_METHOD_NAME(RayTracer, reduce, "__reduce__")
PY_CLASS_METHOD_NAME(RayTracer, getState, "__getstate__")
PY_CLASS_METHOD_NAME(RayTracer, setState, "__setstate__")

// --- impl ----------------------------------------------------------------------------------------

namespace impl
{
	class RequestShaderSamples
	{
	public:
		RequestShaderSamples(const TSamplerPtr& iSampler): sampler_(iSampler) {};
		void operator()(const SceneObject& iObject)
		{
			Shader* shader = iObject.shader().get();
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



const unsigned RayTracer::maxRayGeneration() const
{
	return maxRayGeneration_;
}



void RayTracer::setScene(const TSceneObjectPtr& iScene)
{
    scene_ = iScene;
	lights_ = gatherLightContexts(iScene);
    doPreprocess();
}



void RayTracer::setMaxRayGeneration(unsigned iMaxRayGeneration)
{
	maxRayGeneration_ = iMaxRayGeneration;
}



void RayTracer::requestSamples(const TSamplerPtr& iSampler) 
{
	if (scene_ && iSampler)
	{
		iSampler->clearSubSequenceRequests();

		for (TLightContexts::iterator i = lights_.begin(); i != lights_.end(); ++i)
		{
			i->requestSamples(iSampler);
		}

		forAllObjects(scene_, impl::RequestShaderSamples(iSampler));

		doRequestSamples(iSampler); 
	}
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
		reinterpret_cast<PyObject*>(this->GetType()), python::makeTuple(), this->getState());
}



const TPyObjectPtr RayTracer::getState() const
{
	return doGetState();
}



void RayTracer::setState(const TPyObjectPtr& iState)
{
	doSetState(iState);
}



// --- protected -----------------------------------------------------------------------------------

RayTracer::RayTracer(PyTypeObject* iType):
    PyObjectPlus(iType),
	maxRayGeneration_(8),
	rayGeneration_(0)
{
}



// --- private -------------------------------------------------------------------------------------



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF