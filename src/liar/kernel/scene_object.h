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

/** @class liar::SceneObject
 *  @brief base class of all objects in a scene representation
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SCENE_OBJECT_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SCENE_OBJECT_H

#include "kernel_common.h"
#include "differential_ray.h"
#include "intersection.h"
#include "intersection_context.h"
#include "medium.h"
#include "sample.h"
#include "shader.h"
#include "time_period.h"

#include <lass/util/visitor.h>

namespace liar
{
namespace kernel
{

namespace impl
{
	template <typename VisitableType>
	struct TryParentHelper
	{
		typedef typename VisitableType::_lassPyParentType TParent;
		static void onUnknownPreVisit(util::VisitorBase& visitor, VisitableType& visited)
		{
			VisitableType::preAccept(visitor, static_cast<TParent&>(visited));
		}
		static void onUnknownPostVisit(util::VisitorBase& visitor, VisitableType& visited)
		{
			VisitableType::postAccept(visitor, static_cast<TParent&>(visited));
		}
	};
	template <>
	struct TryParentHelper<SceneObject>
	{
		static void onUnknownPreVisit(util::VisitorBase&, SceneObject&) {}
		static void onUnknownPostVisit(util::VisitorBase&, SceneObject&) {}
	};
	template <>
	struct TryParentHelper<python::PyObjectPlus>
	{
		static void onUnknownPreVisit(util::VisitorBase&, python::PyObjectPlus&) {}
		static void onUnknownPostVisit(util::VisitorBase&, python::PyObjectPlus&) {}
	};
	struct TryParent
	{
		template <typename VisitableType>
		static void onUnknownPreVisit(util::VisitorBase& visitor, VisitableType& visited)
		{
			TryParentHelper<VisitableType>::onUnknownPreVisit(visitor, visited);
		}
		template <typename VisitableType>
		static void onUnknownPostVisit(util::VisitorBase& visitor, VisitableType& visited)
		{
			TryParentHelper<VisitableType>::onUnknownPostVisit(visitor, visited);
		}
	};
}

class SceneObject;
class SceneLight;

/** @relates SceneObject
 */
typedef python::PyObjectPtr<SceneObject>::Type TSceneObjectPtr;
typedef PyObjectRef<SceneObject> TSceneObjectRef;


class LIAR_KERNEL_DLL SceneObject:
	public python::PyObjectPlus,
	public lass::util::VisitableBase<impl::TryParent>
{
	PY_HEADER(python::PyObjectPlus)
public:

	virtual ~SceneObject();

	void preProcess(const TimePeriod& period);

	void intersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const;
	void intersect(const Sample& sample, const DifferentialRay& ray, Intersection& result) const;
	bool isIntersecting(const Sample& sample, const BoundedRay& ray) const;
	bool isIntersecting(const Sample& sample, const DifferentialRay& ray) const;
	void localContext(const Sample& sample, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const;
	bool contains(const Sample& sample, const TPoint3D& point) const;
	void localSpace(TTime time, TTransformation3D& localToWorld) const;

	bool hasSurfaceSampling() const;
	const TPoint3D sampleSurface(const TPoint2D& sample, TVector3D& normal, TScalar& pdf) const;
	const TPoint3D sampleSurface(const TPoint2D& sample, const TPoint3D& target, TVector3D& normal, TScalar& pdf) const;
	const TPoint3D sampleSurface(const TPoint2D& sample, const TPoint3D& target, const TVector3D& targetNormal, TVector3D& normal, TScalar& pdf) const;
	const TPoint3D sampleSurface(const TPoint2D& sample, const TVector3D& viewDirection, TVector3D& normal, TScalar& pdf) const;
	TScalar angularPdf(const Sample& sample, const TRay3D& ray, BoundedRay& shadowRay, TVector3D& normal) const;

	const TAabb3D boundingBox() const;
	const TSphere3D boundingSphere() const;
	bool hasMotion() const;

	TScalar area() const;
	TScalar area(const TVector3D& normal) const;

	const TShaderPtr& shader() const;
	void setShader(const TShaderPtr& shader);
	bool isOverridingShader() const;
	void setOverridingShader(bool enabled = true);

	const TMediumPtr& interior() const;
	void setInterior(const TMediumPtr& medium);
	bool isOverridingInterior() const;
	void setOverridingInterior(bool enabled = true);

	const SceneLight* asLight() const { return doAsLight(); }

	static const TShaderPtr& defaultShader();
	static void setDefaultShader(const TShaderPtr& defaultShader);

	const TPyObjectPtr reduce() const;
	const TPyObjectPtr getState() const;
	void setState(const TPyObjectPtr& state);

protected:

	SceneObject();

private:

	LASS_UTIL_VISITOR_DO_ACCEPT;

	virtual void doPreProcess(const TimePeriod& period);
	virtual void doIntersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const = 0;
	virtual bool doIsIntersecting(const Sample& sample, const BoundedRay& ray) const = 0;
	virtual void doLocalContext(const Sample& sample, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const = 0;
	virtual bool doContains(const Sample& sample, const TPoint3D& point) const = 0;

	virtual bool doHasSurfaceSampling() const;
	virtual const TPoint3D doSampleSurface(const TPoint2D& sample, TVector3D& normal, TScalar& pdf) const;
	virtual const TPoint3D doSampleSurface(const TPoint2D& sample, const TPoint3D& target, TVector3D& normal, TScalar& pdf) const;
	virtual const TPoint3D doSampleSurface(const TPoint2D& sample, const TPoint3D& target, const TVector3D& targetNormal, TVector3D& normal, TScalar& pdf) const;
	virtual const TPoint3D doSampleSurface(const TPoint2D& sample, const TVector3D& viewDirection, TVector3D& normal, TScalar& pdf) const;
	virtual TScalar doAngularPdf(const Sample& sample, const TRay3D& ray, BoundedRay& shadowRay, TVector3D& normal) const;

	virtual const TAabb3D doBoundingBox() const = 0;
	virtual const TSphere3D doBoundingSphere() const;
	virtual TScalar doArea() const = 0;
	virtual TScalar doArea(const TVector3D& viewDirection) const = 0;
	virtual void doLocalSpace(TTime time, TTransformation3D& localToWorld) const;
	virtual bool doHasMotion() const;
	virtual const SceneLight* doAsLight() const;

	virtual const TPyObjectPtr doGetState() const = 0;
	virtual void doSetState(const TPyObjectPtr& state) = 0;

	TShaderPtr shader_;
	TMediumPtr interior_;
	bool isOverridingShader_;
	bool isOverridingInterior_;

	static TShaderPtr defaultShader_;
};

namespace impl
{
	template <typename Functor>
	class ForAllVisitor: public util::VisitorBase, public util::Visitor<SceneObject>
	{
	public:
		ForAllVisitor(Functor fun): functor_(fun) {}
	private:
		void doPreVisit(SceneObject& object) { functor_(object); }
		Functor functor_;
	};

	template <typename Functor>
	class ForUniqueVisitor: public util::VisitorBase, public util::Visitor<SceneObject>
	{
	public:
		ForUniqueVisitor(Functor fun): functor_(fun) {}
	private:
		void doPreVisit(SceneObject& object)
		{
			if (visited_.count(&object) == 0)
			{
				visited_.insert(&object);
				functor_(object);
			}
		}
		Functor functor_;
		std::set<SceneObject*> visited_;
	};
}

/** Applies a function to all objects in the tree.
 *  @relates SceneObject
 *  @param objectTree [in] the root object of the tree to be visited.
 *  @param functor [in] the callback to be invoked
 *
 *  If an SceneObject is instantiated more than once in @a objectTree (i.e. it occurs more than
 *	once in the tree), then @a functor is applied to each instance
 */
template <typename Functor>
void forAllObjects(const TSceneObjectPtr& objectTree, Functor functor)
{
	impl::ForAllVisitor<Functor> visitor(functor);
	objectTree->accept(visitor);
}

/** Applies a function to eeach unique object in the tree.
 *  @relates SceneObject
 *  @param objectTree [in] the root object of the tree to be visited.
 *  @param functor [in] the callback to be invoked
 *
 *  If an SceneObject is instantiated more than once in @a objectTree (i.e. it occurs more than
 *	once in the tree), then @a functor is applied to each instance
 */
template <typename Functor>
void forUniqueObjects(const TSceneObjectPtr& objectTree, Functor functor)
{
	impl::ForUniqueVisitor<Functor> visitor(functor);
	objectTree->accept(visitor);
}

}

}

#include "scene_object.inl"

#endif

// EOF
