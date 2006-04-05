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
#include "shader.h"

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



void Shader::requestSamples(const TSamplerPtr& iSampler)
{
	doRequestSamples(iSampler);
}



// --- protected -----------------------------------------------------------------------------------

Shader::Shader(PyTypeObject* iType):
    python::PyObjectPlus(iType)
{
}



const TPyObjectPtr Shader::reduce() const
{
	return python::makeTuple(
		reinterpret_cast<PyObject*>(this->GetType()), python::makeTuple(), this->getState());
}



const TPyObjectPtr Shader::getState() const
{
	return doGetState();
}



void Shader::setState(const TPyObjectPtr& iState)
{
	doSetState(iState);
}



// --- private -------------------------------------------------------------------------------------

void Shader::doRequestSamples(const TSamplerPtr& iSampler)
{
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF