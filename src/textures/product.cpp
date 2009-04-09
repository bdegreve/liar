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

#include "textures_common.h"
#include "product.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(Product, "makes product of child textures")
PY_CLASS_CONSTRUCTOR_0(Product);
PY_CLASS_CONSTRUCTOR_1(Product, const Product::TFactors&)
PY_CLASS_MEMBER_RW(Product, factors, setFactors);

// --- public --------------------------------------------------------------------------------------

Product::Product():
	factors_()
{
}



Product::Product(const TFactors& factors):
	factors_(factors)
{
}

	

const Product::TFactors& Product::factors() const
{
	return factors_;
}



void Product::setFactors(const TFactors& factors)
{
	factors_ = factors;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const Spectrum Product::doLookUp(const Sample& sample, const IntersectionContext& context) const
{
	if (factors_.empty())
	{
		return Spectrum();
	}
	Spectrum result(TNumTraits::one);
	for (TFactors::const_iterator i = factors_.begin(); i != factors_.end(); ++i)
	{
		result *= (*i)->lookUp(sample, context);
	}
	return result;
}



const TPyObjectPtr Product::doGetState() const
{
	return python::makeTuple(factors_);
}



void Product::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, factors_);
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

