/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2006  Bram de Greve
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
#include "ray_tracer.h"
#include <lass/num/impl/matrix_solve.h>
#include <lass/prim/plane_3d.h>

namespace liar
{
namespace kernel
{

// --- public --------------------------------------------------------------------------------------

IntersectionContext::IntersectionContext(const RayTracer* tracer):
	point_(),
	dPoint_dU_(),
	dPoint_dV_(),
	dPoint_dI_(),
	dPoint_dJ_(),
	geometricNormal_(),
	normal_(),
	dNormal_dU_(),
	dNormal_dV_(),
	dNormal_dI_(),
	dNormal_dJ_(),
	uv_(),
	dUv_dI_(),
	dUv_dJ_(),
	t_(),
	tracer_(tracer),
	shader_(0),
	shaderToWorld_(),
	localToWorld_(),
	hasScreenSpaceDifferentials_(false)
{
}



void IntersectionContext::setShader(const TShaderPtr& shader)
{
	shader_ = shader.get();
	generateShaderToWorld();
}



void IntersectionContext::setScreenSpaceDifferentials(const DifferentialRay& ray)
{
	setScreenSpaceDifferentialsI(ray.differentialI(), dPoint_dI_, dNormal_dI_, dUv_dI_);
	setScreenSpaceDifferentialsI(ray.differentialJ(), dPoint_dJ_, dNormal_dJ_, dUv_dJ_);
	hasScreenSpaceDifferentials_ = true;
}



void IntersectionContext::transformBy(const TTransformation3D& transformation)
{
	if (!shader_)
	{
		point_ = prim::transform(point_, transformation);
		dPoint_dU_ = prim::transform(dPoint_dU_, transformation);
		dPoint_dV_ = prim::transform(dPoint_dV_, transformation);

		geometricNormal_ = prim::normalTransform(geometricNormal_, transformation);
		normal_ = prim::normalTransform(normal_, transformation);
		dNormal_dU_ = prim::normalTransform(dNormal_dU_, transformation);
		dNormal_dV_ = prim::normalTransform(dNormal_dV_, transformation);
		const TScalar rescaleNormal = num::inv(normal_.norm());
		normal_ *= rescaleNormal;
		dNormal_dU_ *= rescaleNormal;
		dNormal_dV_ *= rescaleNormal;
		
		if (hasScreenSpaceDifferentials_)
		{
			dPoint_dI_ = prim::transform(dPoint_dI_, transformation);
			dPoint_dJ_ = prim::transform(dPoint_dJ_, transformation);
			dNormal_dI_ = prim::normalTransform(dNormal_dI_, transformation);
			dNormal_dJ_ = prim::normalTransform(dNormal_dJ_, transformation);
		}
	}
	else
	{
		shaderToWorld_ = prim::concatenate(shaderToWorld_, transformation);
		localToWorld_ = prim::concatenate(localToWorld_, transformation);
	}
}



void IntersectionContext::translateBy(const TVector3D& offset)
{
	if (!shader_)
	{
		point_ += offset;
	}
	else
	{
		TTransformation3D translation = TTransformation3D::translation(offset);
		shaderToWorld_ = prim::concatenate(shaderToWorld_, translation);
		localToWorld_ = prim::concatenate(localToWorld_, translation);
	}
}



/** flip the normal so that it -- in world space -- is in the same hemisphere as @a worldOmega.
 */
const TVector3D IntersectionContext::flipTo(const TVector3D& worldOmega)
{
	const TVector3D worldNormal = prim::transform(TVector3D(0, 0, 1), shaderToWorld_);
	if (dot(worldNormal, worldOmega) >= 0)
	{
		return worldNormal;
	}
	geometricNormal_ = -geometricNormal_;
	normal_ = -normal_;
	dNormal_dU_ = -dNormal_dU_;
	dNormal_dV_ = -dNormal_dV_;
	//shaderToWorld_ = prim::concatenate(TTransformation3D::scaler(TVector3D(1, 1, -1)), shaderToWorld_);
	shaderToWorld_ = prim::concatenate(shaderToWorld_, TTransformation3D::scaler(TVector3D(1, 1, -1)));
	return -worldNormal;
}




// --- protected -----------------------------------------------------------------------------------

// --- private -------------------------------------------------------------------------------------

void IntersectionContext::setScreenSpaceDifferentialsI(const TRay3D& dRay_dI, 
													   TVector3D& dPoint_dI, 
													   TVector3D& dNormal_dI,
													   TVector2D& dUv_dI)
{	
	prim::Plane3D<TScalar, prim::Cartesian, prim::Unnormalized> plane(normal_, point_);
	TScalar t;
	prim::Result result = prim::intersect(plane, dRay_dI, t);
	if (result != prim::rOne)
	{
		dPoint_dI = TVector3D();
		dNormal_dI = TVector3D();
		dUv_dI = TVector2D();
		return;
	}
	dPoint_dI = dRay_dI.point(t) - point_;

	const prim::XYZ majorAxis = plane.majorAxis();
	const prim::XYZ a = majorAxis + 1;
	const prim::XYZ b = majorAxis + 2;

	TScalar matrix[4] = { dPoint_dU_[a], dPoint_dV_[a], dPoint_dU_[b], dPoint_dV_[b] };
	TScalar solution[2] = { dPoint_dI[a], dPoint_dI[b] };
	if (!num::impl::cramer2<TScalar>(matrix, solution, solution + 2))
	{
		dNormal_dI = TVector3D();
		dUv_dI = TVector2D();
		return;
	}
	dNormal_dI = dNormal_dU_ * solution[0] + dNormal_dV_ * solution[1];
	dUv_dI = TVector2D(solution[0], solution[1]);
}



void IntersectionContext::generateShaderToWorld()
{
	if (shader_)
	{
		const TVector3D u = dPoint_dU_.normal();
		const TVector3D v = cross(normal_, u);
		shaderToWorld_ = TTransformation3D(point_, u, v, normal_);
	}
}




// --- free ----------------------------------------------------------------------------------------

}

}

// EOF