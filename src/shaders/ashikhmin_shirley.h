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

/** @class liar::shaders::AshikhminShirley
 *  @brief Anisotropic Phong BRDF model by Ashikhmin and Shirley (2001)
 *
 *  @par reference:
 *		@arg M. Ashikhmin, P. Shirley, <i>An Anisotropic Phong BRDF Model</i>,
 *		Journal of Graphics Tools, <b>5</b> (2), 25--32 (2001).
 *
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SHADERS_ASHIKHMIN_SHIRLEY_H
#define LIAR_GUARDIAN_OF_INCLUSION_SHADERS_ASHIKHMIN_SHIRLEY_H

#include "shaders_common.h"
#include "../kernel/shader.h"
#include "../kernel/texture.h"

namespace liar
{
namespace shaders
{

class LIAR_SHADERS_DLL AshikhminShirley: public Shader
{
	PY_HEADER(Shader)
public:

	AshikhminShirley();
	AshikhminShirley(const TTexturePtr& diffuse, const TTexturePtr& specular);

	const TTexturePtr& diffuse() const;
	void setDiffuse(const TTexturePtr& diffuse);

	const TTexturePtr& specular() const;
	void setSpecular(const TTexturePtr& specular);

	const TTexturePtr& roughnessU() const;
	void setRoughnessU(const TTexturePtr& roughness);

	const TTexturePtr& roughnessV() const;
	void setRoughnessV(const TTexturePtr& roughness);

	const TTexturePtr& specularPowerU() const;
	void setSpecularPowerU(const TTexturePtr& specularPower);

	const TTexturePtr& specularPowerV() const;
	void setSpecularPowerV(const TTexturePtr& specularPower);

	size_t numberOfSamples() const;
	void setNumberOfSamples(size_t number);

	class Bsdf: public kernel::Bsdf
	{
	public:
		Bsdf(const Sample& sample, const IntersectionContext& context, const Spectral& diffuse, const Spectral& specular, TScalar powerU, TScalar powerV);
	private:
		BsdfOut doEvaluate(const TVector3D& k1, const TVector3D& k2, BsdfCaps allowedCaps) const;
		SampleBsdfOut doSample(const TVector3D& k1, const TPoint2D& sample, TScalar componentSample, BsdfCaps allowedCaps) const;
		const Spectral rhoD(const TVector3D& k1, const TVector3D& k2) const;
		const Spectral rhoS(const TVector3D& k1, const TVector3D& k2, const TVector3D& h, TScalar& pdf) const;
		const TVector3D sampleH(const TPoint2D& sample) const;
		Spectral diffuse_;
		Spectral specular_;
		TScalar powerU_;
		TScalar powerV_;
	};

	class PowerFromRoughness: public Texture
	{
		PY_HEADER(Texture)
	public:
		PowerFromRoughness(const TTexturePtr& roughness);
		const TTexturePtr& roughness() const;
		void setRoughness(const TTexturePtr& roughness);
	protected:
		const TPyObjectPtr doGetState() const override;
		void doSetState(const TPyObjectPtr& state) override;
	private:
		const Spectral doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const override;
		TValue doScalarLookUp(const Sample& sample, const IntersectionContext& context) const override;
		bool doIsChromatic() const override;
		TTexturePtr roughness_;
	};

	class RoughnessFromPower: public Texture
	{
		PY_HEADER(Texture)
	public:
		RoughnessFromPower(const TTexturePtr& power);
		const TTexturePtr& power() const;
		void setPower(const TTexturePtr& power);
	protected:
		const TPyObjectPtr doGetState() const override;
		void doSetState(const TPyObjectPtr& state) override;
	private:
		const Spectral doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const override;
		TValue doScalarLookUp(const Sample& sample, const IntersectionContext& context) const override;
		bool doIsChromatic() const override;
		TTexturePtr power_;
	};

private:

	size_t doNumReflectionSamples() const;

	TBsdfPtr doBsdf(const Sample& sample, const IntersectionContext& context) const;

	const TVector3D sampleH(const TPoint2D& sample, TScalar nu, TScalar nv/*, TScalar& pdf*/) const ;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	TTexturePtr diffuse_;
	TTexturePtr specular_;
	TTexturePtr roughnessU_;
	TTexturePtr roughnessV_;
	TTexturePtr specularPowerU_;
	TTexturePtr specularPowerV_;
	size_t numberOfSamples_;
};

}

}

#endif

// EOF
