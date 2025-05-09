/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2024  Bram de Greve (bramz@users.sourceforge.net)
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



size_t RayTracer::maxRayGeneration() const
{
	return maxRayGeneration_;
}



void RayTracer::setScene(const TSceneObjectPtr& scene)
{
	scene_ = scene;
	lights_.gatherContexts(scene);
	mediumStack_ = MediumStack(scene->interior());
}



void RayTracer::setMaxRayGeneration(size_t maxRayGeneration)
{
	maxRayGeneration_ = maxRayGeneration;
}



void RayTracer::requestSamples(const TSamplerPtr& sampler)
{
	if (sampler)
	{
		sampler->clearSubSequenceRequests();
		doRequestSamples(sampler);
	}
}



void RayTracer::preProcess(const TSamplerPtr& sampler, const TimePeriod& period, size_t numberOfThreads)
{
	lights_.setSceneBound(scene_->boundingSphere());
	doPreProcess(sampler, period, numberOfThreads);
}



const TRayTracerPtr RayTracer::clone() const
{
	const TRayTracerPtr result = doClone();
	[[maybe_unused]] const auto& tracer = *result;
	LASS_ASSERT(typeid(tracer) == typeid(*this));
	return result;
}



void RayTracer::seed(num::Tuint32 seed)
{
	doSeed(seed);
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



const Spectral RayTracer::estimateLightContribution(
		const Sample& sample, const TBsdfPtr& bsdf, const LightContext& light,
		const Sample::TSubSequence2D& lightSamples, const Sample::TSubSequence2D& bsdfSamples, const Sample::TSubSequence1D& componentSamples,
		const TPoint3D& target, const TVector3D& targetNormal, const TVector3D& omegaIn) const
{
	LASS_ASSERT(bsdfSamples.size() == componentSamples.size());

	const bool isShadowOnly = bsdf->context().shader()->subtractiveHack();
	const TScalar sign = isShadowOnly ? -1 : 1;

	const bool supportsBsdfSampling = !light.isSingular();
	const TScalar nl = static_cast<TScalar>(lightSamples.size());
	const TScalar nb = supportsBsdfSampling ? static_cast<TScalar>(bsdfSamples.size()) : 0;

	BsdfCaps caps = BsdfCaps::allDiffuse | BsdfCaps::glossy;
	if (!light.isSingular())
	{
		caps |= BsdfCaps::specular;
	}
	// what about indirect estimator caps???

	Spectral result;
	const TPoint3D start = target + 10 * liar::tolerance * targetNormal;

	if (nl > 0)
	{
		for (Sample::TSubSequence2D::iterator ls = lightSamples.begin(); ls != lightSamples.end(); ++ls)
		{
			BoundedRay shadowRay;
			TScalar lightPdf;
			const Spectral radiance = light.sampleEmission(sample, *ls, start, targetNormal, shadowRay, lightPdf);
			if (lightPdf <= 0 || !radiance)
			{
				continue;
			}
			const TVector3D omegaOut = bsdf->worldToBsdf(shadowRay.direction());
			const BsdfOut out = bsdf->evaluate(omegaIn, omegaOut, caps);
			if (!out || scene()->isIntersecting(sample, shadowRay) != isShadowOnly)
			{
				continue;
			}
			const Spectral trans = mediumStack().transmittance(sample, shadowRay);
			const TScalar weight = temp::squaredHeuristic(nl * lightPdf, nb * out.pdf);
			const TScalar cosTheta = omegaOut.z;
			result += out.value * trans * radiance * static_cast<Spectral::TValue>(sign * weight * num::abs(cosTheta) / (nl * lightPdf));
		}
	}

	if (nb > 0)
	{
		Sample::TSubSequence1D::iterator cs = componentSamples.begin();
		for (Sample::TSubSequence2D::iterator bs = bsdfSamples.begin(); bs != bsdfSamples.end(); ++bs, ++cs)
		{
			const SampleBsdfOut out = bsdf->sample(omegaIn, *bs, *cs, caps);
			if (!out)
			{
				continue;
			}
			const TRay3D ray(start, bsdf->bsdfToWorld(out.omegaOut));
			BoundedRay shadowRay;
			TScalar lightPdf;
			const Spectral radiance = light.emission(sample, ray, shadowRay, lightPdf);
			if (lightPdf <= 0 || !radiance)
			{
				continue;
			}
			if (scene()->isIntersecting(sample, shadowRay) != isShadowOnly)
			{
				continue;
			}
			const Spectral trans = mediumStack().transmittance(sample, shadowRay);
			const TScalar weight = !hasCaps(out.usedCaps, BsdfCaps::specular)
				? temp::squaredHeuristic(nb * out.pdf, nl * lightPdf)
				: 1;
			const TScalar cosTheta = out.omegaOut.z;
			result += out.value * trans * radiance * static_cast<Spectral::TValue>(sign * weight * num::abs(cosTheta) / (nb * out.pdf));
		}
	}

	return result;
}


/** Helper function for some RayTracer implementations to request
 *  samples for lights and shaders. This is mostly for the DirectLighting one.
 */
void RayTracer::requestLightAndSceneSamples(const TSamplerPtr& sampler)
{
	lights_.requestSamples(sampler);
	forAllObjects(scene(), impl::requestMemberSamples<Shader>(sampler, &SceneObject::shader));
	forAllObjects(scene(), impl::requestMemberSamples<Medium>(sampler, &SceneObject::interior));
}

// --- private -------------------------------------------------------------------------------------



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
