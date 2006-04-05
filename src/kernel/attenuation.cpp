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
#include "attenuation.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS(Attenuation)
PY_CLASS_CONSTRUCTOR_0(Attenuation)
PY_CLASS_CONSTRUCTOR_3(Attenuation, TScalar, TScalar, TScalar)
PY_CLASS_MEMBER_RW(Attenuation, "constant", constant, setConstant)
PY_CLASS_MEMBER_RW(Attenuation, "linear", linear, setLinear)
PY_CLASS_MEMBER_RW(Attenuation, "quadratic", quadratic, setQuadratic)
PY_CLASS_METHOD_NAME(Attenuation, reduce, "__reduce__")
PY_CLASS_METHOD_NAME(Attenuation, getState, "__getstate__")
PY_CLASS_METHOD_NAME(Attenuation, setState, "__setstate__")

TAttenuationPtr Attenuation::defaultAttenuation_(new Attenuation);

// --- public --------------------------------------------------------------------------------------

Attenuation::Attenuation():
	python::PyObjectPlus(&Type),
	constant_(TNumTraits::zero),
	linear_(TNumTraits::zero),
	quadratic_(TNumTraits::one)
{
}



Attenuation::Attenuation(TScalar iConstant, TScalar iLinear, TScalar iQuadratic):
	python::PyObjectPlus(&Type),
	constant_(iConstant),
	linear_(iLinear),
	quadratic_(iQuadratic)
{
}



const TScalar Attenuation::constant() const
{
	return constant_;
}



const TScalar Attenuation::linear() const
{
	return linear_;
}



const TScalar Attenuation::quadratic() const
{
	return quadratic_;
}



void Attenuation::setConstant(TScalar iX)
{
	constant_ = iX;
}



void Attenuation::setLinear(TScalar iX)
{
	linear_ = iX;
}



void Attenuation::setQuadratic(TScalar iX)
{
	quadratic_ = iX;
}



TAttenuationPtr Attenuation::defaultAttenuation()
{
	return defaultAttenuation_;
}



const TPyObjectPtr Attenuation::reduce() const
{
	return python::makeTuple(
		reinterpret_cast<PyObject*>(this->GetType()), python::makeTuple(), this->getState());
}



const TPyObjectPtr Attenuation::getState() const
{
	return python::makeTuple(constant_, linear_, quadratic_);
}



void Attenuation::setState(const TPyObjectPtr& iState)
{
	python::decodeTuple(iState, constant_, linear_, quadratic_);
}



}

}

// EOF