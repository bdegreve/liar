/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
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


TScalar Sky::radius_ = num::sqrt(TNumTraits::max / 10);


// --- public --------------------------------------------------------------------------------------

Sky::Sky()
{
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void Sky::doIntersect(const Sample&, const BoundedRay& ray, Intersection& result) const
{
	if (ray.inRange(radius_))
	{
		result = Intersection(this, radius_, seLeaving);
	}
	else
	{
		result = Intersection::empty();
	}
}



bool Sky::doIsIntersecting(const Sample&, const BoundedRay& ray) const
{
	return ray.inRange(radius_);
}



void Sky::doLocalContext(const Sample&, const BoundedRay& ray, const Intersection&, IntersectionContext& result) const
{
	//LASS_ASSERT(intersection.t() == diameter_);
	result.setT(radius_);
	result.setPoint(TPoint3D(radius_ * ray.direction()));

	//                 [sin theta * cos phi]
	// R = r * D = r * [sin theta * sin phi]
	//                 [cos theta          ]
	//
	const TVector3D& dir = ray.direction();

	//          [-sin theta * cos phi]
	// N = -D = [-sin theta * sin phi]
	//          [-cos theta          ]
	const TVector3D normal = -ray.direction();
	result.setNormal(normal);
	result.setGeometricNormal(normal);

	// phi = 2pi * u
	// theta = -pi * v
	//
	const TScalar phi = sphericalPhi(dir);
	const TScalar theta = sphericalTheta(dir);
	result.setUv(phi / (2 * TNumTraits::pi), -theta / TNumTraits::pi);

	//               [sin theta * -sin phi]                  [ cos theta * cos phi]
	// dD_du = 2pi * [sin theta *  cos phi]    dD_dv = -pi * [ cos theta * sin phi]
	//               [0                   ]                  [-sin theta          ]
	//
	const TScalar sinTheta = num::sin(theta);
	const TScalar cosTheta_sinTheta = dir.z / sinTheta;
	const TVector3D dDir_dU = 2 * TNumTraits::pi * TVector3D(-dir.y, dir.x, 0);
	const TVector3D dDir_dV = -TNumTraits::pi * TVector3D(cosTheta_sinTheta * dir.x, cosTheta_sinTheta * dir.y, -sinTheta);

	// dN/du = -dD/du    dN/dv = -dD/dv
	//
	result.setDNormal_dU(-dDir_dU);
	result.setDNormal_dV(-dDir_dV);
	
	// dR/du = r * dD/du    dR/dv = r * dD/dv
	//
	result.setDPoint_dU(radius_ * dDir_dU);
	result.setDPoint_dV(radius_ * dDir_dV);
}



bool Sky::doContains(const Sample&, const TPoint3D&) const
{
	return true;
}



const TAabb3D Sky::doBoundingBox() const
{
    return TAabb3D();
}



TScalar Sky::doArea() const
{
	return TNumTraits::infinity;
}



TScalar Sky::doArea(const TVector3D&) const
{
	return TNumTraits::infinity;
}



const TPyObjectPtr Sky::doGetState() const
{
	return python::makeTuple();
}



void Sky::doSetState(const TPyObjectPtr& state)
{
	LASS_ENFORCE(python::decodeTuple(state));
}



bool Sky::doHasSurfaceSampling() const
{
	return true;
}



const TPoint3D Sky::doSampleSurface(const TPoint2D& sample, TVector3D& normal, TScalar& pdf) const
{
	return sampleSurface(sample, TPoint3D(0, 0, 0), normal, pdf);
}



const TPoint3D Sky::doSampleSurface(const TPoint2D& sample, const TPoint3D& target, TVector3D& normal, TScalar& pdf) const
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
