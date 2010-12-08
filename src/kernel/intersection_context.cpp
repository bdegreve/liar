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

#include "kernel_common.h"
#include "intersection_context.h"
#include "differential_ray.h"
#include "ray_tracer.h"
#include <lass/num/impl/matrix_solve.h>
#include <lass/prim/impl/plane_3d_impl_detail.h>
#include <lass/prim/plane_3d.h>

namespace liar
{
namespace kernel
{

// --- public --------------------------------------------------------------------------------------

IntersectionContext::IntersectionContext(const SceneObject& object, const Sample& sample, const BoundedRay& ray, const Intersection& intersection):
	sample_(sample)
{
	init(object, ray, intersection);

	if (shader_)
	{
		shader_->shadeContext(sample_, *this);
	}
	flipTo(-ray.direction());
}



IntersectionContext::IntersectionContext(const SceneObject& object, const Sample& sample, const DifferentialRay& ray, const Intersection& intersection):
	sample_(sample)
{
	init(object, ray.centralRay(), intersection);

	DifferentialRay localRay = transform(ray, worldToLocal());
	setScreenSpaceDifferentialsI(localRay.differentialI(), dPoint_dI_, dNormal_dI_, dUv_dI_);
	setScreenSpaceDifferentialsI(localRay.differentialJ(), dPoint_dJ_, dNormal_dJ_, dUv_dJ_);
	hasScreenSpaceDifferentials_ = true;

	if (shader_)
	{
		shader_->shadeContext(sample_, *this);
	}
	flipTo(-ray.direction());
}



const TBsdfPtr& IntersectionContext::bsdf() const
{
	if (shader_ && !bsdf_)
	{
		bsdf_ = shader_->bsdf(sample_, *this);
	}
	return bsdf_;
}



void IntersectionContext::setPoint(const TPoint3D& point) 
{ 
	point_ = point; 
	bsdfToLocalHasChanged(); 
}



void IntersectionContext::setDPoint_dU(const TVector3D& dPoint_dU) 
{ 
	dPoint_dU_ = dPoint_dU; 
	bsdfToLocalHasChanged(); 
}



void IntersectionContext::setDPoint_dV(const TVector3D& dPoint_dV) 
{ 
	dPoint_dV_ = dPoint_dV; 
	bsdfToLocalHasChanged(); 
}



void IntersectionContext::setDPoint_dI(const TVector3D& dPoint_dI) 
{ 
	dPoint_dI_ = dPoint_dI; 
}



void IntersectionContext::setDPoint_dJ(const TVector3D& dPoint_dJ) 
{ 
	dPoint_dJ_ = dPoint_dJ; 
}



void IntersectionContext::setGeometricNormal(const TVector3D& geometricNormal) 
{ 
	geometricNormal_ = geometricNormal.normal(); 
}



void IntersectionContext::setNormal(const TVector3D& normal)
{ 
	normal_ = normal.normal(); 
	if (geometricNormal_.isZero())
	{
		geometricNormal_ = normal_;
	}
	bsdfToLocalHasChanged(); 
}



void IntersectionContext::setShader(const Shader* shader)
{
	shader_ = shader;
}



void IntersectionContext::transformBy(const TTransformation3D& transformation)
{
	localToWorld_ = prim::concatenate(localToWorld_, transformation);
	localToWorldHasChanged();
}



void IntersectionContext::translateBy(const TVector3D& offset)
{
	const TTransformation3D translation = TTransformation3D::translation(offset);
	localToWorld_ = prim::concatenate(localToWorld_, translation);
	localToWorldHasChanged();
}



const TTransformation3D& IntersectionContext::worldToLocal() const
{
	if (hasDirtyWorldToLocal_)
	{
		worldToLocal_ = localToWorld().inverse();
		hasDirtyWorldToLocal_ = false;
		LASS_ASSERT(hasDirtyWorldToBsdf_);
	}
	return worldToLocal_;
}



const TVector3D IntersectionContext::worldNormal() const
{
	return prim::normalTransform(normal_, localToWorld()).normal();
}



const TVector3D IntersectionContext::worldGeometricNormal() const
{
	return prim::normalTransform(geometricNormal_, localToWorld()).normal();
}



const TTransformation3D& IntersectionContext::bsdfToLocal() const
{
	if (!hasDirtyBsdfToLocal_)
	{
		return bsdfToLocal_;
	}
	TVector3D u = normal_.reject(dPoint_dU_);
	TVector3D v = normal_.reject(dPoint_dV_);
	const TScalar sqrU = u.squaredNorm();
	const TScalar sqrV = v.squaredNorm();
	if (std::max(sqrU, sqrV) < 1e-3f)
	{
		prim::impl::Plane3DImplDetail::generateDirections(normal_, u, v);
	}
	if (sqrU >= sqrV)
	{
		v = cross(normal_, u);
	}
	else
	{
		u = cross(v, normal_);
	}
	bsdfToLocal_ = TTransformation3D(point_, u.normal(), v.normal(), normal_);
	hasDirtyBsdfToLocal_ = false;
	LASS_ASSERT(hasDirtyLocalToBsdf_ && hasDirtyBsdfToWorld_ && hasDirtyWorldToBsdf_);
	return bsdfToLocal_;
}



const TTransformation3D& IntersectionContext::localToBsdf() const
{
	if (hasDirtyLocalToBsdf_)
	{
		localToBsdf_ = bsdfToLocal().inverse();
		hasDirtyLocalToBsdf_ = false;
	}
	return localToBsdf_;
}



const TTransformation3D& IntersectionContext::bsdfToWorld() const
{
	if (hasDirtyBsdfToWorld_)
	{
		bsdfToWorld_ = prim::concatenate(bsdfToLocal(), localToWorld());
		hasDirtyBsdfToWorld_ = false;
	}
	return bsdfToWorld_;
}



const TTransformation3D& IntersectionContext::worldToBsdf() const
{
	if (hasDirtyWorldToBsdf_)
	{
		worldToBsdf_ = bsdfToWorld().inverse();
		hasDirtyWorldToBsdf_ = false;
	}
	return worldToBsdf_;
}



// --- protected -----------------------------------------------------------------------------------

// --- private -------------------------------------------------------------------------------------

void IntersectionContext::init(const SceneObject& object, const BoundedRay& ray, const Intersection& intersection)
{
	t_ = 0;
	shader_ = 0;
	interior_ = 0;
	solidEvent_ = seNoEvent;
	hasScreenSpaceDifferentials_ = false;
	bsdf_ = TBsdfPtr();
	localToWorld_ = worldToLocal_ = TTransformation3D::identity();
	hasDirtyWorldToLocal_ = false;
	bsdfToLocalHasChanged();

	object.localContext(sample_, ray, intersection, *this);
}



/** flip the normal so that it -- in world space -- is in the same hemisphere as @a worldOmega.
 */
void IntersectionContext::flipTo(const TVector3D& worldOmega)
{
	if (dot(worldNormal(), worldOmega) < 0)
	{
		normal_ = -normal_;
		dNormal_dU_ = -dNormal_dU_;
		dNormal_dV_ = -dNormal_dV_;
		bsdfToLocalHasChanged(); 
	}
	if (dot(worldGeometricNormal(), worldOmega) < 0)
	{
		geometricNormal_ = -geometricNormal_;
	}
}



void IntersectionContext::setScreenSpaceDifferentialsI(
		const TRay3D& dRay_dI, TVector3D& dPoint_dI, TVector3D& dNormal_dI, TVector2D& dUv_dI)
{	
	const prim::Plane3D<TScalar, prim::Cartesian, prim::Unnormalized> plane(normal_, point_);
	TScalar t;
	const prim::Result result = prim::intersect(plane, dRay_dI, t);
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

	const TScalar matrix[4] = { dPoint_dU_[a], dPoint_dV_[a], dPoint_dU_[b], dPoint_dV_[b] };
	TScalar solution[2] = { dPoint_dI[a], dPoint_dI[b] };
	if (!num::impl::cramer2<TScalar>(matrix, solution, solution + 2))
	{
		dNormal_dI = TVector3D();
		dUv_dI = TVector2D();
		return;
	}
	dUv_dI = TVector2D(solution[0], solution[1]);
	dNormal_dI = dNormal_dU_ * dUv_dI.x + dNormal_dV_ * dUv_dI.y;
}



/** sets dirty flags caused by change in local to world transformation 
 */
void IntersectionContext::localToWorldHasChanged()
{
	LASS_ASSERT(!bsdf_); // bsdf assumes bsdf space remains unchanged
	hasDirtyWorldToLocal_ = true;
	hasDirtyBsdfToWorld_ = true;
	hasDirtyWorldToBsdf_ = true;
}



/** sets dirty flags caused by change in bsdf to local transformation 
 */
void IntersectionContext::bsdfToLocalHasChanged()
{
	LASS_ASSERT(!bsdf_); // bsdf assumes bsdf space remains unchanged
	hasDirtyBsdfToLocal_ = true;
	hasDirtyLocalToBsdf_ = true;
	hasDirtyBsdfToWorld_ = true;
	hasDirtyWorldToBsdf_ = true;
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
