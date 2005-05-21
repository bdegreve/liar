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
#include "light_point.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_NAME(PyLightPointAttenuation, "Attenuation")
PY_CLASS_CONSTRUCTOR_0(PyLightPointAttenuation);
PY_CLASS_CONSTRUCTOR_3(PyLightPointAttenuation, TScalar, TScalar, TScalar);
PY_CLASS_PUBLIC_MEMBER(PyLightPointAttenuation, constant);
PY_CLASS_PUBLIC_MEMBER(PyLightPointAttenuation, linear);
PY_CLASS_PUBLIC_MEMBER(PyLightPointAttenuation, quadratic);

PY_DECLARE_CLASS(LightPoint)
PY_CLASS_CONSTRUCTOR_0(LightPoint)
PY_CLASS_CONSTRUCTOR_2(LightPoint, const TPoint3D&, const TSpectrum&)
PY_CLASS_MEMBER_RW(LightPoint, "position", position, setPosition)
PY_CLASS_MEMBER_RW(LightPoint, "power", power, setPower)
PY_CLASS_MEMBER_RW(LightPoint, "attenuation", attenuation, setAttenuation)
PY_CLASS_INNER_CLASS_NAME(LightPoint, PyLightPointAttenuation, "Attenuation")


// --- public --------------------------------------------------------------------------------------

LightPoint::Attenuation::Attenuation():
	constant(TNumTraits::zero),
    linear(TNumTraits::zero),
    quadratic(2 * TNumTraits::pi)
{
}



LightPoint::Attenuation::Attenuation(TScalar iConstant, TScalar iLinear, TScalar iQuadratic):
    constant(iConstant),
    linear(iLinear),
    quadratic(iQuadratic)
{
}



LightPoint::LightPoint():
    SceneLight(&Type),
	position_(TPoint3D()),
	power_(TSpectrum()),
    attenuation_()
{
}



LightPoint::LightPoint(const TPoint3D& iPosition, const TSpectrum& iPower):
    SceneLight(&Type),
    position_(iPosition),
	power_(iPower),
    attenuation_()
{
}



const TPoint3D& LightPoint::position() const
{
	return position_;
}



const TSpectrum& LightPoint::power() const
{
	return power_;
}



const LightPoint::Attenuation& LightPoint::attenuation() const
{
	return attenuation_;
}



void LightPoint::setPosition(const TPoint3D& iPosition)
{
	position_ = iPosition;
}



void LightPoint::setPower(const TSpectrum& iPower)
{
	power_ = iPower;
}



void LightPoint::setAttenuation(const LightPoint::Attenuation& iAttenuation)
{
	attenuation_ = iAttenuation;
}




// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void LightPoint::doIntersect(const TRay3D& iRAy, kernel::Intersection& oResult) const
{
	oResult = kernel::Intersection::empty();
}



const bool LightPoint::doIsIntersecting(const TRay3D& iRay, TScalar iMaxT,
										const SceneObject* iExcludeA, 
										const SceneObject* iExcludeB) const
{
	return false;
}



void LightPoint::doLocalContext(const TRay3D& iRay, 
								const kernel::Intersection& iIntersection, 
								kernel::IntersectionContext& oResult) const
{
	LASS_THROW("since LightPoint can never return an intersection, you've called dead code.");
}



const TAabb3D LightPoint::doBoundingBox() const
{
	return TAabb3D(position_, position_);
}



const TSpectrum LightPoint::doSampleRadiance(const TVector2D& iSample, 
											 const TPoint3D& iDestination,
											 TRay3D& oShadowRay,
											 TScalar& oMaxT) const
{
	TSpectrum result = power_;
    const TScalar squaredDistance = (position_ - iDestination).squaredNorm();
	const TScalar att = attenuation_.constant + 
		attenuation_.linear * num::sqrt(squaredDistance) + 
		attenuation_.quadratic * squaredDistance;
	result /= att;

	oShadowRay = TRay3D(iDestination, position_);
	oMaxT = prim::distance(iDestination, position_);
	return result;
}



const unsigned LightPoint::doNumberOfRadianceSamples() const
{
	return 1;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
