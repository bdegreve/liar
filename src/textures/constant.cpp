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

#include "textures_common.h"
#include "constant.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(Constant, "texture with constant value")
PY_CLASS_CONSTRUCTOR_1(Constant, TScalar);
PY_CLASS_CONSTRUCTOR_1(Constant, const XYZ&);
PY_CLASS_CONSTRUCTOR_1(Constant, const TSpectrumPtr&);
PY_CLASS_CONVERTOR(Constant, TScalar);
PY_CLASS_CONVERTOR(Constant, XYZ);
PY_CLASS_CONVERTOR(Constant, TSpectrumPtr);
PY_CLASS_MEMBER_RW(Constant, value, setValue);

// --- public --------------------------------------------------------------------------------------

Constant::Constant(const TSpectrumPtr& spectrum) :
	value_(spectrum)
{
}



Constant::Constant(const XYZ& value) :
	value_(Spectrum::make(value))
{
}



Constant::Constant(TScalar value):
	value_(Spectrum::make(value))
{
}



const TSpectrumPtr& Constant::value() const
{
	return value_;
}



void Constant::setValue(const TSpectrumPtr& value)
{
	value_ = value;
}



// --- protected -----------------------------------------------------------------------------------

const TPyObjectPtr Constant::doGetState() const
{
	return python::makeTuple(value_);
}



void Constant::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, value_);
}



// --- private -------------------------------------------------------------------------------------

const Spectral Constant::doLookUp(const Sample& sample, const IntersectionContext&) const
{
	return value_->evaluate(sample, Reflectant);
}



TScalar Constant::doScalarLookUp(const Sample&, const IntersectionContext&) const
{
	return value_->luminance();
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

