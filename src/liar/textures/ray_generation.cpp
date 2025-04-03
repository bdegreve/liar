/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2020  Bram de Greve (bramz@users.sourceforge.net)
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
#include "ray_generation.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(RayGeneration, "evaluates to intersection context's ray generation")
PY_CLASS_CONSTRUCTOR_0(RayGeneration);

// --- public --------------------------------------------------------------------------------------

RayGeneration::RayGeneration()
{
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const Spectral RayGeneration::doLookUp(const Sample&, const IntersectionContext& context, SpectralType type) const
{
	return Spectral(static_cast<TValue>(context.rayGeneration()), type);
}



Texture::TValue RayGeneration::doScalarLookUp(const Sample&, const IntersectionContext& context) const
{
	return static_cast<TValue>(context.rayGeneration());
}



bool RayGeneration::doIsChromatic() const
{
	return false;
}



const TPyObjectPtr RayGeneration::doGetState() const
{
	return python::makeTuple();
}



void RayGeneration::doSetState(const TPyObjectPtr&)
{
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
