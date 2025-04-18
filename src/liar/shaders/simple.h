/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2024  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::shaders::Lambert
 *  @brief a simple plain old raytracing shader
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SHADERS_SIMPLE_H
#define LIAR_GUARDIAN_OF_INCLUSION_SHADERS_SIMPLE_H

#include "shaders_common.h"
#include "../kernel/shader.h"
#include "../kernel/texture.h"

namespace liar
{
namespace shaders
{

class LIAR_SHADERS_DLL Simple: public Shader
{
    PY_HEADER(Shader)
public:

	Simple();
	Simple(const TTexturePtr& iDiffuse);
	Simple(const TTexturePtr& iDiffuse, const TTexturePtr& iSpecular);

	const TTexturePtr& diffuse() const;
	void setDiffuse(const TTexturePtr& iDiffuse);

	const TTexturePtr& specular() const;
	void setSpecular(const TTexturePtr& iSpecular);

	const TTexturePtr& specularPower() const;
	void setSpecularPower(const TTexturePtr& iSpecularPower);

	const TTexturePtr& reflectance() const;
	void setReflectance(const TTexturePtr& iReflective);

	const TTexturePtr& transmittance() const;
	void setTransmittance(const TTexturePtr& iTransmittance);

	const TTexturePtr& refractionIndex() const;
	void setRefractionIndex(const TTexturePtr& iRefractionIndex);

private:

	class SimpleBsdf : public Bsdf
	{
	public:
		SimpleBsdf(const Sample& sample, const IntersectionContext& context, BsdfCaps caps, const Spectral& diffuse,
			const Spectral& specular, const Spectral& specularPower, const Spectral& reflectance, const Spectral& transmittance,
			const Spectral& refractionIndex);
	private:
		BsdfOut doEvaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, BsdfCaps allowedCaps) const override;
		SampleBsdfOut doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, BsdfCaps allowedCaps) const override;
		Spectral diffuse_;
		Spectral specular_;
		Spectral specularPower_;
		Spectral reflectance_;
		Spectral transmittance_;
		Spectral refractionIndex_;
	};

	TBsdfPtr doBsdf(const Sample& sample, const IntersectionContext& context) const override;

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

	TTexturePtr diffuse_;
	TTexturePtr specular_;
	TTexturePtr specularPower_;
	TTexturePtr reflectance_;
	TTexturePtr transmittance_;
	TTexturePtr refractionIndex_;
};

}

}

#endif

// EOF
