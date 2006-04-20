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
#include "sky.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(Sky)
PY_CLASS_CONSTRUCTOR_0(Sky)



// --- public --------------------------------------------------------------------------------------

Sky::Sky():
	SceneObject(&Type),
	radius_(1e5f)
{
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void Sky::doIntersect(const Sample& iSample, const BoundedRay& iRay, Intersection& oResult) const
{
	if (iRay.inRange(radius_))
	{
		oResult = Intersection(this, TNumTraits::infinity, seLeaving);
	}
	else
	{
		oResult = Intersection::empty();
	}
}



const bool Sky::doIsIntersecting(const Sample& iSample, const BoundedRay& iRay) const
{
	return iRay.inRange(radius_);
}



void Sky::doLocalContext(const Sample& iSample, const BoundedRay& iRay, 
		const Intersection& iIntersection, IntersectionContext& oResult) const
{
	//LASS_ASSERT(iIntersection.t() == radius_);
    oResult.setT(radius_);
    oResult.setPoint(iRay.point(radius_));

	//         [sin theta * cos phi]
	// R = r * [sin theta * sin phi]
	//         [cos theta          ]
	//
	const TVector3D normal = -iRay.direction();
	oResult.setNormal(normal);
	oResult.setGeometricNormal(normal);

	// phi = 2pi * u
	// theta = pi * v
	//
	LASS_ASSERT(normal.z >= -TNumTraits::one && normal.z <= TNumTraits::one);
    const TScalar phi = num::atan2(normal.x, normal.y);
    const TScalar theta = num::acos(normal.z);
	oResult.setUv(phi / (2 * TNumTraits::pi), theta / TNumTraits::pi);

	//               [sin theta * -sin phi]               [cos theta * cos phi]
	// dN_du = 2pi * [sin theta * cos phi ]  dN_dv = pi * [cos theta * sin phi]
	//               [0                   ]               [-sin theta         ]
	//
	const TScalar sinTheta = num::sin(theta);
	const TScalar cosTheta_sinTheta = normal.z / sinTheta;
	const TVector3D dNormal_dU = 2 * TNumTraits::pi * TVector3D(-normal.y, normal.x, 0);
	const TVector3D dNormal_dV = TNumTraits::pi * TVector3D(
		cosTheta_sinTheta * normal.x, cosTheta_sinTheta * normal.y, -sinTheta);
	oResult.setDNormal_dU(dNormal_dU);
	oResult.setDNormal_dV(dNormal_dV);
	
	oResult.setDPoint_dU(-radius_ * dNormal_dU);
	oResult.setDPoint_dV(-radius_ * dNormal_dU);
}



const bool Sky::doContains(const Sample& iSample, const TPoint3D& iPoint) const
{
	return true;
}



const TAabb3D Sky::doBoundingBox() const
{
    return TAabb3D(TPoint3D(-radius_, -radius_, -radius_), TPoint3D(radius_, radius_, radius_));
}



const TScalar Sky::doArea() const
{
	return TNumTraits::infinity;
}



const TPyObjectPtr Sky::doGetState() const
{
	return python::makeTuple(radius_);
}



void Sky::doSetState(const TPyObjectPtr& iState)
{
	LASS_ENFORCE(python::decodeTuple(iState, radius_));
}



const bool Sky::doHasSurfaceSampling() const
{
	return true;
}



const TPoint3D Sky::doSampleSurface(const TVector2D& iSample, TVector3D& oNormal, 
		TScalar& oPdf) const
{
	return sampleSurface(iSample, TPoint3D(0, 0, 0), oNormal, oPdf);
}



const TPoint3D Sky::doSampleSurface(const TVector2D& iSample, const TPoint3D& iTarget,
		TVector3D& oNormal, TScalar& oPdf) const
{
	const TScalar z = 2 * iSample.y - 1;
	const TScalar rho = num::sqrt(1 - num::sqr(z));
	const TScalar theta = 2 * TNumTraits::pi * iSample.x;
	const TScalar x = rho * num::cos(theta);
	const TScalar y = rho * num::sin(theta);

	oNormal = TVector3D(x, y, z);
	oPdf = 1 / (4 * TNumTraits::pi); // we're actually selecting a direction ...
	return iTarget - radius_ * oNormal;
}




// --- free ----------------------------------------------------------------------------------------



// --- python --------------------------------------------------------------------------------------

}

}

// EOF
