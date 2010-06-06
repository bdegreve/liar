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
#include "uv.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(Uv, "mixes two textures by the u and v context channels")
PY_CLASS_CONSTRUCTOR_2(Uv, TTexturePtr, TTexturePtr);

// --- public --------------------------------------------------------------------------------------

Uv::Uv(const TTexturePtr& a, const TTexturePtr& b):
	Mix2(a, b)
{
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const XYZ Uv::doLookUp(const Sample& sample, const IntersectionContext& context) const
{
	const TScalar u = num::mod(context.uv().x, TNumTraits::one);
	const TScalar v = num::mod(context.uv().y, TNumTraits::one);
	return u * textureA()->lookUp(sample, context) + v * textureB()->lookUp(sample, context);	
}



const TPyObjectPtr Uv::doGetMixState() const
{
	return python::makeTuple();
}



void Uv::doSetMixState(const TPyObjectPtr&)
{
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

