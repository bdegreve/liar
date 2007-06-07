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

#include "kernel_common.h"
#include "attenuation.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS(Attenuation)
PY_CLASS_CONSTRUCTOR_0(Attenuation)
PY_CLASS_CONSTRUCTOR_3(Attenuation, TScalar, TScalar, TScalar)
PY_CLASS_MEMBER_RW(Attenuation, constant, setConstant)
PY_CLASS_MEMBER_RW(Attenuation, linear, setLinear)
PY_CLASS_MEMBER_RW(Attenuation, quadratic, setQuadratic)
PY_CLASS_METHOD_NAME(Attenuation, reduce, "__reduce__")
PY_CLASS_METHOD_NAME(Attenuation, getState, "__getstate__")
PY_CLASS_METHOD_NAME(Attenuation, setState, "__setstate__")

TAttenuationPtr Attenuation::defaultAttenuation_(new Attenuation);

// --- public --------------------------------------------------------------------------------------

/** Construct with default quadratic attenuation 1 / (2 * pi * r * r).
 */
Attenuation::Attenuation():
	constant_(TNumTraits::zero),
	linear_(TNumTraits::zero),
	quadratic_(2 * TNumTraits::pi)
{
}



/** Construct with custom attenuation.
 */
Attenuation::Attenuation(TScalar constant, TScalar linear, TScalar quadratic):
	constant_(constant),
	linear_(linear),
	quadratic_(quadratic)
{
}



/** return constant attenuation term.
 */
const TScalar Attenuation::constant() const
{
	return constant_;
}



/** return factor of linear attenuation term.
 */
const TScalar Attenuation::linear() const
{
	return linear_;
}



/** return factor of quadratic attenuation term.
 */
const TScalar Attenuation::quadratic() const
{
	return quadratic_;
}



/** set constant attenuation term
 */
void Attenuation::setConstant(TScalar constant)
{
	constant_ = constant;
}



/** set factor of linear attenuation term
 */
void Attenuation::setLinear(TScalar linear)
{
	linear_ = linear;
}



/** set factor of quadratic attenuation term
 */
void Attenuation::setQuadratic(TScalar quadratic)
{
	quadratic_ = quadratic;
}



/** return default attenuation object 1 / (2 * pi * r * r).
 */
TAttenuationPtr Attenuation::defaultAttenuation()
{
	return defaultAttenuation_;
}



const TPyObjectPtr Attenuation::reduce() const
{
	return python::makeTuple(
		python::fromNakedToSharedPtrCast<PyObject>(reinterpret_cast<PyObject*>(this->_lassPyGetType())), 
		python::makeTuple(), this->getState());
}



const TPyObjectPtr Attenuation::getState() const
{
	return python::makeTuple(constant_, linear_, quadratic_);
}



void Attenuation::setState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, constant_, linear_, quadratic_);
}



}

}

// EOF
