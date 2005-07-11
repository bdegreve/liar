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

class LIAR_SHADERS_DLL Simple: public kernel::Shader
{
    PY_HEADER(Shader)
public:

	Simple();
	Simple(const kernel::TTexturePtr& iDiffuse);
	Simple(const kernel::TTexturePtr& iDiffuse, const kernel::TTexturePtr& iSpecular); 

	const kernel::TTexturePtr& diffuse() const;
	void setDiffuse(const kernel::TTexturePtr& iDiffuse);

	const kernel::TTexturePtr& specular() const;
	void setSpecular(const kernel::TTexturePtr& iSpecular);

	const kernel::TTexturePtr& specularPower() const;
	void setSpecularPower(const kernel::TTexturePtr& iSpecularPower);

private:

	kernel::Spectrum doDirectLight(const kernel::Sample& iSample, const kernel::DifferentialRay& iRay,
		const kernel::Intersection& iIntersection, const kernel::IntersectionContext& iContext, 
		const kernel::TSceneObjectPtr& iScene, const kernel::LightContext& iLight);

	kernel::TTexturePtr diffuse_;
	kernel::TTexturePtr specular_;
	kernel::TTexturePtr specularPower_;
};

}

}

#endif

// EOF

