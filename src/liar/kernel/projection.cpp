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
#include "projection.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(Projection, "Abstract base class of projections")
PY_CLASS_METHOD_NAME(Projection, reduce, "__reduce__")
PY_CLASS_METHOD_NAME(Projection, getState, "__getstate__")
PY_CLASS_METHOD_NAME(Projection, setState, "__setstate__")

// --- public --------------------------------------------------------------------------------------

Projection::~Projection()
{
}



// --- protected -----------------------------------------------------------------------------------

Projection::Projection()
{
}



const TPyObjectPtr Projection::reduce() const
{
	return python::makeTuple(
		python::fromNakedToSharedPtrCast<PyObject>(reinterpret_cast<PyObject*>(this->_lassPyGetClassDef()->type())),
		python::makeTuple(), this->getState());
}



// --- private -------------------------------------------------------------------------------------



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
