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

/** @class liar::shaders::CookTorrance
 *  @brief Specular Microfacet BRDF model by Cook and Torrance (1981)
 *
 *  @par reference:
 *      @arg R. L. Cook, K. E. Torrance, <i>A reflectance model for computer graphics</i>
 *      Computer Graphics (SIGGRAPH '81 Proceedings), <b>15</b>, 307--316 (1981)
 *
 *  @author Bram de Greve [Bramz]
 */

#pragma once

#include "shaders_common.h"
#include "../kernel/shader.h"
#include "../kernel/texture.h"

namespace liar
{
namespace shaders
{

class LIAR_SHADERS_DLL CookTorrance: public Shader
{
	PY_HEADER(Shader)
public:

	CookTorrance();
	CookTorrance(const TTexturePtr& refractionIndex);
	CookTorrance(const TTexturePtr& refractionIndex, const TTexturePtr& absorptionCoefficient);

	const TTexturePtr& refractionIndex() const;
	void setRefractionIndex(const TTexturePtr& refractionIndex);
	const TTexturePtr& absorptionCoefficient() const;
	void setAbsorptionCoefficient(const TTexturePtr& absorptionCoefficient);

	const TTexturePtr& reflectance() const;
	void setReflectance(const TTexturePtr& reflectance);

	const TTexturePtr& roughnessU() const;
	void setRoughnessU(const TTexturePtr& roughness);

	const TTexturePtr& roughnessV() const;
	void setRoughnessV(const TTexturePtr& roughness);

	size_t numberOfSamples() const;
	void setNumberOfSamples(size_t number);

	class Bsdf: public kernel::Bsdf
	{
	public:
		typedef Spectral::TValue TValue;
		Bsdf(const Sample& sample, const IntersectionContext& context, const Spectral& eta, const Spectral& kappa, const Spectral& reflectance, TValue mU, TValue mV);
	private:
		BsdfOut doEvaluate(const TVector3D& k1, const TVector3D& k2, TBsdfCaps allowedCaps) const;
		SampleBsdfOut doSample(const TVector3D& k1, const TPoint2D& sample, TScalar componentSample, TBsdfCaps allowedCaps) const;
		Spectral eta_;
		Spectral kappa_;
		Spectral reflectance_;
		TValue mU_;
		TValue mV_;
	};

private:

	size_t doNumReflectionSamples() const;

	TBsdfPtr doBsdf(const Sample& sample, const IntersectionContext& context) const;

	const TVector3D sampleH(const TPoint2D& sample, TScalar nu, TScalar nv/*, TScalar& pdf*/) const ;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	TTexturePtr refractionIndex_;
	TTexturePtr absorptionCoefficient_;
	TTexturePtr reflectance_;
	TTexturePtr roughnessU_;
	TTexturePtr roughnessV_;
	size_t numberOfSamples_;
};

}

}


// EOF
