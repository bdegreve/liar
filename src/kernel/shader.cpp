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
#include "shader.h"
#include "sampler.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS(Shader)
PY_CLASS_METHOD_NAME(Shader, reduce, "__reduce__")
PY_CLASS_METHOD_NAME(Shader, getState, "__getstate__")
PY_CLASS_METHOD_NAME(Shader, setState, "__setstate__")

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
	if (idTransmissionSamples_ < 0)
	{
		idTransmissionSamples_ = sampler->requestSubSequence2D(numTransmissionSamples());
	}
	doRequestSamples(sampler);
}



const unsigned Shader::numReflectionSamples() const
{
	return doNumReflectionSamples();
}



const unsigned Shader::numTransmissionSamples() const
{
	return doNumTransmissionSamples();
}



const int Shader::idReflectionSamples() const
{
	return idReflectionSamples_;
}



const int Shader::idTransmissionSamples() const
{
	return idTransmissionSamples_;
}



// --- protected -----------------------------------------------------------------------------------

Shader::Shader(unsigned capabilityFlags):
	caps_(capabilityFlags),
	idReflectionSamples_(-1),
	idTransmissionSamples_(-1)
{
}



void Shader::setCaps(unsigned capabilityFlags)
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

void Shader::doRequestSamples(const TSamplerPtr& sampler)
{
}



const unsigned Shader::doNumReflectionSamples() const
{
	return 0;
}



const unsigned Shader::doNumTransmissionSamples() const
{
	return 0;
}



void Shader::doShadeContext(const Sample& sample, IntersectionContext& context) const
{
}



const Spectrum Shader::doEmission(const Sample& sample, const IntersectionContext& context, 
		const TVector3D& dirOut) const
{
	return Spectrum();
}



void Shader::zeroBsdf(BsdfOut* first, BsdfOut* last) const
{
	while (first != last)
	{
		first->value = Spectrum();
		first->pdf = 0;
		++first;
	}
}



void Shader::zeroSampleBsdf(SampleBsdfOut* first, SampleBsdfOut* last) const
{
	while (first != last)
	{
		first->omegaOut = TVector3D(0, 0, 1);
		first->value = Spectrum();
		first->pdf = 0;
		++first;
	}
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
