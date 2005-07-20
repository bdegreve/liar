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
PY_CLASS_CONSTRUCTOR_1(Lambert, const kernel::TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(Lambert, "diffuse", diffuse, setDiffuse, "texture for diffuse component")

// --- public --------------------------------------------------------------------------------------

Lambert::Lambert():
	kernel::Shader(&Type),
	diffuse_(kernel::Texture::white())
{
}



Lambert::Lambert(const kernel::TTexturePtr& iDiffuse):
	kernel::Shader(&Type),
	diffuse_(iDiffuse)
{
}



const kernel::TTexturePtr& Lambert::diffuse() const
{
	return diffuse_;
}



void Lambert::setDiffuse(const kernel::TTexturePtr& iDiffuse)
{
	diffuse_ = iDiffuse;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

kernel::Spectrum Lambert::doDirectLight(const kernel::Sample& iSample,
										const kernel::DifferentialRay& iPrimaryRay,
										const kernel::Intersection& iIntersection,
										const kernel::IntersectionContext& iContext,
										const kernel::TSceneObjectPtr& iScene,
										const kernel::LightContext& iLight)
{
	// can we move these outside?
	const kernel::Spectrum diffuse = diffuse_->lookUp(iSample, iContext);
	if (!diffuse)
	{
		return kernel::Spectrum();
	}

	const TPoint3D& intersectionPoint = iContext.point();
	const TPoint3D shadowStartPoint = intersectionPoint + 
		(liar::tolerance * intersectionPoint.position().norm()) * iContext.normal(); 

	kernel::Spectrum result;
	for (kernel::Sample::TSubSequence2D i = iSample.subSequence2D(iLight.idLightSamples()); i; ++i)
	{
		kernel::BoundedRay shadowRay;
		kernel::Spectrum radiance = iLight.sampleRadiance(*i, shadowStartPoint, iSample.time(), shadowRay);
		const TScalar cosTheta = prim::dot(iContext.normal(), shadowRay.direction());
		if (cosTheta > TNumTraits::zero)
		{
			if (iLight.light()->isShadowless() || 
				!iScene->isIntersecting(iSample, shadowRay))
			{
				radiance *= cosTheta;
				radiance *= diffuse;
				result += radiance; 
			}
		}
	}

	return result;
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF