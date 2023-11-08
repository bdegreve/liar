/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2023  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::shaders::OrenNayar
 *  @brief Diffuse Microfacet BRDF model by Oren and Nayar (1994)
 *  @author Bram de Greve [Bramz]
 *
 *  @par reference:
 *		@arg M. Oren and S. K. Nayar, <i>Generalization of Lambert's reflectance model</i>,
 *		SIGGRAPH '94: Proceedings of the 21st annual conference on Computer graphics and interactive techniques, 239--246 (1994)
 *		https://doi.org/10.1145/192161.192213
*/

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SHADERS_OREN_NAYAR_H
#define LIAR_GUARDIAN_OF_INCLUSION_SHADERS_OREN_NAYAR_H

#include "shaders_common.h"
#include "../kernel/shader.h"
#include "../kernel/texture.h"

namespace liar
{
namespace shaders
{

class LIAR_SHADERS_DLL OrenNayar: public Shader
{
	PY_HEADER(Shader)
public:

	OrenNayar();
	OrenNayar(const TTexturePtr& diffuse, const TTexturePtr& sigma);

	const TTexturePtr& diffuse() const;
	void setDiffuse(const TTexturePtr& diffuse);

	const TTexturePtr& sigma() const;
	void setSigma(const TTexturePtr& sigma);

private:

	class Bsdf: public kernel::Bsdf
	{
	public:
		using TValue = Spectral::TValue;
		Bsdf(const Sample& sample, const IntersectionContext& context, BsdfCaps caps, const Spectral& diffuse, TValue a, TValue b);
	private:
		BsdfOut doEvaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, BsdfCaps allowedCaps) const;
		SampleBsdfOut doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, BsdfCaps allowedCaps) const;
		Spectral eval(const TVector3D& omegaIn, const TVector3D& omegaOut) const;
		Spectral diffuseOverPi_;
		TValue a_;
		TValue b_;
	};

	TBsdfPtr doBsdf(const Sample& sample, const IntersectionContext& context) const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	TTexturePtr diffuse_;
	TTexturePtr sigma_;
};

}

}

#endif

// EOF
