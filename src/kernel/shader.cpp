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
#include "shader.h"
#include "sampler.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(Shader, "Abstract base class of shaders")
PY_CLASS_METHOD_NAME(Shader, reduce, "__reduce__")
PY_CLASS_METHOD_NAME(Shader, getState, "__getstate__")
PY_CLASS_METHOD_NAME(Shader, setState, "__setstate__")
PY_CLASS_MEMBER_RW(Shader, subtractiveHack, setSubtractiveHack)

// --- public --------------------------------------------------------------------------------------

Shader::~Shader()
{
}



void Shader::requestSamples(const TSamplerPtr& sampler)
{
	if (idReflectionSamples_ < 0)
	{
		idReflectionSamples_ = sampler->requestSubSequence2D(numReflectionSamples());
	}
	if (idReflectionComponentSamples_ < 0)
	{
		idReflectionComponentSamples_ = sampler->requestSubSequence1D(numReflectionSamples());
	}
	if (idTransmissionSamples_ < 0)
	{
		idTransmissionSamples_ = sampler->requestSubSequence2D(numTransmissionSamples());
	}
	if (idTransmissionComponentSamples_ < 0)
	{
		idTransmissionComponentSamples_ = sampler->requestSubSequence1D(numTransmissionSamples());
	}
	doRequestSamples(sampler);
}



size_t Shader::numReflectionSamples() const
{
	return doNumReflectionSamples();
}



size_t Shader::numTransmissionSamples() const
{
	return doNumTransmissionSamples();
}



int Shader::idReflectionSamples() const
{
	return idReflectionSamples_;
}



int Shader::idReflectionComponentSamples() const
{
	return idReflectionComponentSamples_;
}



int Shader::idTransmissionSamples() const
{
	return idTransmissionSamples_;
}



int Shader::idTransmissionComponentSamples() const
{
	return idTransmissionComponentSamples_;
}



// --- protected -----------------------------------------------------------------------------------

Shader::Shader(TBsdfCaps capabilityFlags):
	caps_(capabilityFlags),
	idReflectionSamples_(-1),
	idReflectionComponentSamples_(-1),
	idTransmissionSamples_(-1),
	idTransmissionComponentSamples_(-1),
	subtractiveHack_(false)
{
}



void Shader::setCaps(TBsdfCaps capabilityFlags)
{
	caps_ = capabilityFlags;
}



const TPyObjectPtr Shader::reduce() const
{
	return python::makeTuple(
		python::fromNakedToSharedPtrCast<PyObject>(reinterpret_cast<PyObject*>(this->_lassPyGetClassDef()->type())), 
		python::makeTuple(), this->getState());
}



const TPyObjectPtr Shader::getState() const
{
	return doGetState();
}



void Shader::setState(const TPyObjectPtr& state)
{
	doSetState(state);
}



// --- private -------------------------------------------------------------------------------------

void Shader::doRequestSamples(const TSamplerPtr&)
{
}



size_t Shader::doNumReflectionSamples() const
{
	return 0;
}



size_t Shader::doNumTransmissionSamples() const
{
	return 0;
}



void Shader::doShadeContext(const Sample&, IntersectionContext&) const
{
}



TBsdfPtr Shader::doBsdf(const Sample& sample, const IntersectionContext& context) const
{
	return TBsdfPtr();
}



const XYZ Shader::doEmission(const Sample&, const IntersectionContext&, const TVector3D&) const
{
	return XYZ();
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
