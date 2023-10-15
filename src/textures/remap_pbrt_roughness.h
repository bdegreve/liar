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

/** @class liar::textures::RemapPBRTRoughness
 *  @brief remap roughness from PBRT to LiAR
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  This class remaps roughness values from PBRT to LiAR.
 *
 *  PBRT uses the roughness values directly as the alpha parameter of the microfacet
 *  distribution, while LiAR uses the more common Disney and UE4 mapping: alpha = roughness^2
 *	So, we need to take the square root of the roughness value to convert from PBRT to LiAR.
 *
 *  If remaproughness is set to true, PBRT already applies another remapping to the
 *  roughness, which we need to concatenate. See tools/remap_pbrt_roughness.ipynb for the
 *  derivation of the polynomial fit.
 *
 *  @par reference:
 *      @arg Matt Pharr, Wenzel Jakob, and Greg Humphreys. 2016. <i>Physically Based Rendering:
 *      From Theory to Implementation (3rd ed.)</i>. Morgan Kaufmann Publishers Inc.,
 *      San Francisco, CA, USA.
 *
 *      @arg RoughnessToAlpha https://github.com/mmp/pbrt-v3/blob/13d871faae88233b327d04cda24022b8bb0093ee/src/core/microfacet.h#L83-L87
 *
 *      @arg B. Burley, <i>Physically-Based Shading at Disney</i> (2012)
 *
 *      @arg B. Karis, <i>Real Shading in Unreal Engine 4</i> (2013)
 *
 *      @arg B. Karis, <i>Specular BRDF Reference</i> (2013) http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_REMAP_PBRT_ROUGHNESS_H
#define LIAR_GUARDIAN_OF_INCLUSION_REMAP_PBRT_ROUGHNESS_H

#include "textures_common.h"
#include "unary_operator.h"

namespace liar
{
namespace textures
{

class LIAR_TEXTURES_DLL RemapPbrtRoughness: public UnaryOperator
{
	PY_HEADER(UnaryOperator)
public:

	RemapPbrtRoughness(const TTexturePtr& roughness, bool remapRoughness);

	bool remapRoughness() const;
	void setRemapRoughness(bool remapRoughness);

	static TValue remap(TValue roughness, bool remapRoughness);

private:

	const Spectral doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const override;
	TValue doScalarLookUp(const Sample& sample, const IntersectionContext& context) const override;

	bool remapRoughness_;
};

}

}

#endif

// EOF
