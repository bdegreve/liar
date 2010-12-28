/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
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
	template <typename Member, typename Getter>
	class RequestMemberSamples
	{
	public:
		RequestMemberSamples(const TSamplerPtr& sampler, Getter getter): sampler_(sampler), getter_(getter) {};
		void operator()(const SceneObject& object)
		{
			Member* member = (object.*getter_)().get();
			if (member && visited_.count(member) == 0)
			{
				member->requestSamples(sampler_);
				visited_.insert(member);
			}
		}
	private:
		std::set<Member*> visited_;
		TSamplerPtr sampler_;
		Getter getter_;
	};

	template <typename MemberPtr, typename Getter>
	RequestMemberSamples<MemberPtr, Getter> requestMemberSamples(const TSamplerPtr& sampler, Getter getter)
	{
		return RequestMemberSamples<MemberPtr, Getter>(sampler, getter);
	}
}



// --- public --------------------------------------------------------------------------------------

RayTracer::~RayTracer()
{
}



const TSceneObjectPtr& RayTracer::scene() const
{
	return scene_;
}



int RayTracer::maxRayGeneration() const
{
	return maxRayGeneration_;
}



void RayTracer::setScene(const TSceneObjectPtr& scene)
{
	scene_ = scene;
	lights_.gatherContexts(scene);
	mediumStack_ = MediumStack(scene->interior());
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
		lights_.requestSamples(sampler);
		forAllObjects(scene_, impl::requestMemberSamples<Shader>(sampler, &SceneObject::shader));
		forAllObjects(scene_, impl::requestMemberSamples<Medium>(sampler, &SceneObject::interior));
		doRequestSamples(sampler); 
	}
}



void RayTracer::preProcess(const TSamplerPtr& sampler, const TimePeriod& period, size_t numberOfThreads)
{
	const TAabb3D sceneBound = scene_->boundingBox();
	lights_.setSceneBound(sceneBound, period);
	doPreProcess(sampler, period, numberOfThreads);
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



namespace temp
{
	inline TScalar squaredHeuristic(TScalar pdfA, TScalar pdfB)
	{
		return num::sqr(pdfA) / (num::sqr(pdfA) + num::sqr(pdfB));
	}
}



const XYZ RayTracer::estimateLightContribution(
		const Sample& sample, const TBsdfPtr& bsdf, const LightContext& light, 
		const Sample::TSubSequence2D& lightSamples, const Sample::TSubSequence2D& bsdfSamples, const Sample::TSubSequence1D& componentSamples,
		const TPoint3D& target, const TVector3D& targetNormal, const TVector3D& omegaIn) const
{
	LASS_ASSERT(bsdfSamples.size() == componentSamples.size());

	const TScalar nl = static_cast<TScalar>(lightSamples.size());
	const TScalar nb = static_cast<TScalar>(bsdfSamples.size());
	const bool isMultipleImportanceSampling = nb > 0 && !light.isSingular();

	TBsdfCaps caps = Bsdf::capsAllDiffuse | Bsdf::capsGlossy;
	if (!light.isSingular())
	{
		caps |= Bsdf::capsSpecular;
	}
	// what about indirect estimator caps???

	XYZ result;
	const TPoint3D start = target;// + 10 * liar::tolerance * targetNormal;
	for (Sample::TSubSequence2D::iterator ls = lightSamples.begin(); ls != lightSamples.end(); ++ls)
	{
		BoundedRay shadowRay;
		TScalar lightPdf;
		const XYZ radiance = light.sampleEmission(sample, *ls, start, targetNormal, shadowRay, lightPdf);
		if (lightPdf <= 0 || !radiance)
		{
			continue;
		}
		const TVector3D& omegaOut = bsdf->worldToBsdf(shadowRay.direction());
		const BsdfOut out = bsdf->evaluate(omegaIn, omegaOut, caps);
		if (!out || scene()->isIntersecting(sample, shadowRay))
		{
			continue;
		}
		const XYZ trans = mediumStack().transmittance(shadowRay);
		const TScalar weight = isMultipleImportanceSampling ? temp::squaredHeuristic(nl * lightPdf, nb * out.pdf) : TNumTraits::one;
		const TScalar cosTheta = omegaOut.z;
		result += out.value * trans * radiance * (weight * num::abs(cosTheta) / (nl * lightPdf));
	}

	if (isMultipleImportanceSampling)
	{
		Sample::TSubSequence1D::iterator cs = componentSamples.begin();
		for (Sample::TSubSequence2D::iterator bs = bsdfSamples.begin(); bs != bsdfSamples.end(); ++bs, ++cs)
		{
			const SampleBsdfOut out = bsdf->sample(omegaIn, *bs, *cs, caps);
			if (!out)
			{
				continue;
			}
			const TRay3D ray(start, bsdf->bsdfToWorld(out.omegaOut).normal());
			BoundedRay shadowRay;
			TScalar lightPdf;
			const XYZ radiance = light.emission(sample, ray, shadowRay, lightPdf);
			if (lightPdf <= 0 || !radiance || scene()->isIntersecting(sample, shadowRay))
			{
				continue;
			}
			const XYZ trans = mediumStack().transmittance(shadowRay);
			const TScalar weight = temp::squaredHeuristic(nb * out.pdf, nl * lightPdf);
			const TScalar cosTheta = out.omegaOut.z;
			result += out.value * trans * radiance * (weight * abs(cosTheta) / (nb * out.pdf));
		}
	}

	return result;
}


// --- private -------------------------------------------------------------------------------------



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
