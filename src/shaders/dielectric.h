/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
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

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SHADERS_DIELECTRIC_H
#define LIAR_GUARDIAN_OF_INCLUSION_SHADERS_DIELECTRIC_H

#include "shaders_common.h"
#include "../kernel/shader.h"
#include "../kernel/texture.h"

namespace liar
{
namespace shaders
{

class LIAR_SHADERS_DLL Dielectric: public Shader
{
	PY_HEADER(Shader)
public:

	Dielectric();
	Dielectric(const TTexturePtr& innerRefractionIndex);
	Dielectric(const TTexturePtr& innerRefractionIndex, const TTexturePtr& outerRefractionIndex);

	const TTexturePtr& innerRefractionIndex() const;
	void setInnerRefractionIndex(const TTexturePtr& refractionIndex);
	const TTexturePtr& outerRefractionIndex() const;
	void setOuterRefractionIndex(const TTexturePtr& refractionIndex);

	const TTexturePtr& reflectance() const;
	void setReflectance(const TTexturePtr& reflectance);
	const TTexturePtr& transmittance() const;
	void setTransmittance(const TTexturePtr& transmittance);

	class DielectricBsdf: public Bsdf
	{
	public:
		DielectricBsdf(const Sample& sample, const IntersectionContext& context, TBsdfCaps caps, const TScalar ior, const XYZ& reflectance, const XYZ& transmittance);
	private:
		BsdfOut doEvaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, TBsdfCaps allowedCaps) const;
		SampleBsdfOut doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, TBsdfCaps allowedCaps) const;
		Spectral reflectance_;
		Spectral transmittance_;
		TScalar ior_;
	};

private:

	size_t doNumReflectionSamples() const;
	size_t doNumTransmissionSamples() const;
	TBsdfPtr doBsdf(const Sample& sample, const IntersectionContext& context) const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	void init(
		const TTexturePtr& innerRefractionIndex = Texture::white(), const TTexturePtr& outerRefractionIndex = Texture::white(),
		const TTexturePtr& reflectance = Texture::white(), const TTexturePtr& transmittance = Texture::white());

	TTexturePtr innerRefractionIndex_;
	TTexturePtr outerRefractionIndex_;
	TTexturePtr reflectance_;
	TTexturePtr transmittance_;
};

}

}

#endif

// EOF
