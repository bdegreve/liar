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
#include "frequency.h"

namespace liar
{

namespace textures
{

PY_DECLARE_CLASS_DOC(Frequency, "evaluates to sampled electromagnetic frequency")
PY_CLASS_CONSTRUCTOR_0(Frequency);

// --- public --------------------------------------------------------------------------------------

Frequency::Frequency()
{
}



// --- protected -----------------------------------------------------------------------------------

const TPyObjectPtr Frequency::doGetState() const
{
	return python::makeTuple();
}



void Frequency::doSetState(const TPyObjectPtr&)
{
}



// --- private -------------------------------------------------------------------------------------

const Spectral Frequency::doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const
{
#pragma LASS_FIXME("what about type==Reflectant?")
	return Spectral(doScalarLookUp(sample, context), type);
}


Texture::TValue Frequency::doScalarLookUp(const Sample& sample, const IntersectionContext&) const
{
	const TWavelength c0 = static_cast<TWavelength>(299792458); // speed of light in vacuum
	return static_cast<TValue>(c0 / sample.wavelength());
}


bool Frequency::doIsChromatic() const
{
    // although the value of doLookup obviously depends on the sample's wavelength, it _is_ a flat spectrum.
    return false;
}


// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

