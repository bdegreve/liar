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

/** @class liar::IntersectionContext
 *  @brief contains local geometry context of intersection point
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_INTERSECTION_CONTEXT_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_INTERSECTION_CONTEXT_H

#include "kernel_common.h"
#include "light_sample.h"
#include "shader.h"
#include "medium.h"
#include "solid_event.h"

namespace liar
{
namespace kernel
{

class DifferentialRay;

class LIAR_KERNEL_DLL IntersectionContext
{
public:

	IntersectionContext(const RayTracer* tracer = 0);

	const TAabb3D& bounds() const { return bounds_; }
	void setBounds(const TAabb3D& bounds) { bounds_ = bounds; }

	//@{
	/** 3D position of intersection in the local space of the object that sets the shader.
	 */
	const TPoint3D& point() const { return point_; }
	void setPoint(const TPoint3D& point) { point_ = point; hasDirtyBsdfToWorld_ = true; }
	const TVector3D& dPoint_dU() const { return dPoint_dU_; }
	const TVector3D& dPoint_dV() const { return dPoint_dV_; }
	const TVector3D& dPoint_dI() const { return dPoint_dI_; }
	const TVector3D& dPoint_dJ() const { return dPoint_dJ_; }
	//}@

	const TVector3D& geometricNormal() const { return geometricNormal_; }
	const TVector3D& normal() const { return normal_; }
	const TVector3D& dNormal_dU() const { return dNormal_dU_; }
	const TVector3D& dNormal_dV() const { return dNormal_dV_; }
	const TVector3D& dNormal_dI() const { return dNormal_dI_; }
	const TVector3D& dNormal_dJ() const { return dNormal_dJ_; }
	const TPoint2D& uv() const { return uv_; }
	const TVector2D& dUv_dI() const { return dUv_dI_; }
	const TVector2D& dUv_dJ() const { return dUv_dJ_; }
	TScalar t() const { return t_; }
	const Shader* shader() const { return shader_; }
	const Medium* interior() const { return interior_; }
	SolidEvent solidEvent() const { return solidEvent_; }

	void setDPoint_dU(const TVector3D& dPoint_dU) { dPoint_dU_ = dPoint_dU; hasDirtyBsdfToWorld_ = true; }
	void setDPoint_dV(const TVector3D& dPoint_dV) { dPoint_dV_ = dPoint_dV; hasDirtyBsdfToWorld_ = true; }
	void setDPoint_dI(const TVector3D& dPoint_dI) { dPoint_dI_ = dPoint_dI; }
	void setDPoint_dJ(const TVector3D& dPoint_dJ) { dPoint_dJ_ = dPoint_dJ; }
	void setGeometricNormal(const TVector3D& geometricNormal) { geometricNormal_ = geometricNormal; }
	void setNormal(const TVector3D& normal) { normal_ = normal; hasDirtyBsdfToWorld_ = true; }
	void setDNormal_dU(const TVector3D& dNormal_dU) { dNormal_dU_ = dNormal_dU; }
	void setDNormal_dV(const TVector3D& dNormal_dV) { dNormal_dV_ = dNormal_dV; }
	void setUv(const TPoint2D& uv) { uv_ = uv; }
	void setUv(const TScalar u, const TScalar iV) { uv_.x = u; uv_.y = iV; }
	void setDUv_dI(const TVector2D& dUv_dI) { dUv_dI_ = dUv_dI; }
	void setDUv_dJ(const TVector2D& dUv_dJ) { dUv_dJ_ = dUv_dJ; }
	void setT(TScalar t) { t_ = t; }
	void setShader(const Shader* shader);
	void setShader(const TShaderPtr& shader) { setShader(shader.get()); }
	void setInterior(const Medium* interior) { interior_ = interior; }
	void setInterior(const TMediumPtr& interior) { setInterior(interior.get()); }
	void setSolidEvent(SolidEvent solidEvent) { solidEvent_ = solidEvent; }

	void setScreenSpaceDifferentials(const DifferentialRay& ray);
	bool hasScreenSpaceDifferentials() const { return hasScreenSpaceDifferentials_; }

	void transformBy(const TTransformation3D& transformation);
	void translateBy(const TVector3D& offset);
	const TVector3D flipTo(const TVector3D& worldOmega);

	const TTransformation3D& bsdfToWorld() const;
	const TTransformation3D worldToBsdf() const { return bsdfToWorld().inverse(); }
	const TVector3D bsdfToWorld(const TVector3D& v) const { return prim::transform(v, bsdfToWorld()); }
	const TVector3D worldToBsdf(const TVector3D& v) const { return prim::transform(v, worldToBsdf()); }

	const TTransformation3D& localToWorld() const { return localToWorld_; }
	const TTransformation3D worldToLocal() const { return localToWorld().inverse(); }
	//const TVector3D localToWorld(const TVector3D& v) const { return prim::transform(v, localToWorld()); }
	//const TVector3D worldToLocal(const TVector3D& v) const { return prim::transform(v, worldToLocal()); }

private:

	void setScreenSpaceDifferentialsI(const TRay3D& ray, TVector3D& oDPoint, 
		TVector3D& oDNormal, TVector2D& oDUv);
	void generateShaderToWorld();

	TAabb3D bounds_;

	TPoint3D point_;		/**< world space coordinate */
	TVector3D dPoint_dU_;	/**< partial derivative of point_ to surface coordinate u */
	TVector3D dPoint_dV_;	/**< partial derivative of point_ to surface coordinate v */
	TVector3D dPoint_dI_;	/**< partial derivative of point_ to screen space coordinate i */
	TVector3D dPoint_dJ_;	/**< partial derivative of point_ to screen space coordinate j */

	TVector3D geometricNormal_;	/**< normal of underlying geometry.  Do we need it?  Yes, think 
			triangle normal in triangle meshes ...*/
	TVector3D normal_;		/**< normal of surface in world space */
	TVector3D dNormal_dU_;	/**< partial derivative of normal_ to surface coordinate u */
	TVector3D dNormal_dV_;	/**< partial derivative of normal_ to surface coordinate v */
	TVector3D dNormal_dI_;	/**< partial derivative of normal_ to screen space coordinate i */
	TVector3D dNormal_dJ_;	/**< partial derivative of normal_ to screen space coordinate j */

	TPoint2D uv_;			/**< parametric coordinate of point_ on surface */
	TVector2D dUv_dI_;		/**< partial derivative of uv_ to screen space coordinate i */
	TVector2D dUv_dJ_;		/**< partial derivative of uv_ to screen space coordinate j */

	TScalar t_;				/**< parameter of point_ on ray */
	const Shader* shader_;		/**< shader to be used */
	const Medium* interior_;
	const RayTracer* tracer_;	/**< tracer to be used */

	TTransformation3D shaderToWorld_;
	TTransformation3D localToWorld_;
	mutable TTransformation3D bsdfToWorld_;

	SolidEvent solidEvent_;
	bool hasScreenSpaceDifferentials_;
	mutable bool hasDirtyBsdfToWorld_;
};

}

}

#endif

// EOF
