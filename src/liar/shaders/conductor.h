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

/** @class liar::shaders::Lambert
*  @brief very simple shader using lambert's cosine law.
*  @author Bram de Greve [Bramz]
*/

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SHADERS_CONDUCTOR_H
#define LIAR_GUARDIAN_OF_INCLUSION_SHADERS_CONDUCTOR_H

#include "shaders_common.h"
#include "../kernel/shader.h"
#include "../kernel/texture.h"

namespace liar
{
namespace shaders
{

class LIAR_SHADERS_DLL Conductor : public Shader
{
	PY_HEADER(Shader)
public:

	Conductor();
	Conductor(const TTextureRef& refractionIndex);
	Conductor(const TTextureRef& refractionIndex, const TTextureRef& absorptionCoefficient);

	const TTextureRef& refractionIndex() const;
	void setRefractionIndex(const TTextureRef& refractionIndex);
	const TTextureRef& absorptionCoefficient() const;
	void setAbsorptionCoefficient(const TTextureRef& absorptionCoefficient);

	const TTextureRef& reflectance() const;
	void setReflectance(const TTextureRef& reflectance);

	class ConductorBsdf : public Bsdf
	{
	public:
		ConductorBsdf(const Sample& sample, const IntersectionContext& context, BsdfCaps caps, const Spectral& eta, const Spectral& kappa, const Spectral& reflectance);
	private:
		BsdfOut doEvaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, BsdfCaps allowedCaps) const override;
		SampleBsdfOut doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, BsdfCaps allowedCaps) const override;
		Spectral reflectance_;
		Spectral eta_;
		Spectral kappa_;
	};

private:

	size_t doNumReflectionSamples() const override;
	TBsdfPtr doBsdf(const Sample& sample, const IntersectionContext& context) const override;

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

	void init(
		const TTextureRef& refractionIndex = Texture::white(), const TTextureRef& absorptionCoefficient = Texture::white(),
		const TTextureRef& reflectance = Texture::white());

	TTextureRef refractionIndex_;
	TTextureRef absorptionCoefficient_;
	TTextureRef reflectance_;
};

}

}

#endif

// EOF
