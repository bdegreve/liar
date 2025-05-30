/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2021-2025  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::shaders::Walter
 *  @brief Anistropic Microfacet Dielectric BSDF by Walter et al. (2007)
 *
 *  @par reference:
 *      @arg B. Walter, S. R. Marschner, H. Li, K. E. Torrance,
 *      <i>Microfacet Models for Refraction through Rough Surfaces</i>
 *      Proceedings of the 18th Eurographics conference on Rendering Techniques
 *      (EGSR'07). Eurographics Association, Goslar, DEU, 195–206 (2007)
 *
 *      @arg B. Burley, <i>Physically-Based Shading at Disney</i> (2012)
 *
 *      @arg E. Heitz, <i>Understanding the Masking-Shadowing Function in
 *      Microfacet-Based BRDFs</i>, Journal of Computer Graphics Techniques
 *      <b>3</b> (2), 48--107 (2014)
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

class LIAR_SHADERS_DLL Walter: public Shader
{
	PY_HEADER(Shader)
public:

	Walter();
	Walter(const TTextureRef& innerRefractionIndex);
	Walter(const TTextureRef& innerRefractionIndex, const TTextureRef& outerRefractionIndex);

	const TTextureRef& innerRefractionIndex() const;
	void setInnerRefractionIndex(const TTextureRef& refractionIndex);
	const TTextureRef& outerRefractionIndex() const;
	void setOuterRefractionIndex(const TTextureRef& refractionIndex);

	const TTextureRef& reflectance() const;
	void setReflectance(const TTextureRef& reflectance);
	const TTextureRef& transmittance() const;
	void setTransmittance(const TTextureRef& transmittance);

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
		Bsdf(
			const Sample& sample, const IntersectionContext& context, const Spectral& reflectance, const Spectral& transmittance, TValue etaI, TValue etaT,
			const MicrofacetDistribution* mdf, TValue alphaU, TValue alphaV);
	private:
		BsdfOut doEvaluate(const TVector3D& k1, const TVector3D& k2, BsdfCaps allowedCaps) const override;
		SampleBsdfOut doSample(const TVector3D& k1, const TPoint2D& sample, TScalar componentSample, BsdfCaps allowedCaps) const override;
		TValue pdfReflection(TValue rFresnel, BsdfCaps allowedCaps) const;
		Spectral reflectance_;
		Spectral transmittance_;
		const MicrofacetDistribution* mdf_;
		TValue etaI_;
		TValue etaT_;
		TValue alphaU_;
		TValue alphaV_;
	};

	size_t doNumReflectionSamples() const override;
	size_t doNumTransmissionSamples() const override;

	TBsdfPtr doBsdf(const Sample& sample, const IntersectionContext& context) const override;

	const TVector3D sampleH(const TPoint2D& sample, TScalar nu, TScalar nv/*, TScalar& pdf*/) const ;

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

	TTextureRef innerRefractionIndex_;
	TTextureRef outerRefractionIndex_;
	TTextureRef reflectance_;
	TTextureRef transmittance_;
	TTextureRef roughnessU_;
	TTextureRef roughnessV_;
	TMicrofacetDistributionRef mdf_;
	size_t numberOfSamples_;
};

}

}


// EOF
