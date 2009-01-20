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
PY_CLASS_MEMBER_RW(Csg, childA, setChildA)
PY_CLASS_MEMBER_RW(Csg, childB, setChildB)
PY_CLASS_MEMBER_RW(Csg, operation, setOperation)
PY_CLASS_STATIC_CONST(Csg, "UNION", "union");
PY_CLASS_STATIC_CONST(Csg, "INTERSECTION", "intersection");
PY_CLASS_STATIC_CONST(Csg, "DIFFERENCE", "difference");

Csg::TOperationDictionary Csg::operationDictionary_ = Csg::makeOperationDictionary();

// --- public --------------------------------------------------------------------------------------

Csg::Csg(const TSceneObjectPtr& iChildA,
		 const TSceneObjectPtr& iChildB):
    childA_(iChildA),
    childB_(iChildB),
	operation_(oUnion)
{
}



Csg::Csg(const TSceneObjectPtr& iChildA,
		 const TSceneObjectPtr& iChildB,
		 const std::string& iOperation):
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



void Csg::setChildA(const TSceneObjectPtr& child)
{
	childA_ = child;
}



void Csg::setChildB(const TSceneObjectPtr& child)
{
	childB_ = child;
}



void Csg::setOperation(const std::string& iOperation)
{
	operation_ = operationDictionary_[stde::tolower(iOperation)];
}



// --- protected -----------------------------------------------------------------------------------

// --- private -------------------------------------------------------------------------------------

void Csg::doAccept(util::VisitorBase& visitor)
{
    preAccept(visitor, *this);
    childA_->accept(visitor);
    childB_->accept(visitor);
	postAccept(visitor, *this);
}



void Csg::doPreProcess(const TSceneObjectPtr& scene, const TimePeriod& period)
{
	childA_->preProcess(scene, period);
	childB_->preProcess(scene, period);
}



void Csg::doIntersect(const Sample& sample, const BoundedRay& ray, 
					  Intersection& result) const
{
	Intersection resultA;
	Intersection resultB;
	Intersection finalResult;
	
	childA_->intersect(sample, ray, resultA);
	childB_->intersect(sample, ray, resultB);
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
					BoundedRay ray(ray.unboundedRay(), resultA.t(), ray.farLimit());
					childA_->intersect(sample, ray, resultB);
					childB_->intersect(sample, ray, resultB);
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
				BoundedRay ray(ray.unboundedRay(), resultA.t(), ray.farLimit());
				childA_->intersect(sample, ray, resultA);
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
				BoundedRay ray(ray.unboundedRay(), resultB.t(), ray.farLimit());
				childB_->intersect(sample, ray, resultB);
			}
		}
	}

	if (!finalResult)
	{
		if (resultB)
		{
			if (childA_->contains(sample, ray.point(resultB.t())) == (operation_ != oUnion))
			{
				finalResult.swap(resultB);
			}
		}
		else if (resultA)
		{
			if (childB_->contains(sample, ray.point(resultB.t())) == (operation_ == oIntersection))
			{
				finalResult.swap(resultA);
			}
		}
	}

	if (finalResult)
	{
		finalResult.push(this);
	}

	result.swap(finalResult);
}




const bool Csg::doIsIntersecting(const Sample& sample, 
								 const BoundedRay& ray) const
{
	Intersection temp;
	intersect(sample, ray, temp);
	return temp;
}



void Csg::doLocalContext(const Sample& sample, const BoundedRay& ray,
								 const Intersection& intersection, 
								 IntersectionContext& result) const
{
    IntersectionDescendor descend(intersection);
	if (intersection.object() == childA_.get())
	{
		childA_->localContext(sample, ray, intersection, result);
	}
	else
	{
		LASS_ASSERT(intersection.object() == childB_.get());
		childB_->localContext(sample, ray, intersection, result);
		/*if (operation_ == oDifference)
		{
			result.flipNormal();
		}*/
	}
}



void Csg::doLocalSpace(TTime time, TTransformation3D& localToWorld) const 
{
}



const bool Csg::doContains(const Sample& sample, const TPoint3D& point) const
{
	switch (operation_)
	{
	case oUnion:
		return childA_->contains(sample, point) || childB_->contains(sample, point);
	case oIntersection:
		return childA_->contains(sample, point) && childB_->contains(sample, point);
	case oDifference:
		return childA_->contains(sample, point) && !childB_->contains(sample, point);
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



const TPyObjectPtr Csg::doGetState() const
{
	return python::makeTuple(childA_, childB_, static_cast<int>(operation_));
}



void Csg::doSetState(const TPyObjectPtr& state)
{
	int operation = 0;
	LASS_ENFORCE(python::decodeTuple(state, childA_, childB_, operation));
	operation_ = static_cast<Operation>(operation);
}



Csg::TOperationDictionary Csg::makeOperationDictionary()
{
	TOperationDictionary result;
	result.enableSuggestions();
	result.add("union", oUnion);
	result.add("intersection", oIntersection);
	result.add("difference", oDifference);
	return result;
}




// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
