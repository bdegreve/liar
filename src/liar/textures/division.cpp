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
#include "division.h"
#include <lass/stde/extended_string.h>

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(Division, "a / b")
PY_CLASS_CONSTRUCTOR_2(Division, const TTextureRef&, const TTextureRef&);

// --- public --------------------------------------------------------------------------------------

Division::Division(const TTextureRef& a, const TTextureRef& b):
	BinaryOperator(a, b)
{
}



// --- private -------------------------------------------------------------------------------------

const Spectral Division::doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const
{
	return Spectral(textureA()->lookUp(sample, context, SpectralType::Illuminant) / textureB()->lookUp(sample, context, SpectralType::Illuminant), type);
}


Texture::TValue Division::doScalarLookUp(const Sample& sample, const IntersectionContext& context) const
{
	return textureA()->scalarLookUp(sample, context) / textureB()->scalarLookUp(sample, context);
}


// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
