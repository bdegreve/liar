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

/** @class liar::SceneObject
 *  @brief base class of all objects in a scene representation
 *  @author Bram de Greve [BdG]
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
	template <typename VisitableType> struct TryParent;
}

class LIAR_KERNEL_DLL SceneObject: public python::PyObjectPlus, public lass::util::VisitableBase<impl::TryParent>
{
    PY_HEADER(python::PyObjectPlus)
public:

    virtual ~SceneObject();

	void preProcess(const TimePeriod& iPeriod);

    void intersect(const Sample& iSample, const BoundedRay& iRay, Intersection& oResult) const;
	void intersect(const Sample& iSample, const DifferentialRay& iRay, 
		Intersection& oResult) const;
	const bool isIntersecting(const Sample& iSample, const BoundedRay& iRay) const ;
	const bool isIntersecting(const Sample& iSample, const DifferentialRay& iRay) const;
	void localContext(const Sample& iSample, const BoundedRay& iRay, 
			const Intersection& iIntersection, IntersectionContext& oResult) const;
	void localContext(const Sample& iSample, const DifferentialRay& iRay, 
			const Intersection& iIntersection, IntersectionContext& oResult) const;
	const bool contains(const Sample& iSample, const TPoint3D& iPoint) const;
    void localSpace(TTime iTime, TTransformation3D& ioLocalToWorld) const;

	const bool hasSurfaceSampling() const;
	const TPoint3D sampleSurface(const TVector2D& iSample, TVector3D& oNormal, 
			TScalar& oPdf) const;
	const TPoint3D sampleSurface(const TVector2D& iSample, const TPoint3D& iTarget, 
			TVector3D& oNormal, TScalar& oPdf) const;
	const TPoint3D sampleSurface(const TVector2D& iSample, const TPoint3D& iTarget,
		const TVector3D& iTargetNormal, TVector3D& oNormal, TScalar& oPdf) const;

    const TAabb3D boundingBox() const;
	const bool hasMotion() const;

	const TScalar area() const;

    const TShaderPtr& shader() const;
    void setShader(const TShaderPtr& iShader);
	const bool isOverridingShader() const;
	void setOverridingShader(bool iEnabled = true);

	const TMediumPtr& interior() const;
	void setInterior(const TMediumPtr& iMedium);

    static const TShaderPtr& defaultShader();
    static void setDefaultShader(const TShaderPtr& i_defaultShader);

	const TPyObjectPtr reduce() const;
	const TPyObjectPtr getState() const;
	void setState(const TPyObjectPtr& iState);
	
protected:

    SceneObject(PyTypeObject* iType);

private:

    TShaderPtr shader_;
	TMediumPtr interior_;
	bool isOverridingShader_;

    LASS_UTIL_ACCEPT_VISITOR;
	virtual void doPreProcess(const TimePeriod& iPeriod);
    virtual void doIntersect(const Sample& iSample, const BoundedRay& iRay, 
			Intersection& oResult) const = 0;
	virtual const bool doIsIntersecting(const Sample& iSample, const BoundedRay& iRay) const = 0;
    virtual void doLocalContext(const Sample& iSample, const BoundedRay& iRay,
			const Intersection& iIntersection, IntersectionContext& oResult) const = 0;
	virtual const bool doContains(const Sample& iSample, const TPoint3D& iPoint) const = 0;
    
	virtual const bool doHasSurfaceSampling() const;
	virtual const TPoint3D doSampleSurface(const TVector2D& iSample, TVector3D& oNormal,
			TScalar& oPdf) const;
	virtual const TPoint3D doSampleSurface(const TVector2D& iSample, const TPoint3D& iTarget,
			TVector3D& oNormal, TScalar& oPdf) const;
	virtual const TPoint3D doSampleSurface(const TVector2D& iSample, const TPoint3D& iTarget,
			const TVector3D& iTargetNormal, TVector3D& oNormal, TScalar& oPdf) const;
	
	virtual const TAabb3D doBoundingBox() const = 0;
	virtual const TScalar doArea() const = 0;
	virtual void doLocalSpace(TTime iTime, TTransformation3D& ioLocalToWorld) const;
	virtual const bool doHasMotion() const;

	virtual const TPyObjectPtr doGetState() const = 0;
	virtual void doSetState(const TPyObjectPtr& iState) = 0;

    static TShaderPtr defaultShader_;
};

/** @relates SceneObject
 */
typedef python::PyObjectPtr<SceneObject>::Type TSceneObjectPtr;

/** Applies a function to all objects in the tree.
 *  @relates SceneObject
 *  @param iObjectTree [in] the root object of the tree to be visited.
 *  @param iFunctor [in] the callback to be invoked
 *
 *  If an SceneObject is instantiated more than once in @a iObjectTree (i.e. it occurs more than
 *	once in the tree), then @a iFunctor is applied to each instance
 */
template <typename Functor>
void forAllObjects(const TSceneObjectPtr& iObjectTree, Functor iFunctor)
{
	class InstanceVisitor: public util::VisitorBase, public util::Visitor<SceneObject>
	{
	public:
		InstanceVisitor(Functor iFun): functor_(iFun) {}
	private:
		void doVisit(SceneObject& iSceneObject) { functor_(iSceneObject); }
		Functor functor_;
	};
	InstanceVisitor visitor(iFunctor);
	iObjectTree->accept(visitor);
}

/** Applies a function to eeach unique object in the tree.
 *  @relates SceneObject
 *  @param iObjectTree [in] the root object of the tree to be visited.
 *  @param iFunctor [in] the callback to be invoked
 *
 *  If an SceneObject is instantiated more than once in @a iObjectTree (i.e. it occurs more than
 *	once in the tree), then @a iFunctor is applied to each instance
 */
template <typename Functor>
void forUniqueObjects(const TSceneObjectPtr& iObjectTree, Functor iFunctor)
{
	class InstanceVisitor: public util::VisitorBase, public util::Visitor<SceneObject>
	{
	public:
		InstanceVisitor(Functor iFun): functor_(iFun) {}
	private:
		void doVisit(SceneObject& iSceneObject)
		{
			if (visited_.count(&iSceneObject) == 0)
			{
				visited_.insert(&iSceneObject);
				functor_(iSceneObject);
			}
		}
		Functor functor_;
		std::set<SceneObject*> visited_;
	};
	InstanceVisitor visitor(iFunctor);
	iObjectTree->accept(visitor);
}



// --- implementation details ----------------------------------------------------------------------

namespace impl
{

template <typename VisitableType>
struct TryParent
{
	static void onUnknownVisitor(VisitableType& iVisited, util::VisitorBase& iVisitor)
	{
		VisitableType::doVisit(static_cast<typename VisitableType::TPyParent&>(iVisited), iVisitor);
	}
	static void onUnknownVisitorOnExit(VisitableType& iVisited, util::VisitorBase& iVisitor)
	{
		VisitableType::doVisitOnExit(static_cast<typename VisitableType::TPyParent&>(iVisited), iVisitor);
	}
};

template <> 
struct TryParent<SceneObject>: public util::VisitNonStrict<SceneObject>
{
};

}

}

}

#include "scene_object.inl"

#endif

// EOF
