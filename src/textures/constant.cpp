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

#include "textures_common.h"
#include "constant.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS(Constant)
PY_CLASS_CONSTRUCTOR_1(Constant, TScalar);
PY_CLASS_CONSTRUCTOR_1(Constant, const Spectrum&);
//PY_CLASS_CONSTRUCTOR_2(Constant, const std::vector<TScalar>&, const TSpectrumFormatPtr&);
PY_CLASS_MEMBER_RW(Constant, "value", value, setValue);

// --- public --------------------------------------------------------------------------------------

Constant::Constant(const Spectrum& iSpectrum):
	Texture(&Type),
	value_(iSpectrum)
{
}



Constant::Constant(TScalar iValue):
	Texture(&Type),
	value_(iValue)
{
}


/*
Constant::Constant(const std::vector<TScalar>& iPowerDensities):
	Texture(&Type),
	value_(iPowerDensities, SpectrumFormat::defaultFormat())
{
}



Constant::Constant(const std::vector<TScalar>& iPowerDensities, 
				   const TSpectrumFormatPtr& iFormat):
	Texture(&Type),
	value_(iPowerDensities, iFormat)
{
}
*/



const Spectrum& Constant::value() const
{
	return value_;
}



void Constant::setValue(const Spectrum& iValue)
{
	value_ = iValue;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

Spectrum Constant::doLookUp(const Sample& iSample, 
									const IntersectionContext& iContext) const
{
	return value_;
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

