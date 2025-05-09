/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2025  Bram de Greve (bramz@users.sourceforge.net)
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
#include "matrix_4x4.h"
#include "differential_ray.h"

namespace liar
{
namespace kernel
{

namespace experimental
{
	template <typename T>
	class ResetOnCopy
	{
	public:
		ResetOnCopy(): value_() {}
		ResetOnCopy(const T& x): value_(x) {}
		ResetOnCopy(T&& x) : value_(std::forward<T>(x)) {}
		ResetOnCopy(const ResetOnCopy&): value_() {}
		ResetOnCopy& operator=(const T& x) { value_ = x; return *this; }
		ResetOnCopy& operator=(T&& x) { value_ = std::forward<T>(x); return *this; }
		ResetOnCopy& operator=(const ResetOnCopy& /*other*/) { value_ = T(); return *this; }
		T& operator*() { return value_; }
		bool operator!() const { return !value_; }
	private:
		T value_;
	};
}


class LIAR_KERNEL_DLL IntersectionContext
{
public:

	class LIAR_KERNEL_DLL Transformation
	{
	public:
		using TMatrix = Matrix4x4<TScalar>;

		TVector3D transform(const TVector3D& v) const { return (*forward_) * v; }
		TPoint3D transform(const TPoint3D& p) const { return (*forward_) * p; }
		TRay3D transform(const TRay3D& ray) const { return kernel::transform(ray, *forward_); }
		TRay3D transform(const TRay3D& ray, TScalar& tScale) const { return kernel::transform(ray, *forward_, tScale); }
		BoundedRay transform(const BoundedRay& ray) const { return kernel::transform(ray, *forward_); }
		DifferentialRay transform(const DifferentialRay& ray) const { return kernel::transform(ray, *forward_); }
		TVector3D normalTransform(const TVector3D& n) const
		{
			invert();
			return n * (*reverse_);
		}

		const TMatrix& matrix() const { return *forward_; }

		Transformation inverse() const
		{
			invert();
			return Transformation(reverse_, forward_, nullptr);
		}

	private:
		friend class IntersectionContext;

		Transformation(TMatrix* forward, TMatrix* reverse, bool* reverseIsDirty);
		void invert() const;

		TMatrix* forward_;
		TMatrix* reverse_;
		bool* reverseIsDirty_;
	};

	IntersectionContext(const SceneObject& object, const Sample& sample, const BoundedRay& primaryRay, const Intersection& intersection, size_t rayGeneration);
	IntersectionContext(const SceneObject& object, const Sample& sample, const DifferentialRay& primaryRay, const Intersection& intersection, size_t rayGeneration);
	IntersectionContext(const IntersectionContext& other) = default;

	const kernel::Sample& sample() const { return sample_; }
	const SceneObject& object() const { LASS_ASSERT(object_); return *object_; }

	const TAabb3D& bounds() const { return bounds_; }
	void setBounds(const TAabb3D& bounds) { bounds_ = bounds; }

	//@{
	/** 3D position of intersection in the local space of the object that sets the shader.
	 */
	const TPoint3D& point() const { return point_; }
	const TVector3D& dPoint_dU() const { return dPoint_dU_; }
	const TVector3D& dPoint_dV() const { return dPoint_dV_; }
	const TVector3D& dPoint_dI() const { return dPoint_dI_; }
	const TVector3D& dPoint_dJ() const { return dPoint_dJ_; }
	void setPoint(const TPoint3D& point);
	void setDPoint_dU(const TVector3D& dPoint_dU);
	void setDPoint_dV(const TVector3D& dPoint_dV);
	void setDPoint_dI(const TVector3D& dPoint_dI);
	void setDPoint_dJ(const TVector3D& dPoint_dJ);
	//}@

	//@{
	/** surface normal of intersection in the local space of the object that sets the shader.
	 */
	const TVector3D& geometricNormal() const { return geometricNormal_; }
	const TVector3D& normal() const { return normal_; }
	const TVector3D& dNormal_dU() const { return dNormal_dU_; }
	const TVector3D& dNormal_dV() const { return dNormal_dV_; }
	const TVector3D& dNormal_dI() const { return dNormal_dI_; }
	const TVector3D& dNormal_dJ() const { return dNormal_dJ_; }
	//}@

	const TPoint2D& uv() const { return uv_; }
	const TVector2D& dUv_dI() const { return dUv_dI_; }
	const TVector2D& dUv_dJ() const { return dUv_dJ_; }
	TScalar t() const { return t_; }
	const Shader* shader() const { return shader_; }
	const TBsdfPtr& bsdf() const;
	const Medium* interior() const { return interior_; }
	SolidEvent solidEvent() const { return solidEvent_; }
	size_t rayGeneration() const { return rayGeneration_; }

	void setGeometricNormal(const TVector3D& geometricNormal);
	void setNormal(const TVector3D& normal);
	void setDNormal_dU(const TVector3D& dNormal_dU) { LIAR_ASSERT_FINITE(dNormal_dU); dNormal_dU_ = dNormal_dU; }
	void setDNormal_dV(const TVector3D& dNormal_dV) { LIAR_ASSERT_FINITE(dNormal_dV); dNormal_dV_ = dNormal_dV; }
	void setUv(const TPoint2D& uv) { uv_ = uv; }
	void setUv(const TScalar u, const TScalar iV) { uv_.x = u; uv_.y = iV; }
	void setDUv_dI(const TVector2D& dUv_dI) { LIAR_ASSERT_FINITE(dUv_dI); dUv_dI_ = dUv_dI; }
	void setDUv_dJ(const TVector2D& dUv_dJ) { LIAR_ASSERT_FINITE(dUv_dJ); dUv_dJ_ = dUv_dJ; }
	void setT(TScalar t) { t_ = t; }
	void setShader(const Shader* shader);
	void setShader(const TShaderPtr& shader) { setShader(shader.get()); }
	void setInterior(const Medium* interior) { interior_ = interior; }
	void setInterior(const TMediumPtr& interior) { setInterior(interior.get()); }
	void setSolidEvent(SolidEvent solidEvent) { solidEvent_ = solidEvent; }

	bool hasScreenSpaceDifferentials() const { return hasScreenSpaceDifferentials_; }

	void transformBy(const TTransformation3D& transformation);
	void translateBy(const TVector3D& offset);

	Transformation localToWorld() const { return Transformation(const_cast<TMatrix*>(&localToWorld_), &worldToLocal_, &hasDirtyWorldToLocal_); }
	Transformation worldToLocal() const;
	const TVector3D worldNormal() const;
	const TVector3D worldGeometricNormal() const;

	Transformation bsdfToLocal() const;
	Transformation localToBsdf() const;

	Transformation bsdfToWorld() const;
	Transformation worldToBsdf() const;
	const TVector3D bsdfToWorld(const TVector3D& v) const { return bsdfToWorld().transform(v).normal(); }
	const TVector3D worldToBsdf(const TVector3D& v) const { return worldToBsdf().transform(v).normal(); }

private:

	using TMatrix = Transformation::TMatrix;

	void init(const SceneObject& object, const BoundedRay& primaryRay, const Intersection& intersection, size_t rayGeneration);
	void flipTo(const TVector3D& worldOmega);
	void setScreenSpaceDifferentialsI(const TRay3D& ray, TVector3D& dPoint, TVector3D& dNormal, TVector2D& dUv);
	void generateShaderToWorld();

	void localToWorldHasChanged();
	void bsdfToLocalHasChanged();

	TMatrix localToWorld_;
	mutable TMatrix worldToLocal_;
	mutable TMatrix bsdfToLocal_;
	mutable TMatrix localToBsdf_;
	mutable TMatrix bsdfToWorld_;
	mutable TMatrix worldToBsdf_;

	TAabb3D bounds_;

	TPoint3D point_;		/**< world space coordinate */
	TVector3D dPoint_dU_;	/**< partial derivative of point_ to surface coordinate u */
	TVector3D dPoint_dV_;	/**< partial derivative of point_ to surface coordinate v */
	TVector3D dPoint_dI_;	/**< partial derivative of point_ to screen space coordinate i */
	TVector3D dPoint_dJ_;	/**< partial derivative of point_ to screen space coordinate j */

	TVector3D geometricNormal_;	/**< normal of underlying geometry.  Do we need it?  Yes, think triangle normal in triangle meshes ...*/
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
	const SceneObject* object_;
	const kernel::Sample& sample_;
	mutable experimental::ResetOnCopy<TBsdfPtr> bsdf_;

	size_t rayGeneration_;
	SolidEvent solidEvent_;
	bool hasScreenSpaceDifferentials_;
	mutable bool hasDirtyWorldToLocal_;
	mutable bool hasDirtyBsdfToLocal_;
	mutable bool hasDirtyLocalToBsdf_;
	mutable bool hasDirtyBsdfToWorld_;
	mutable bool hasDirtyWorldToBsdf_;
};

static_assert(alignof(IntersectionContext) >= alignof(IntersectionContext::Transformation::TMatrix));

}

}

#endif

// EOF
