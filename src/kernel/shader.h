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
 *
 *  http://liar.sourceforge.net
 */

/** @class liar::Shader
 *  @brief base class of all shaders
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SHADER_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SHADER_H

#include "kernel_common.h"
#include "spectrum.h"
#include <lass/util/small_object.h>

namespace liar
{
namespace kernel
{

class DifferentialRay;
class Intersection;
class IntersectionContext;
class RayTracer;
class Sample;
class Sampler;
class SceneObject;
class LightContext;

typedef python::PyObjectPtr<Sampler>::Type TSamplerPtr;
typedef python::PyObjectPtr<SceneObject>::Type TSceneObjectPtr;

class LIAR_KERNEL_DLL Shader: public python::PyObjectPlus
{
    PY_HEADER(python::PyObjectPlus)
public:

	enum CapsFlags
	{
		capsReflection = 0x01,
		capsTransmission = 0x02,
		capsDiffuse = 0x04,
		capsSpecular = 0x08,
	};

    virtual ~Shader();

	const unsigned caps() const { return caps_; }
	const bool hasCaps(unsigned wantedCaps) const { return (caps_ & wantedCaps) == wantedCaps; }

	const Spectrum emission(const Sample& sample, const IntersectionContext& context, 
		const TVector3D& omegaOut) const
	{
		return doEmission(sample, context, omegaOut);
	}

	void bsdf(const Sample& sample, const IntersectionContext& context, const TVector3D& omegaOut,
		const TVector3D* firstOmegaIn, const TVector3D* lastOmegaIn, 
		Spectrum* firstValue, TScalar* firstPdf) const
	{
		return doBsdf(sample, context, omegaOut, firstOmegaIn, lastOmegaIn, 
			firstValue, firstPdf);
	}

	void sampleBsdf(const Sample& sample, const IntersectionContext& context, const TVector3D& omegaOut,
		const TPoint2D* firstBsdfSample, const TPoint2D* lastBsdfSample,
		TVector3D* firstOmegaIn, Spectrum* firstValue, TScalar* firstPdf) const
	{
		return doSampleBsdf(sample, context, omegaOut, firstBsdfSample, lastBsdfSample, 
			firstOmegaIn, firstValue, firstPdf);
	}

	void requestSamples(const TSamplerPtr& sampler);

	const TPyObjectPtr reduce() const;
	const TPyObjectPtr getState() const;
	void setState(const TPyObjectPtr& state);

protected:

    Shader(unsigned capabilityFlags);

private:

	virtual const Spectrum doEmission(const Sample& sample, const IntersectionContext& context,
		const TVector3D& omegaOut) const;

	virtual void doBsdf(const Sample& sample, const IntersectionContext& context, const TVector3D& omegaOut,
		const TVector3D* firstOmegaIn, const TVector3D* lastOmegaIn, 
		Spectrum* firstValue, TScalar* firstPdf) const = 0;

	virtual void doSampleBsdf(const Sample& sample, const IntersectionContext& context, const TVector3D& omegaOut,
		const TPoint2D* firstBsdfSample, const TPoint2D* lastBsdfSample,
		TVector3D* firstOmegaIn, Spectrum* firstValue, TScalar* firstPdf) const = 0;
	
	virtual void doRequestSamples(const TSamplerPtr& sampler);

	virtual const TPyObjectPtr doGetState() const = 0;
	virtual void doSetState(const TPyObjectPtr& state) = 0;

	unsigned caps_;
};

typedef python::PyObjectPtr<Shader>::Type TShaderPtr;

}

}

#endif

// EOF

