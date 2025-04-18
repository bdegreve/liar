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
#include "../kernel/microfacet.h"

namespace liar
{
namespace shaders
{

class LIAR_SHADERS_DLL CookTorrance: public Shader
{
	PY_HEADER(Shader)
public:

	CookTorrance();
	CookTorrance(const TTextureRef& refractionIndex);
	CookTorrance(const TTextureRef& refractionIndex, const TTextureRef& absorptionCoefficient);

	const TTextureRef& refractionIndex() const;
	void setRefractionIndex(const TTextureRef& refractionIndex);
	const TTextureRef& absorptionCoefficient() const;
	void setAbsorptionCoefficient(const TTextureRef& absorptionCoefficient);

	const TTextureRef& reflectance() const;
	void setReflectance(const TTextureRef& reflectance);

	const TTextureRef& roughnessU() const;
	void setRoughnessU(const TTextureRef& roughness);

	const TTextureRef& roughnessV() const;
	void setRoughnessV(const TTextureRef& roughness);

	const TMicrofacetDistributionRef& mdf() const;
	void setMdf(const TMicrofacetDistributionRef& mdf);

	size_t numberOfSamples() const;
	void setNumberOfSamples(size_t number);

private:

	class Bsdf : public kernel::Bsdf
	{
	public:
		typedef Spectral::TValue TValue;
		Bsdf(const Sample& sample, const IntersectionContext& context, const Spectral& eta, const Spectral& kappa, const Spectral& reflectance,
			const MicrofacetDistribution* mdf, TValue alphaU, TValue alphaV);
	private:
		BsdfOut doEvaluate(const TVector3D& k1, const TVector3D& k2, BsdfCaps allowedCaps) const override;
		SampleBsdfOut doSample(const TVector3D& k1, const TPoint2D& sample, TScalar componentSample, BsdfCaps allowedCaps) const override;
		Spectral eta_;
		Spectral kappa_;
		Spectral reflectance_;
		const MicrofacetDistribution* mdf_;
		TValue alphaU_;
		TValue alphaV_;
	};

	size_t doNumReflectionSamples() const override;

	TBsdfPtr doBsdf(const Sample& sample, const IntersectionContext& context) const override;

	const TVector3D sampleH(const TPoint2D& sample, TScalar nu, TScalar nv/*, TScalar& pdf*/) const ;

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

	TTextureRef refractionIndex_;
	TTextureRef absorptionCoefficient_;
	TTextureRef reflectance_;
	TTextureRef roughnessU_;
	TTextureRef roughnessV_;
	TMicrofacetDistributionRef mdf_;
	size_t numberOfSamples_;
};

}

}


// EOF
