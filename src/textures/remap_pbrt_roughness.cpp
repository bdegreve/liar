/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2023  Bram de Greve (bramz@users.sourceforge.net)
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
#include "remap_pbrt_roughness.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(RemapPbrtRoughness, "Remap roughness from PBRT to LiAR")
PY_CLASS_CONSTRUCTOR_2(RemapPbrtRoughness, const TTexturePtr&, bool);
PY_CLASS_MEMBER_RW_DOC(RemapPbrtRoughness, remapRoughness, setRemapRoughness, "remap roughness from PBRT to LiAR");
PY_CLASS_STATIC_METHOD(RemapPbrtRoughness, remap)

// --- public --------------------------------------------------------------------------------------

RemapPbrtRoughness::RemapPbrtRoughness(const TTexturePtr& roughness, bool remapRoughness):
	UnaryOperator(roughness),
	remapRoughness_(remapRoughness)
{
}



bool RemapPbrtRoughness::remapRoughness() const
{
	return remapRoughness_;
}



void RemapPbrtRoughness::setRemapRoughness(bool remapRoughness)
{
	remapRoughness_ = remapRoughness;
}



Texture::TValue RemapPbrtRoughness::remap(TValue roughness, bool remapRoughness)
{
	if (remapRoughness)
	{
		constexpr TValue minRoughness = 1e-3f;
		const TValue x = num::log(std::max(roughness, minRoughness));
		return 1.2730328f + (0.31971025f + (0.02405594f + (-0.0018453393f + -0.00026452233f * x) * x) * x) * x;
	}
	else
	{
		return num::sqrt(roughness);
	}
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const Spectral RemapPbrtRoughness::doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const
{
	return texture()->lookUp(sample, context, SpectralType::Illuminant).map([this](TValue x) { return remap(x , remapRoughness_); }, type);
}


Texture::TValue RemapPbrtRoughness::doScalarLookUp(const Sample& sample, const IntersectionContext& context) const
{
	return remap(texture()->scalarLookUp(sample, context), remapRoughness_);
}


// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
