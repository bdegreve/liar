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

/** @class liar::Shader
 *  @brief base class of all shaders
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SHADER_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SHADER_H

#include "kernel_common.h"
#include "bsdf.h"
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

class LIAR_KERNEL_DLL Shader: public python::PyObjectPlus
{
	PY_HEADER(python::PyObjectPlus)
public:

	virtual ~Shader();

	TBsdfCaps caps() const { return caps_; }
	bool hasCaps(TBsdfCaps wantedCaps) const { return kernel::hasCaps(caps_, wantedCaps); }
	bool compatibleCaps(TBsdfCaps allowedCaps) const { return kernel::compatibleCaps(caps_, allowedCaps); }

	void shadeContext(const Sample& sample, IntersectionContext& context) const
	{
		doShadeContext(sample, context);
	}

	const Spectrum emission(const Sample& sample, const IntersectionContext& context, 
		const TVector3D& omegaOut) const
	{
		return doEmission(sample, context, omegaOut);
	}

	TBsdfPtr bsdf(const Sample& sample, const IntersectionContext& context) const
	{
		return doBsdf(sample, context);
	}

	void requestSamples(const TSamplerPtr& sampler);
	int idReflectionSamples() const;
	int idReflectionComponentSamples() const;
	int idTransmissionSamples() const;
	int idTransmissionComponentSamples() const;

	size_t numReflectionSamples() const;
	size_t numTransmissionSamples() const;

	bool subtractiveHack() const { return subtractiveHack_; }
	void setSubtractiveHack(bool enable) { subtractiveHack_ = enable; }

	const TPyObjectPtr reduce() const;
	const TPyObjectPtr getState() const;
	void setState(const TPyObjectPtr& state);

protected:

	Shader(TBsdfCaps capabilityFlags);

	void setCaps(TBsdfCaps capabilityFlags);

private:

	virtual void doShadeContext(const Sample& sample, IntersectionContext& context) const;
	virtual const Spectrum doEmission(const Sample& sample, const IntersectionContext& context,
		const TVector3D& omegaOut) const;
	virtual TBsdfPtr doBsdf(const Sample& sample, const IntersectionContext& context) const;
	
	virtual void doRequestSamples(const TSamplerPtr& sampler);
	virtual size_t doNumReflectionSamples() const;
	virtual size_t doNumTransmissionSamples() const;

	virtual const TPyObjectPtr doGetState() const = 0;
	virtual void doSetState(const TPyObjectPtr& state) = 0;

	TBsdfCaps caps_;
	int idReflectionSamples_;
	int idReflectionComponentSamples_;
	int idTransmissionSamples_;
	int idTransmissionComponentSamples_;
	bool subtractiveHack_;
};

typedef python::PyObjectPtr<Shader>::Type TShaderPtr;

}

}

#endif

// EOF

