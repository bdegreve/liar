/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
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
#include <lass/util/bit_manip.h>

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

struct BsdfIn
{
	BsdfIn(): omegaOut(), allowedCaps() {}
	BsdfIn(const TVector3D& omegaOut, unsigned allowedCaps): 
		omegaOut(omegaOut), allowedCaps(allowedCaps) {}
	TVector3D omegaOut;
	unsigned allowedCaps;
};

struct BsdfOut
{
	Spectrum value;
	TScalar pdf;
};

struct SampleBsdfIn
{
	SampleBsdfIn(): sample(), allowedCaps() {}
	SampleBsdfIn(const TPoint2D& sample, unsigned allowedCaps): sample(sample), allowedCaps(allowedCaps) {}
	TPoint2D sample;
	unsigned allowedCaps;
};

struct SampleBsdfOut
{
	TVector3D omegaOut;
	Spectrum value;
	TScalar pdf;
	unsigned usedCaps;
};

class LIAR_KERNEL_DLL Shader: public python::PyObjectPlus
{
    PY_HEADER(python::PyObjectPlus)
public:

	enum CapsFlags
	{
		capsNone = 0x00,
		capsEmission = 0x01,
		capsReflection = 0x02,
		capsTransmission = 0x04,
		capsDiffuse = 0x08,
		capsSpecular = 0x10,
		capsGlossy = 0x20,
		capsAll = 0xff,

		capsNonDiffuse = capsGlossy | capsSpecular,
		capsAllReflection = capsReflection | capsDiffuse | capsNonDiffuse,
		capsAllTransmission = capsTransmission | capsDiffuse | capsNonDiffuse,
		capsAllDiffuse = capsReflection | capsTransmission | capsDiffuse,
		capsAllSpecular = capsReflection | capsTransmission | capsSpecular,
		capsAllGlossy = capsReflection | capsTransmission | capsGlossy,
		capsAllNonDiffuse = capsReflection | capsTransmission | capsNonDiffuse,
	};

    virtual ~Shader();

	const unsigned caps() const { return caps_; }
	const bool hasCaps(unsigned wantedCaps) const { return testCaps(caps_, wantedCaps); }

	void shadeContext(const Sample& sample, IntersectionContext& context) const
	{
		doShadeContext(sample, context);
	}

	const Spectrum emission(const Sample& sample, const IntersectionContext& context, 
		const TVector3D& omegaOut) const
	{
		return doEmission(sample, context, omegaOut);
	}

	void bsdf(const Sample& sample, const IntersectionContext& context, const TVector3D& omegaIn,
		const BsdfIn* first, const BsdfIn* last, BsdfOut* result) const
	{
		LASS_ASSERT(omegaIn.z >= 0);
		zeroBsdf(result, result + (last - first));
		doBsdf(sample, context, omegaIn, first, last, result);
	}

	void sampleBsdf(const Sample& sample, const IntersectionContext& context, const TVector3D& omegaIn,
		const SampleBsdfIn* first, const SampleBsdfIn* last, SampleBsdfOut* result) const
	{
		LASS_ASSERT(omegaIn.z >= 0);
		zeroSampleBsdf(result, result + (last - first));
		doSampleBsdf(sample, context, omegaIn, first, last, result);
	}

	void requestSamples(const TSamplerPtr& sampler);
	const unsigned numReflectionSamples() const;
	const unsigned numTransmissionSamples() const;
	const int idReflectionSamples() const;
	const int idTransmissionSamples() const;

	const TPyObjectPtr reduce() const;
	const TPyObjectPtr getState() const;
	void setState(const TPyObjectPtr& state);

protected:

    Shader(unsigned capabilityFlags);

	void setCaps(unsigned capabilityFlags);
	const bool testCaps(unsigned capsUnderTest, unsigned wantedCaps) const
	{
		return (capsUnderTest & wantedCaps) == wantedCaps;
	}

private:

	virtual void doShadeContext(const Sample& sample, IntersectionContext& context) const;
	virtual const Spectrum doEmission(const Sample& sample, const IntersectionContext& context,
		const TVector3D& omegaOut) const;
	virtual void doBsdf(const Sample& sample, const IntersectionContext& context, const TVector3D& omegaIn,
		const BsdfIn* first, const BsdfIn* last, BsdfOut* result) const = 0;
	virtual void doSampleBsdf(const Sample& sample, const IntersectionContext& context,	const TVector3D& omegaIn,
		const SampleBsdfIn* first, const SampleBsdfIn* last, SampleBsdfOut* result) const = 0;
	
	virtual void doRequestSamples(const TSamplerPtr& sampler);
	virtual const unsigned doNumReflectionSamples() const;
	virtual const unsigned doNumTransmissionSamples() const;

	virtual const TPyObjectPtr doGetState() const = 0;
	virtual void doSetState(const TPyObjectPtr& state) = 0;

	void zeroBsdf(BsdfOut* first, BsdfOut* last) const;
	void zeroSampleBsdf(SampleBsdfOut* first, SampleBsdfOut* last) const;

	unsigned caps_;
	int idReflectionSamples_;
	int idTransmissionSamples_;
};

typedef python::PyObjectPtr<Shader>::Type TShaderPtr;

}

}

#endif

// EOF

