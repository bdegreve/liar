/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2025  Bram de Greve (bramz@users.sourceforge.net)
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
PY_CLASS_CONSTRUCTOR_2(Product, const TTextureRef&, const TTextureRef&)
PY_CLASS_MEMBER_RW(Product, factors, setFactors);

// --- public --------------------------------------------------------------------------------------

Product::Product():
	isChromatic_(false)
{
}



Product::Product(const TFactors& factors):
	isChromatic_(false)
{
	setFactors(factors);
}



Product::Product(const TTextureRef& a, const TTextureRef& b):
	isChromatic_(a->isChromatic() || b->isChromatic())
{
	factors_.push_back(a);
	factors_.push_back(b);
}



const Product::TFactors& Product::factors() const
{
	return factors_;
}



void Product::setFactors(const TFactors& factors)
{
	factors_ = factors;

	isChromatic_ = false;
	for (TFactors::const_iterator i = factors_.begin(); i != factors_.end(); ++i)
	{
		isChromatic_ |= (*i)->isChromatic();
	}
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const Spectral Product::doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const
{
	Spectral result(1);
	for (TFactors::const_iterator i = factors_.begin(); i != factors_.end(); ++i)
	{
		result *= (*i)->lookUp(sample, context, SpectralType::Illuminant);
	}
	return Spectral(result, type);
}



Texture::TValue Product::doScalarLookUp(const Sample& sample, const IntersectionContext& context) const
{
	TValue result = 1;
	for (TFactors::const_iterator i = factors_.begin(); i != factors_.end(); ++i)
	{
		result *= (*i)->scalarLookUp(sample, context);
	}
	return result;
}



bool Product::doIsChromatic() const
{
	return isChromatic_;
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
