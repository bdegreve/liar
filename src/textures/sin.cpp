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
#include "sin.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(Sin, "Sine value of texture")
PY_CLASS_CONSTRUCTOR_1(Sin, const TTexturePtr&);

// --- public --------------------------------------------------------------------------------------

Sin::Sin(const TTexturePtr& texture):
    UnaryOperator(texture)
{
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const Spectral Sin::doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType /*type*/) const
{
    return sin(texture()->lookUp(sample, context, SpectralType::Illuminant));
}


Texture::TValue Sin::doScalarLookUp(const Sample& sample, const IntersectionContext& context) const
{
    return num::sin(texture()->scalarLookUp(sample, context));
}


// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

