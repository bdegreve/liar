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
#include "max.h"
#include <lass/stde/extended_string.h>

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(Max, "max(a, b)")
PY_CLASS_CONSTRUCTOR_2(Max, const TTexturePtr&, const TTexturePtr&);

// --- public --------------------------------------------------------------------------------------

Max::Max(const TTexturePtr& a, const TTexturePtr& b):
	BinaryOperator(a, b)
{
}



// --- private -------------------------------------------------------------------------------------

const Spectral Max::doLookUp(const Sample& sample, const IntersectionContext& context) const
{
	return max(textureA()->lookUp(sample, context), textureB()->lookUp(sample, context));
}


TScalar Max::doScalarLookUp(const Sample& sample, const IntersectionContext& context) const
{
	return std::max(textureA()->scalarLookUp(sample, context), textureB()->scalarLookUp(sample, context));
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

