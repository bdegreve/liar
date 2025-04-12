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

/** @class liar::shaders::LinearInterpolator
 *  @brief interpolates between shaders based on gray value of control texture
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SHADERS_LINEAR_INTERPOLATOR_H
#define LIAR_GUARDIAN_OF_INCLUSION_SHADERS_LINEAR_INTERPOLATOR_H

#include "shaders_common.h"
#include "../kernel/shader.h"
#include "../kernel/texture.h"

namespace liar
{
namespace shaders
{

class LIAR_SHADERS_DLL LinearInterpolator: public Shader
{
	PY_HEADER(Shader)
public:

	typedef Spectral::TValue TValue;
	typedef std::pair<TValue, TShaderRef> TKeyShader;
	typedef std::vector<TKeyShader> TKeyShaders;

	LinearInterpolator();
	LinearInterpolator(const TKeyShaders& keyShaders, const TTextureRef& controlTexture);

	const TKeyShaders& keys() const;
	const TTextureRef& control() const;

	void setKeys(const TKeyShaders& keyShaders);
	void setControl(const TTextureRef& controlTexture);

	void addKey(TValue keyValue, const TShaderRef& keyShader);

private:

	struct LesserKey
	{
		bool operator()(const TKeyShader& a, const TKeyShader& b) const { return a.first < b.first; }
	};

	class Bsdf: public kernel::Bsdf
	{
	public:
		Bsdf(const Sample& sample, const IntersectionContext& context, BsdfCaps caps, const IntersectionContext& contextA, const IntersectionContext& contextB, TValue t);
	private:
		BsdfOut doEvaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, BsdfCaps allowedCaps) const override;
		SampleBsdfOut doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, BsdfCaps allowedCaps) const override;
		IntersectionContext a_;
		IntersectionContext b_;
		TValue t_;
	};

	static_assert(sizeof(Bsdf) <= 4096, "Bsdf must fit in freelist");
	static_assert(alignof(Bsdf) >= alignof(IntersectionContext));

	const Spectral doEmission(const Sample& sample, const IntersectionContext& context, const TVector3D& omegaOut) const override;
	TBsdfPtr doBsdf(const Sample& sample, const IntersectionContext& context) const override;

	void doRequestSamples(const TSamplerPtr& sampler) override;
	size_t doNumReflectionSamples() const override;
	size_t doNumTransmissionSamples() const override;

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

	TKeyShaders keys_;
	TTextureRef control_;
};

}

}

#endif

// EOF
