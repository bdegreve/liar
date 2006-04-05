/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2005  Bram de Greve
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
 *  @brief a simple plain old raytracing shader
 *  @author Bram de Greve [BdG]
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

	const Spectrum doShade(const Sample& iSample,	const DifferentialRay& iPrimaryRay, 
		const Intersection& iIntersection, const IntersectionContext& iContext, 
		const RayTracer& iTracer);

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& iState);

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

