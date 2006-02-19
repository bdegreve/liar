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

#include "scenery_common.h"
#include "csg.h"
#include <lass/stde/extended_string.h>

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(Csg)
PY_CLASS_CONSTRUCTOR_2(Csg, const TSceneObjectPtr&, const TSceneObjectPtr&)
PY_CLASS_CONSTRUCTOR_3(Csg, const TSceneObjectPtr&, const TSceneObjectPtr&, const std::string&)
PY_CLASS_MEMBER_RW(Csg, "childA", childA, setChildA)
PY_CLASS_MEMBER_RW(Csg, "childB", childB, setChildB)
PY_CLASS_MEMBER_RW(Csg, "operation", operation, setOperation)
PY_CLASS_STATIC_CONST(Csg, "UNION", "union");
PY_CLASS_STATIC_CONST(Csg, "INTERSECTION", "intersection");
PY_CLASS_STATIC_CONST(Csg, "DIFFERENCE", "difference");

Csg::TOperationDictionary Csg::operationDictionary_ = Csg::makeOperationDictionary();

// --- public --------------------------------------------------------------------------------------

Csg::Csg(const TSceneObjectPtr& iChildA,
		 const TSceneObjectPtr& iChildB):
    SceneObject(&Type),
    childA_(iChildA),
    childB_(iChildB),
	operation_(oUnion)
{
}



Csg::Csg(const TSceneObjectPtr& iChildA,
		 const TSceneObjectPtr& iChildB,
		 const std::string& iOperation):
    SceneObject(&Type),
    childA_(iChildA),
    childB_(iChildB)
{
	setOperation(iOperation);
}



const TSceneObjectPtr& Csg::childA() const
{
	return childA_;
}



const TSceneObjectPtr& Csg::childB() const
{
	return childB_;
}



const std::string Csg::operation() const
{
	LASS_ASSERT(operationDictionary_.isValue(operation_));
	return operationDictionary_.key(operation_);
}



void Csg::setChildA(const TSceneObjectPtr& iChild)
{
	childA_ = iChild;
}



void Csg::setChildB(const TSceneObjectPtr& iChild)
{
	childB_ = iChild;
}



void Csg::setOperation(const std::string& iOperation)
{
	operation_ = operationDictionary_[stde::tolower(iOperation)];
}



// --- protected -----------------------------------------------------------------------------------

// --- private -------------------------------------------------------------------------------------

void Csg::doAccept(util::VisitorBase& ioVisitor)
{
    doVisit(*this, ioVisitor);
    childA_->accept(ioVisitor);
    childB_->accept(ioVisitor);
	doVisitOnExit(*this, ioVisitor);
}



void Csg::doPreProcess(const TimePeriod& iPeriod)
{
	childA_->preProcess(iPeriod);
	childB_->preProcess(iPeriod);
}



void Csg::doIntersect(const Sample& iSample, const BoundedRay& iRay, 
					  Intersection& oResult) const
{
	Intersection resultA;
	Intersection resultB;
	Intersection finalResult;
	
	childA_->intersect(iSample, iRay, resultA);
	childB_->intersect(iSample, iRay, resultB);
	while (!finalResult && (resultA && resultB))
	{
		if (num::almostEqual(resultA.t(), resultB.t(), tolerance))
		{
			LASS_META_ASSERT(seNoEvent == 0 && seEntering == 1 && 
				seLeaving == 2 && numSolidEvent == 3,
				SolidEvent_has_different_values_than_assumed);
			LASS_META_ASSERT(oUnion == 0 && oIntersection == 1 && oDifference == 2 && 
				numOperation == 3,
				Operation_has_different_values_than_assumed);

			static int whichOne[3][9] = 
			{ 
				{ 1, 2, 0, 1, 1, 1, 0, 2, 1 },
				{ 1, 1, 1, 2, 1, 2, 1, 1, 1 },
				{ 0, 0, 1, 1, 0, 1, 1, 0, 0 }
			};
			static int event[3][9] = 
			{ 
				{ 2, 2, 0, 2, 0, 1, 0, 1, 1 },
				{ 2, 0, 0, 0, 0, 0, 0, 0, 1 },
				{ 0, 0, 1, 2, 0, 1, 2, 0, 0 }
			};

			const int i = resultB.solidEvent() * 3 + resultA.solidEvent();
			switch (whichOne[operation_][i])
			{
			case 0:
				{
					BoundedRay ray(iRay.unboundedRay(), resultA.t(), iRay.farLimit());
					childA_->intersect(iSample, ray, resultB);
					childB_->intersect(iSample, ray, resultB);
				}
				break;
			case 1:
				finalResult.swap(resultA);
				finalResult.setSolidEvent(static_cast<SolidEvent>(event[operation_][i]));
				break;
			case 2:
				finalResult.swap(resultB);
				finalResult.setSolidEvent(static_cast<SolidEvent>(event[operation_][i]));
				break;
			default:
				LASS_ASSERT_UNREACHABLE;
			}
		}
		else if (resultA.t() < resultB.t())
		{
			// union: point on A must not be in B
			// intersection: point on A must be in B
			// difference: point on A must not be in B
			const bool insideB = resultB.solidEvent() == seLeaving;
			if (insideB == (operation_ == oIntersection))
			{
				finalResult.swap(resultA);
			}
			else
			{
				BoundedRay ray(iRay.unboundedRay(), resultA.t(), iRay.farLimit());
				childA_->intersect(iSample, ray, resultA);
			}
		}
		else // if (resultB.t() < resultA.t())
		{
			LASS_ASSERT(resultB.t() < resultA.t());

			// union: point on B must not be in A
			// intersection: point on B must be in A
			// difference: point on B must be in A
			const bool insideA = resultA.solidEvent() == seLeaving;
			if (insideA == (operation_ != oUnion))
			{
				finalResult.swap(resultB);
			}
			else
			{
				BoundedRay ray(iRay.unboundedRay(), resultB.t(), iRay.farLimit());
				childB_->intersect(iSample, ray, resultB);
			}
		}
	}

	if (!finalResult)
	{
		if (resultB)
		{
			if (childA_->contains(iSample, iRay.point(resultB.t())) == (operation_ != oUnion))
			{
				finalResult.swap(resultB);
			}
		}
		else if (resultA)
		{
			if (childB_->contains(iSample, iRay.point(resultB.t())) == (operation_ == oIntersection))
			{
				finalResult.swap(resultA);
			}
		}
	}

	if (finalResult)
	{
		finalResult.push(this);
	}

	oResult.swap(finalResult);
}




const bool Csg::doIsIntersecting(const Sample& iSample, 
								 const BoundedRay& iRay) const
{
	Intersection temp;
	intersect(iSample, iRay, temp);
	return temp;
}



void Csg::doLocalContext(const Sample& iSample, const BoundedRay& iRay,
								 const Intersection& iIntersection, 
								 IntersectionContext& oResult) const
{
    IntersectionDescendor descend(iIntersection);
	if (iIntersection.object() == childA_.get())
	{
		childA_->localContext(iSample, iRay, iIntersection, oResult);
	}
	else
	{
		LASS_ASSERT(iIntersection.object() == childB_.get());
		childB_->localContext(iSample, iRay, iIntersection, oResult);
		if (operation_ == oDifference)
		{
			oResult.flipGeometricNormal();
		}
	}
}



void Csg::doLocalSpace(TTime iTime, TTransformation3D& ioLocalToWorld) const 
{
}



const bool Csg::doContains(const Sample& iSample, const TPoint3D& iPoint) const
{
	switch (operation_)
	{
	case oUnion:
		return childA_->contains(iSample, iPoint) || childB_->contains(iSample, iPoint);
	case oIntersection:
		return childA_->contains(iSample, iPoint) && childB_->contains(iSample, iPoint);
	case oDifference:
		return childA_->contains(iSample, iPoint) && !childB_->contains(iSample, iPoint);
	default:
		LASS_ASSERT_UNREACHABLE;
		return false;
	}
}



const TAabb3D Csg::doBoundingBox() const
{
	switch (operation_)
	{
	case oUnion:
		return childA_->boundingBox() + childB_->boundingBox();
	case oIntersection:
		{
			TAabb3D result;
			prim::intersect(childA_->boundingBox(), childB_->boundingBox(), result);
			return result;
		}
	case oDifference:
		return childA_->boundingBox();
	default:
		LASS_ASSERT_UNREACHABLE;
		return TAabb3D();
	}
}



const TScalar Csg::doArea() const
{
	LASS_THROW("not implemented yet");
	return TNumTraits::qNaN;
}



Csg::TOperationDictionary Csg::makeOperationDictionary()
{
	TOperationDictionary result;
	result.add("union", oUnion);
	result.add("intersection", oIntersection);
	result.add("difference", oDifference);
	return result;
}




// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
