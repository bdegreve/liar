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

	typedef std::pair<TScalar, TShaderPtr> TKeyShader;
	typedef std::vector<TKeyShader> TKeyShaders;

	LinearInterpolator();
	LinearInterpolator(const TKeyShaders& keyShaders, const TTexturePtr& controlTexture);

	const TKeyShaders& keys() const;
	const TTexturePtr& control() const;

	void setKeys(const TKeyShaders& keyShaders);
	void setControl(const TTexturePtr& controlTexture);

	void addKey(TScalar keyValue, const TShaderPtr& keyShader);

private:

	struct LesserKey
	{
		bool operator()(const TKeyShader& a, const TKeyShader& b) const { return a.first < b.first; }
	};

	class Bsdf: public kernel::Bsdf
	{
	public:
		Bsdf(const Sample& sample, const IntersectionContext& context, TBsdfCaps caps, const TBsdfPtr& a, const TBsdfPtr& b, TScalar t);
	private:
		BsdfOut doCall(const TVector3D& omegaIn, const TVector3D& omegaOut, TBsdfCaps allowedCaps) const;
		SampleBsdfOut doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, TBsdfCaps allowedCaps) const;
		TBsdfPtr a_;
		TBsdfPtr b_;
		TScalar t_;
	};

	const XYZ doEmission(const Sample& sample, const IntersectionContext& context, const TVector3D& omegaOut) const;
	TBsdfPtr doBsdf(const Sample& sample, const IntersectionContext& context) const;

	void doRequestSamples(const TSamplerPtr& sampler);
	size_t doNumReflectionSamples() const;
	size_t doNumTransmissionSamples() const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	TKeyShaders keys_;
	TTexturePtr control_;
};

}

}

#endif

// EOF
