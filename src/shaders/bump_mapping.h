/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
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
 *  http://liar.sourceforge.net
 */

/** @class liar::shaders::BumpMapping
 *  @brief null shader simply gives full transmission in straight direction ...
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SHADERS_BUMP_MAPPING_H
#define LIAR_GUARDIAN_OF_INCLUSION_SHADERS_BUMP_MAPPING_H

#include "shaders_common.h"
#include "../kernel/shader.h"
#include "../kernel/texture.h"

namespace liar
{
namespace shaders
{

class LIAR_SHADERS_DLL BumpMapping: public Shader
{
    PY_HEADER(Shader)
public:

	BumpMapping(const TShaderPtr& shader, const TTexturePtr& displacement);

	const TShaderPtr& shader() const;
	void setShader(const TShaderPtr& shader);

	const TTexturePtr& displacement() const;
	void setDisplacement(const TTexturePtr& displacement);

private:

	void doRequestSamples(const TSamplerPtr& sampler);
	const unsigned doNumReflectionSamples() const;
	const unsigned doNumTransmissionSamples() const;

	void doShadeContext(const Sample& sample, IntersectionContext& context) const;
	const Spectrum doEmission(const Sample& sample, const IntersectionContext& context,
		const TVector3D& omegaOut) const;
	void doBsdf(const Sample& sample, const IntersectionContext& context, const TVector3D& omegaIn,
		const BsdfIn* first, const BsdfIn* last, BsdfOut* result) const;
	void doSampleBsdf(const Sample& sample, const IntersectionContext& context,	const TVector3D& omegaIn,
		const SampleBsdfIn* first, const SampleBsdfIn* last, SampleBsdfOut* result) const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	const TVector2D dDisplacement_dUv(const Sample& sample, IntersectionContext& context) const;

	TShaderPtr shader_;
	TTexturePtr displacement_;
};

}

}

#endif

// EOF
