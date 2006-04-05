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



DifferentialRay::DifferentialRay(const BoundedRay& iCentralRay, 
								 const TRay3D& iDifferentialI, 
                                 const TRay3D& iDifferentialJ):
    centralRay_(iCentralRay),
    differentialI_(iDifferentialI),
    differentialJ_(iDifferentialJ)
{
}




// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------



// --- free ----------------------------------------------------------------------------------------

DifferentialRay reflect(const IntersectionContext& iContext, const DifferentialRay& iRay)
{
	const TVector3D& normal = iContext.normal();
	const TVector3D& incident = iRay.direction();

	const TPoint3D& support = iContext.point();
	const TVector3D reflected = normal.reflect(-incident);

	const TScalar cosI = -dot(incident, normal);

	const TVector3D dIncident_dI = iRay.differentialI().direction() - incident;
	const TScalar dCosI_dI = -dot(dIncident_dI, normal) - dot(incident, iContext.dNormal_dI());
	const TPoint3D supportI = support + iContext.dPoint_dI();
	const TVector3D reflectedI = reflected + 
		dIncident_dI + 2 * (dCosI_dI * normal + cosI * iContext.dNormal_dI());

	const TVector3D dIncident_dJ = iRay.differentialJ().direction() - incident;
	const TScalar dCosI_dJ = -dot(dIncident_dJ, normal) - dot(incident, iContext.dNormal_dJ());
	const TPoint3D supportJ = support + iContext.dPoint_dJ();
	const TVector3D reflectedJ = reflected + 
		dIncident_dJ + 2 * (dCosI_dJ * normal + cosI * iContext.dNormal_dJ());

	return DifferentialRay(
		BoundedRay(support, reflected, tolerance, TNumTraits::infinity, prim::IsAlreadyNormalized()),
		TRay3D(supportI, reflectedI), TRay3D(supportJ, reflectedJ));
}


DifferentialRay refract(const IntersectionContext& iContext, const DifferentialRay& iRay, 
						TScalar iRefractionIndex1over2)
{
	const TVector3D& incident = iRay.direction();
	TVector3D normal = iContext.normal();
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

	const TPoint3D& support = iContext.point();
	const TVector3D transmitted = n * incident + alpha * normal;
	
	const TVector3D dIncident_dI = iRay.differentialI().direction() - incident;
	const TScalar dCosI_dI = -dot(iContext.dNormal_dI(), incident) - dot(normal, dIncident_dI); 
 	const TPoint3D supportI = support + iContext.dPoint_dI();
	const TVector3D transmittedI = transmitted + 
		n * dIncident_dI + alpha * iContext.dNormal_dI() + dAlpha_dCosI * dCosI_dI * normal;
	
	const TVector3D dIncident_dJ = iRay.differentialJ().direction() - incident;
	const TScalar dCosI_dJ = -dot(iContext.dNormal_dJ(), incident) - dot(normal, dIncident_dJ); 
 	const TPoint3D supportJ = support + iContext.dPoint_dJ();
	const TVector3D transmittedJ = transmitted + 
		n * dIncident_dJ + alpha * iContext.dNormal_dJ() + dAlpha_dCosI * dCosI_dJ * normal;
	
	return DifferentialRay(
		BoundedRay(support, transmitted, tolerance, TNumTraits::infinity),
		TRay3D(supportI, transmittedI), TRay3D(supportJ, transmittedJ));
}



}

}

// EOF