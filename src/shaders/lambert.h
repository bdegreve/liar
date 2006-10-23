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

/** @class liar::shaders::Lambert
 *  @brief very simple shader using lambert's cosine law.
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SHADERS_LAMBERT_H
#define LIAR_GUARDIAN_OF_INCLUSION_SHADERS_LAMBERT_H

#include "shaders_common.h"
#include "../kernel/shader.h"
#include "../kernel/texture.h"

namespace liar
{
namespace shaders
{

class LIAR_SHADERS_DLL Lambert: public Shader
{
    PY_HEADER(Shader)
public:

	Lambert();
	Lambert(const TTexturePtr& iDiffuse);

	const TTexturePtr& diffuse() const;
	void setDiffuse(const TTexturePtr& iDiffuse);

private:

	virtual void doBsdf(
		const Sample& sample, const IntersectionContext& context, const TVector3D& omegaOut,
		const TVector3D* firstOmegaIn, const TVector3D* lastOmegaIn, 
		Spectrum* firstValue, TScalar* firstPdf, unsigned allowCaps) const;

	virtual void doSampleBsdf(
		const Sample& sample, const IntersectionContext& context, const TVector3D& omegaOut, 
		const TPoint2D* firstBsdfSample, const TPoint2D* lastBsdfSample,
		TVector3D* firstOmegaIn, Spectrum* firstValue, TScalar* firstPdf,
		unsigned allowedCaps) const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	TTexturePtr diffuse_;
};

}

}

#endif

// EOF

