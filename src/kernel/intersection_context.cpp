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
#include "intersection_context.h"
#include "differential_ray.h"
#include <lass/num/impl/matrix_solve.h>
#include <lass/prim/plane_3d.h>

namespace liar
{
namespace kernel
{

// --- public --------------------------------------------------------------------------------------

IntersectionContext::IntersectionContext():
	point_(),
	dPoint_dU_(),
	dPoint_dV_(),
	dPoint_dI_(),
	dPoint_dJ_(),
	normal_(),
	dNormal_dU_(),
	dNormal_dV_(),
	dNormal_dI_(),
	dNormal_dJ_(),
	geometricNormal_(),
	uv_(),
	dUv_dI_(),
	dUv_dJ_(),
	t_(),
	shader_(),
	hasScreenSpaceDifferentials_(false)
{
}



void IntersectionContext::setScreenSpaceDifferentials(const DifferentialRay& iRay)
{
	setScreenSpaceDifferentialsI(iRay.differentialI(), dPoint_dI_, dNormal_dI_, dUv_dI_);
	setScreenSpaceDifferentialsI(iRay.differentialJ(), dPoint_dJ_, dNormal_dJ_, dUv_dJ_);
	hasScreenSpaceDifferentials_ = true;
}



void IntersectionContext::transform(const TTransformation3D& iTransformation)
{
	point_ = prim::transform(point_, iTransformation);
	dPoint_dU_ = prim::transform(dPoint_dU_, iTransformation);
	dPoint_dV_ = prim::transform(dPoint_dV_, iTransformation);
	normal_ = prim::normalTransform(normal_, iTransformation);
	dNormal_dU_ = prim::normalTransform(dNormal_dU_, iTransformation);
	dNormal_dV_ = prim::normalTransform(dNormal_dV_, iTransformation);
	geometricNormal_ = prim::normalTransform(geometricNormal_, iTransformation);
	
	if (hasScreenSpaceDifferentials_)
	{
		dPoint_dI_ = prim::transform(dPoint_dI_, iTransformation);
		dPoint_dJ_ = prim::transform(dPoint_dJ_, iTransformation);
		dNormal_dI_ = prim::normalTransform(dNormal_dI_, iTransformation);
		dNormal_dJ_ = prim::normalTransform(dNormal_dJ_, iTransformation);
	}
}



void IntersectionContext::translate(const TVector3D& iOffset)
{
	point_ += iOffset;
}



void IntersectionContext::flipNormal()
{
	normal_ = -normal_;
	dNormal_dU_ = -dNormal_dU_;
	dNormal_dV_ = -dNormal_dV_;
	dNormal_dI_ = -dNormal_dI_;
	dNormal_dJ_ = -dNormal_dJ_;
}



void IntersectionContext::flipGeometricNormal()
{
	geometricNormal_ = -geometricNormal_;
}




// --- protected -----------------------------------------------------------------------------------

// --- private -------------------------------------------------------------------------------------

void IntersectionContext::setScreenSpaceDifferentialsI(const TRay3D& iRay_dI, 
													   TVector3D& oDPoint_dI, 
													   TVector3D& oDNormal_dI,
													   TVector2D& oDUv_dI)
{	
	prim::Plane3D<TScalar, prim::Cartesian, prim::Unnormalized> plane(normal_, point_);
	TScalar t;
	prim::Result result = prim::intersect(plane, iRay_dI, t);
	if (result != prim::rOne)
	{
		oDPoint_dI = TVector3D();
		oDNormal_dI = TVector3D();
		oDUv_dI = TVector2D();
		return;
	}
	oDPoint_dI = iRay_dI.point(t) - point_;

	const prim::XYZ majorAxis = plane.majorAxis();
	const prim::XYZ a = majorAxis + 1;
	const prim::XYZ b = majorAxis + 2;

	TScalar matrix[4] = { dPoint_dU_[a], dPoint_dV_[a], dPoint_dU_[b], dPoint_dV_[b] };
	TScalar solution[2] = { oDPoint_dI[a], oDPoint_dI[b] };
	if (!num::impl::cramer2<TScalar>(matrix, solution, solution + 2))
	{
		oDNormal_dI = TVector3D();
		oDUv_dI = TVector2D();
		return;
	}
	oDNormal_dI = dNormal_dU_ * solution[0] + dNormal_dV_ * solution[1];
	oDUv_dI = TVector2D(solution[0], solution[1]);
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF