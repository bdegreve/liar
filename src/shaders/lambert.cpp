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

#include "shaders_common.h"
#include "lambert.h"
#include "../kernel/ray_tracer.h"

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS(Lambert)
PY_CLASS_CONSTRUCTOR_0(Lambert)
PY_CLASS_CONSTRUCTOR_1(Lambert, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(Lambert, "diffuse", diffuse, setDiffuse, "texture for diffuse component")

// --- public --------------------------------------------------------------------------------------

Lambert::Lambert():
	Shader(&Type),
	diffuse_(Texture::white())
{
}



Lambert::Lambert(const TTexturePtr& iDiffuse):
	Shader(&Type),
	diffuse_(iDiffuse)
{
}



const TTexturePtr& Lambert::diffuse() const
{
	return diffuse_;
}



void Lambert::setDiffuse(const TTexturePtr& iDiffuse)
{
	diffuse_ = iDiffuse;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

Spectrum Lambert::doShade(const Sample& iSample,
						  const DifferentialRay& iPrimaryRay,
						  const Intersection& iIntersection,
						  const IntersectionContext& iContext,
						  const RayTracer& iTracer)
{
	const Spectrum diffuse = diffuse_->lookUp(iSample, iContext);
	if (!diffuse)
	{
		return Spectrum();
	}
	
	const TVector3D& geoNormal = iContext.normal();
	const bool isOutside = dot(geoNormal, iPrimaryRay.direction()) < 0;
	const TVector3D shadeNormal = isOutside ? geoNormal : -geoNormal;

	Spectrum result;
	RayTracer::TLightRange lightSamples = iTracer.sampleLights(iSample, iContext);
	for (RayTracer::TLightRange::iterator i = lightSamples.begin(); i != lightSamples.end(); ++i)
	{
		const TScalar cosTheta = prim::dot(shadeNormal, i->direction());
		if (cosTheta > TNumTraits::zero)
		{
			result += i->radiance() * cosTheta;
		}
	}
	result *= diffuse;

	return result;
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF