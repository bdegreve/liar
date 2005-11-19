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

/** @class liar::IntersectionContext
 *  @brief contains local geometry context of intersection point
 *  @author Bram de Greve [BdG]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_INTERSECTION_CONTEXT_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_INTERSECTION_CONTEXT_H

#pragma once
#include "kernel_common.h"
#include "shader.h"

namespace liar
{
namespace kernel
{

class DifferentialRay;

class LIAR_KERNEL_DLL IntersectionContext
{
public:

	IntersectionContext();

    const TPoint3D& point() const { return point_; }
	const TVector3D& dPoint_dU() const { return dPoint_dU_; }
	const TVector3D& dPoint_dV() const { return dPoint_dV_; }
	const TVector3D& dPoint_dI() const { return dPoint_dI_; }
	const TVector3D& dPoint_dJ() const { return dPoint_dJ_; }
    const TVector3D& normal() const { return normal_; }
	const TVector3D& dNormal_dU() const { return dNormal_dU_; }
	const TVector3D& dNormal_dV() const { return dNormal_dV_; }
	const TVector3D& dNormal_dI() const { return dNormal_dI_; }
	const TVector3D& dNormal_dJ() const { return dNormal_dJ_; }
	const TVector3D& geometricNormal() const { return geometricNormal_; }
	const TPoint2D& uv() const { return uv_; }
	const TVector2D& dUv_dI() const { return dUv_dI_; }
	const TVector2D& dUv_dJ() const { return dUv_dJ_; }
    const TScalar t() const { return t_; }
    const TShaderPtr& shader() const { return shader_; }

    void setPoint(const TPoint3D& iPoint) { point_ = iPoint; }
	void setDPoint_dU(const TVector3D& iDPoint_dU) { dPoint_dU_ = iDPoint_dU; }
	void setDPoint_dV(const TVector3D& iDPoint_dV) { dPoint_dV_ = iDPoint_dV; }
    void setNormal(const TVector3D& iNormal) { normal_ = iNormal; }
	void setDNormal_dU(const TVector3D& iDNormal_dU) { dNormal_dU_ = iDNormal_dU; }
	void setDNormal_dV(const TVector3D& iDNormal_dV) { dNormal_dV_ = iDNormal_dV; }
	void setGeometricNormal(const TVector3D& iNormal) { geometricNormal_ = iNormal; }
	void setUv(const TPoint2D& iUv) { uv_ = iUv; }
	void setUv(const TScalar iU, const TScalar iV) { uv_.x = iU; uv_.y = iV; }
    void setT(TScalar iT) { t_ = iT; }
    void setShader(const TShaderPtr& iShader) { shader_ = iShader; }

	void setScreenSpaceDifferentials(const DifferentialRay& iRay);

	void transform(const TTransformation3D& iTransformation);
	void translate(const TVector3D& iOffset);
	void flipNormal();
	void flipGeometricNormal();

private:

	void setScreenSpaceDifferentialsI(const TRay3D& iRay, TVector3D& oDPoint, 
		TVector3D& oDNormal, TVector2D& oDUv);

    TPoint3D point_;		/**< world space coordinate */
	TVector3D dPoint_dU_;	/**< partial derivative of point_ to surface coordinate u */
	TVector3D dPoint_dV_;	/**< partial derivative of point_ to surface coordinate v */
	TVector3D dPoint_dI_;	/**< partial derivative of point_ to screen space coordinate i */
	TVector3D dPoint_dJ_;	/**< partial derivative of point_ to screen space coordinate j */

	TVector3D normal_;		/**< normal of surface in world space */
	TVector3D dNormal_dU_;	/**< partial derivative of normal_ to surface coordinate u */
	TVector3D dNormal_dV_;	/**< partial derivative of normal_ to surface coordinate v */
	TVector3D dNormal_dI_;	/**< partial derivative of normal_ to screen space coordinate i */
	TVector3D dNormal_dJ_;	/**< partial derivative of normal_ to screen space coordinate j */
	TVector3D geometricNormal_;

	TPoint2D uv_;			/**< parametric coordinate of point_ on surface */
	TVector2D dUv_dI_;		/**< partial derivative of uv_ to screen space coordinate i */
	TVector2D dUv_dJ_;		/**< partial derivative of uv_ to screen space coordinate j */

	TScalar t_;				/**< parameter of point_ on ray */
    TShaderPtr shader_;		/**< shader to be used */

	bool hasScreenSpaceDifferentials_;
};

}

}

#endif

// EOF
