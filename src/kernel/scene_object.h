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

/** @class liar::kernel::SceneObject
 *  @brief base class of all objects in a scene representation
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SCENE_OBJECT_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SCENE_OBJECT_H

#include "kernel_common.h"
#include "differential_ray.h"
#include "intersection.h"
#include "intersection_context.h"
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

    void intersect(const Sample& iSample, const BoundedRay& iRay, Intersection& oResult) const 
	{ 
		doIntersect(iSample, iRay, oResult);
	}
	void intersect(const Sample& iSample, const DifferentialRay& iRay, Intersection& oResult) const 
	{ 
		doIntersect(iSample, iRay.centralRay(), oResult); 
	}
	
	const bool isIntersecting(const Sample& iSample, const BoundedRay& iRay) const 
	{
		return doIsIntersecting(iSample, iRay);
	}
	const bool isIntersecting(const Sample& iSample, const DifferentialRay& iRay) const 
	{
		return doIsIntersecting(iSample, iRay.centralRay()); 
	}

	void localContext(const Sample& iSample, const TRay3D& iRay, 
		const Intersection& iIntersection, IntersectionContext& oResult) const
    {
        doLocalContext(iSample, iRay, iIntersection, oResult);
        if (!oResult.shader())
        {
            oResult.setShader(shader_);
        }
    }
	void localContext(const Sample& iSample, const BoundedRay& iRay, 
		const Intersection& iIntersection, IntersectionContext& oResult) const
    {
		localContext(iSample, iRay.unboundedRay(), iIntersection, oResult);
    }
	void localContext(const Sample& iSample, const DifferentialRay& iRay, 
		const Intersection& iIntersection, IntersectionContext& oResult) const
    {
		localContext(iSample, iRay.centralRay(), iIntersection, oResult);
		oResult.setScreenSpaceDifferentials(iRay);
    }

	const bool contains(const Sample& iSample, const TPoint3D& iPoint) const 
	{ 
		return doContains(iSample, iPoint); 
	}

    void localSpace(TTime iTime, TTransformation3D& ioLocalToWorld) const 
	{ 
		doLocalSpace(iTime, ioLocalToWorld); 
	}

    const TAabb3D boundingBox(const TimePeriod& iPeriod) const { return doBoundingBox(iPeriod); }
	const bool hasMotion() const { return doHasMotion(); }

    const TShaderPtr& shader() const;
    void setShader(const TShaderPtr& iShader);

    static const TShaderPtr& defaultShader();
    static void setDefaultShader(const TShaderPtr& i_defaultShader);

protected:

    SceneObject(PyTypeObject* iType);

private:

    TShaderPtr shader_;

    LASS_UTIL_ACCEPT_VISITOR;
    virtual void doIntersect(const Sample& iSample, const BoundedRay& iRay, 
		Intersection& oResult) const = 0;
	virtual const bool doIsIntersecting(const Sample& iSample, const BoundedRay& iRay) const = 0;
    virtual void doLocalContext(const Sample& iSample, const TRay3D& iRay, 
		const Intersection& iIntersection, IntersectionContext& oResult) const = 0;
	virtual const bool doContains(const Sample& iSample, const TPoint3D& iPoint) const = 0;
    virtual const TAabb3D doBoundingBox(const TimePeriod& iPeriod) const = 0;

	virtual void doLocalSpace(TTime iTime, TTransformation3D& ioLocalToWorld) const;
	virtual const bool doHasMotion() const;

    static TShaderPtr defaultShader_;
};

typedef python::PyObjectPtr<SceneObject>::Type TSceneObjectPtr;


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

#endif

// EOF
