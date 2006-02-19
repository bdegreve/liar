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

#include "scenery_common.h"
#include "light_directional.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(LightDirectional)
PY_CLASS_CONSTRUCTOR_0(LightDirectional)
PY_CLASS_CONSTRUCTOR_2(LightDirectional, const TVector3D&, const Spectrum&)
PY_CLASS_MEMBER_RW(LightDirectional, "direction", direction, setDirection)
PY_CLASS_MEMBER_RW(LightDirectional, "intensity", intensity, setIntensity)


// --- public --------------------------------------------------------------------------------------

LightDirectional::LightDirectional():
    SceneLight(&Type),
    direction_(TVector3D(0, 0, -1)),
	intensity_(Spectrum(1))
{
}



LightDirectional::LightDirectional(const TVector3D& iDirection, const Spectrum& iIntensity):
    SceneLight(&Type),
    direction_(iDirection),
	intensity_(iIntensity)
{
}



const TVector3D& LightDirectional::direction() const
{
	return direction_;
}



const Spectrum& LightDirectional::intensity() const
{
	return intensity_;
}



void LightDirectional::setDirection(const TVector3D& iDirection)
{
	direction_ = iDirection;
}



void LightDirectional::setIntensity(const Spectrum& iIntensity)
{
	intensity_ = iIntensity;
}




// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void LightDirectional::doIntersect(const Sample& iSample, const BoundedRay& iRAy, 
							 Intersection& oResult) const
{
	oResult = Intersection::empty();
}



const bool LightDirectional::doIsIntersecting(const Sample& iSample, 
											  const BoundedRay& iRay) const
{
	return false;
}



void LightDirectional::doLocalContext(const Sample& iSample, const BoundedRay& iRay,
								const Intersection& iIntersection, 
								IntersectionContext& oResult) const
{
	LASS_THROW("since LightDirectional can never return an intersection, you've called dead code.");
}



const bool LightDirectional::doContains(const Sample& iSample, const TPoint3D& iPoint) const
{
	return false;
}




const TAabb3D LightDirectional::doBoundingBox() const
{
	return TAabb3D();
}



const TScalar LightDirectional::doArea() const
{
	return 0;
}



const Spectrum LightDirectional::doSampleEmission(const Sample& iSample,
											const TVector2D& iLightSample, 
											const TPoint3D& iDestination,
											BoundedRay& oShadowRay,
											TScalar& oPdf) const
{
	oShadowRay = BoundedRay(iDestination, -direction_, tolerance, TNumTraits::infinity);
	return intensity_;
}



const unsigned LightDirectional::doNumberOfEmissionSamples() const
{
	return 1;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
