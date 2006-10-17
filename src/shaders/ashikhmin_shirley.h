/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2006  Bram de Greve
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  http://liar.sourceforge.net
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
	AshikhminShirley(const TTexturePtr& iDiffuse, const TTexturePtr& iSpecular);

	const TTexturePtr& diffuse() const;
	void setDiffuse(const TTexturePtr& iDiffuse);

	const TTexturePtr& specular() const;
	void setSpecular(const TTexturePtr& iSpecular);

	const TTexturePtr& specularPowerU() const;
	void setSpecularPowerU(const TTexturePtr& iSpecularPower);

	const TTexturePtr& specularPowerV() const;
	void setSpecularPowerV(const TTexturePtr& iSpecularPower);

	const unsigned numberOfSamples() const;
	void setNumberOfSamples(unsigned number);

private:

	virtual void doBsdf(
		const Sample& sample, const IntersectionContext& context, const TVector3D& omegaOut,
		const TVector3D* firstOmegaIn, const TVector3D* lastOmegaIn, 
		Spectrum* firstValue, TScalar* firstPdf) const;
	virtual void doSampleBsdf(
		const Sample& sample, const IntersectionContext& context, const TVector3D& omegaOut, 
		const TPoint2D* firstBsdfSample, const TPoint2D* lastBsdfSample,
		TVector3D* firstOmegaIn, Spectrum* firstValue, TScalar* firstPdf) const;
	void doRequestSamples(const TSamplerPtr& sampler);

	const Spectrum brdf(const TVector3D& k1, const TVector3D& k2, const TVector3D& h,
		const TVector3D& n, const TVector3D& u, const TVector3D& v,
		const Spectrum& Rs, const Spectrum& Rd, TScalar n_u, TScalar n_v) const;
	const TVector3D generateH(const TVector2D& sample, 
		const TVector3D& n, const TVector3D& u, const TVector3D& v, 
		TScalar n_u, TScalar n_v, TScalar& pdf) const ;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	TTexturePtr diffuse_;
	TTexturePtr specular_;
	TTexturePtr specularPowerU_;
	TTexturePtr specularPowerV_;
	unsigned numberOfSamples_;
	int samplesId_;
};

}

}

#endif

// EOF

