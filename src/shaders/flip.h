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

/** @class liar::shaders::FlipHemisphere
 *  @brief flips
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SHADERS_FLIP_HEMISPHERE_H
#define LIAR_GUARDIAN_OF_INCLUSION_SHADERS_FLIP_HEMISPHERE_H

#include "shaders_common.h"
#include "../kernel/shader.h"
#include "../kernel/texture.h"

namespace liar
{
namespace shaders
{

class LIAR_SHADERS_DLL Flip: public Shader
{
	PY_HEADER(Shader)
public:

	Flip(const TShaderPtr& child);

	const TShaderPtr& child() const;
	void setChild(const TShaderPtr& child);

private:

	class Bsdf: public kernel::Bsdf
	{
	public:
		Bsdf(const Sample& sample, const IntersectionContext& context, BsdfCaps caps, TBsdfPtr&& child);
		static BsdfCaps flip(BsdfCaps caps);
		static TVector3D flip(const TVector3D& omega);
	private:
		BsdfOut doEvaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, BsdfCaps allowedCaps) const override;
		SampleBsdfOut doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, BsdfCaps allowedCaps) const override;
		TBsdfPtr child_;
	};

	void doShadeContext(const Sample& sample, IntersectionContext& context) const override;
	const Spectral doEmission(const Sample& sample, const IntersectionContext& context, const TVector3D& omegaOut) const override;
	TBsdfPtr doBsdf(const Sample& sample, const IntersectionContext& context) const override;

	void doRequestSamples(const TSamplerPtr& sampler) override;
	size_t doNumReflectionSamples() const override;
	size_t doNumTransmissionSamples() const override;

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

	TShaderPtr child_;
};

}

}

#endif

// EOF
