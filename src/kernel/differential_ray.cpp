/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
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
 *  http://liar.bramz.org
 */

#include "kernel_common.h"
#include "differential_ray.h"
#include "intersection_context.h"

namespace liar
{
namespace kernel
{

// --- public --------------------------------------------------------------------------------------

DifferentialRay::DifferentialRay()
{
}



DifferentialRay::DifferentialRay(const BoundedRay& centralRay, 
								 const TRay3D& differentialI, 
                                 const TRay3D& differentialJ):
    centralRay_(centralRay),
    differentialI_(differentialI),
    differentialJ_(differentialJ)
{
}




// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------



// --- free ----------------------------------------------------------------------------------------

DifferentialRay transform(const DifferentialRay& ray, const TTransformation3D& transformation)
{
	return DifferentialRay(
		kernel::transform(ray.centralRay(), transformation),
		prim::transform(ray.differentialI(), transformation),
		prim::transform(ray.differentialJ(), transformation));
}



DifferentialRay reflect(const IntersectionContext& context, const DifferentialRay& ray)
{
	const TVector3D& normal = context.normal();
	const TVector3D& incident = ray.direction();

	const TPoint3D& support = context.point();
	const TVector3D reflected = normal.reflect(-incident);

	const TScalar cosI = -dot(incident, normal);

	const TVector3D dIncident_dI = ray.differentialI().direction() - incident;
	const TScalar dCosI_dI = -dot(dIncident_dI, normal) - dot(incident, context.dNormal_dI());
	const TPoint3D supportI = support + context.dPoint_dI();
	const TVector3D reflectedI = reflected + 
		dIncident_dI + 2 * (dCosI_dI * normal + cosI * context.dNormal_dI());

	const TVector3D dIncident_dJ = ray.differentialJ().direction() - incident;
	const TScalar dCosI_dJ = -dot(dIncident_dJ, normal) - dot(incident, context.dNormal_dJ());
	const TPoint3D supportJ = support + context.dPoint_dJ();
	const TVector3D reflectedJ = reflected + 
		dIncident_dJ + 2 * (dCosI_dJ * normal + cosI * context.dNormal_dJ());

	return DifferentialRay(
		BoundedRay(support, reflected, tolerance, TNumTraits::infinity, prim::IsAlreadyNormalized()),
		TRay3D(supportI, reflectedI), TRay3D(supportJ, reflectedJ));
}


DifferentialRay refract(const IntersectionContext& context, const DifferentialRay& ray, 
						TScalar iRefractionIndex1over2)
{
	const TVector3D& incident = ray.direction();
	TVector3D normal = context.normal();
	TScalar cosI = -dot(normal, incident);
	if (cosI < 0)
	{
		normal = -normal;
		cosI = -cosI;
	}

	const TScalar n = iRefractionIndex1over2;
	const TScalar sinT2 = n * n * (1 - cosI * cosI);
	if (sinT2 >= 1)
	{
		DifferentialRay();
	}
	const TScalar cosT = num::sqrt(1 - sinT2);
	const TScalar alpha = n * cosI - cosT;
	const TScalar dAlpha_dCosI = n * (1 - n * cosI / cosT);

	const TPoint3D& support = context.point();
	const TVector3D transmitted = n * incident + alpha * normal;
	
	const TVector3D dIncident_dI = ray.differentialI().direction() - incident;
	const TScalar dCosI_dI = -dot(context.dNormal_dI(), incident) - dot(normal, dIncident_dI); 
 	const TPoint3D supportI = support + context.dPoint_dI();
	const TVector3D transmittedI = transmitted + 
		n * dIncident_dI + alpha * context.dNormal_dI() + dAlpha_dCosI * dCosI_dI * normal;
	
	const TVector3D dIncident_dJ = ray.differentialJ().direction() - incident;
	const TScalar dCosI_dJ = -dot(context.dNormal_dJ(), incident) - dot(normal, dIncident_dJ); 
 	const TPoint3D supportJ = support + context.dPoint_dJ();
	const TVector3D transmittedJ = transmitted + 
		n * dIncident_dJ + alpha * context.dNormal_dJ() + dAlpha_dCosI * dCosI_dJ * normal;
	
	return DifferentialRay(
		BoundedRay(support, transmitted, tolerance, TNumTraits::infinity),
		TRay3D(supportI, transmittedI), TRay3D(supportJ, transmittedJ));
}



/** @relates DifferentialRay
 */
DifferentialRay bound(const DifferentialRay& ray, TScalar nearLimit, TScalar farLimit)
{
	return DifferentialRay(bound(ray.centralRay(), nearLimit, farLimit),
		ray.differentialI(), ray.differentialJ());
}

}

}

// EOF
