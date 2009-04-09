/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
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

#include "scenery_common.h"
#include "sky.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_DOC(Sky, "sky sphere")
PY_CLASS_CONSTRUCTOR_0(Sky)



// --- public --------------------------------------------------------------------------------------

Sky::Sky():
	radius_(1e5f)
{
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void Sky::doIntersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const
{
	if (ray.inRange(radius_))
	{
		result = Intersection(this, TNumTraits::infinity, seLeaving);
	}
	else
	{
		result = Intersection::empty();
	}
}



const bool Sky::doIsIntersecting(const Sample& sample, const BoundedRay& ray) const
{
	return ray.inRange(radius_);
}



void Sky::doLocalContext(const Sample& sample, const BoundedRay& ray, 
		const Intersection& intersection, IntersectionContext& result) const
{
	//LASS_ASSERT(intersection.t() == radius_);
    result.setT(radius_);
    result.setPoint(ray.point(radius_));

	//         [sin theta * cos phi]
	// R = r * [sin theta * sin phi]
	//         [cos theta          ]
	//
	const TVector3D normal = -ray.direction();
	result.setNormal(normal);
	result.setGeometricNormal(normal);

	// phi = 2pi * u
	// theta = pi * v
	//
	LASS_ASSERT(normal.z >= -TNumTraits::one && normal.z <= TNumTraits::one);
    const TScalar phi = num::atan2(normal.x, normal.y);
    const TScalar theta = num::acos(normal.z);
	result.setUv(phi / (2 * TNumTraits::pi), theta / TNumTraits::pi);

	//               [sin theta * -sin phi]               [cos theta * cos phi]
	// dN_du = 2pi * [sin theta * cos phi ]  dN_dv = pi * [cos theta * sin phi]
	//               [0                   ]               [-sin theta         ]
	//
	const TScalar sinTheta = num::sin(theta);
	const TScalar cosTheta_sinTheta = normal.z / sinTheta;
	const TVector3D dNormal_dU = 2 * TNumTraits::pi * TVector3D(-normal.y, normal.x, 0);
	const TVector3D dNormal_dV = TNumTraits::pi * TVector3D(
		cosTheta_sinTheta * normal.x, cosTheta_sinTheta * normal.y, -sinTheta);
	result.setDNormal_dU(dNormal_dU);
	result.setDNormal_dV(dNormal_dV);
	
	result.setDPoint_dU(-radius_ * dNormal_dU);
	result.setDPoint_dV(-radius_ * dNormal_dU);
}



const bool Sky::doContains(const Sample& sample, const TPoint3D& point) const
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



void Sky::doSetState(const TPyObjectPtr& state)
{
	LASS_ENFORCE(python::decodeTuple(state, radius_));
}



const bool Sky::doHasSurfaceSampling() const
{
	return true;
}



const TPoint3D Sky::doSampleSurface(const TPoint2D& sample, TVector3D& normal, 
		TScalar& pdf) const
{
	return sampleSurface(sample, TPoint3D(0, 0, 0), normal, pdf);
}



const TPoint3D Sky::doSampleSurface(const TPoint2D& sample, const TPoint3D& target,
		TVector3D& normal, TScalar& pdf) const
{
	const TScalar z = 2 * sample.y - 1;
	const TScalar rho = num::sqrt(1 - num::sqr(z));
	const TScalar theta = 2 * TNumTraits::pi * sample.x;
	const TScalar x = rho * num::cos(theta);
	const TScalar y = rho * num::sin(theta);

	normal = TVector3D(x, y, z);
	pdf = 1 / (4 * TNumTraits::pi); // we're actually selecting a direction ...
	return target - radius_ * normal;
}




// --- free ----------------------------------------------------------------------------------------



// --- python --------------------------------------------------------------------------------------

}

}

// EOF
