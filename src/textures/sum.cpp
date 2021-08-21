/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2021  Bram de Greve (bramz@users.sourceforge.net)
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
#include "sum.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(Sum, "makes sum of child textures")
PY_CLASS_CONSTRUCTOR_0(Sum);
PY_CLASS_CONSTRUCTOR_1(Sum, const Sum::TTerms&)
PY_CLASS_CONSTRUCTOR_2(Sum, const TTexturePtr&, const TTexturePtr&)
PY_CLASS_MEMBER_RW(Sum, terms, setTerms);

// --- public --------------------------------------------------------------------------------------

Sum::Sum():
	isChromatic_(false)
{
}



Sum::Sum(const TTerms& terms):
	isChromatic_(false)
{
	setTerms(terms);
}



Sum::Sum(const TTexturePtr& a, const TTexturePtr& b):
	isChromatic_(a->isChromatic() || b->isChromatic())
{
	terms_.push_back(a);
	terms_.push_back(b);
}



const Sum::TTerms& Sum::terms() const
{
	return terms_;
}



void Sum::setTerms(const TTerms& terms)
{
	terms_ = terms;

	isChromatic_ = false;
	for (TTerms::const_iterator i = terms_.begin(); i != terms_.end(); ++i)
	{
		isChromatic_ |= (*i)->isChromatic();
	}
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const Spectral Sum::doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const
{
	if (terms_.empty())
	{
		return Spectral();
	}
	Spectral result(0);
	for (TTerms::const_iterator i = terms_.begin(); i != terms_.end(); ++i)
	{
		result += (*i)->lookUp(sample, context, SpectralType::Illuminant);
	}
	return Spectral(result, type);
}



Texture::TValue Sum::doScalarLookUp(const Sample& sample, const IntersectionContext& context) const
{
	if (terms_.empty())
	{
		return 0;
	}
	TValue result = 0;
	for (TTerms::const_iterator i = terms_.begin(); i != terms_.end(); ++i)
	{
		result += (*i)->scalarLookUp(sample, context);
	}
	return result;
}



bool Sum::doIsChromatic() const
{
	return isChromatic_;
}


const TPyObjectPtr Sum::doGetState() const
{
	return python::makeTuple(terms_);
}



void Sum::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, terms_);
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

