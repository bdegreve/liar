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
#include "list.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(List)
PY_CLASS_CONSTRUCTOR_0(List)
PY_CLASS_CONSTRUCTOR_1(List, const List::TChildren&)
PY_CLASS_METHOD_QUALIFIED_1(List, add, void, const TSceneObjectPtr&)
PY_CLASS_METHOD_QUALIFIED_1(List, add, void, const List::TChildren&)


// --- public --------------------------------------------------------------------------------------

List::List():
    SceneObject(&Type)
{
}



List::List(const TChildren& iChildren):
    SceneObject(&Type),
    children_(iChildren)
{
}



void List::add(const TSceneObjectPtr& iChild)
{
    children_.push_back(iChild);
}



void List::add(const TChildren& iChildren)
{
    children_.insert(children_.end(), iChildren.begin(), iChildren.end());
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void List::doAccept(util::VisitorBase& ioVisitor)
{
	doVisit(*this, ioVisitor);
	const TChildren::const_iterator end = children_.end();
    for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
    {
        (*i)->accept(ioVisitor);
    }
	doVisitOnExit(*this, ioVisitor);
}



void List::doPreProcess(const TimePeriod& iPeriod)
{
	const TChildren::const_iterator end = children_.end();
    for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
    {
        (*i)->preProcess(iPeriod);
    }
}



void List::doIntersect(const Sample& iSample, const BoundedRay& iRay,
					   Intersection& oResult) const
{
    Intersection result = Intersection::empty();
	const TChildren::const_iterator end = children_.end();
    for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
    {
        Intersection temp;
		const SceneObject* child = i->get();
		LASS_ASSERT(child);
        child->intersect(iSample, iRay, temp);
        if (temp)
        {
            if (!result)
            {
                result.swap(temp);
            }
            else if (temp.t() < result.t())
            {
                result.swap(temp);
            }
        }
    }
    if (result)
    {
        result.push(this);
    }
    oResult.swap(result);
}



const bool List::doIsIntersecting(const Sample& iSample, 
								  const BoundedRay& iRay) const
{
	const TChildren::const_iterator end = children_.end();
    for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
    {
		const SceneObject* child = i->get();
		LASS_ASSERT(child);
		if (child->isIntersecting(iSample, iRay))
		{
			return true;
		}
	}
	return false;
}



void List::doLocalContext(const Sample& iSample, const BoundedRay& iRay,
                          const Intersection& iIntersection,
                          IntersectionContext& oResult) const
{
    IntersectionDescendor descend(iIntersection);
    iIntersection.object()->localContext(iSample, iRay, iIntersection, oResult);
}



const bool List::doContains(const Sample& iSample, const TPoint3D& iPoint) const
{
	const TChildren::const_iterator end = children_.end();
    for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
    {
		const SceneObject* child = i->get();
		LASS_ASSERT(child);
		if (child->contains(iSample, iPoint))
		{
			return true;
		}
	}
	return false;
}



const TAabb3D List::doBoundingBox() const
{
	TAabb3D result;
	const TChildren::const_iterator end = children_.end();
    for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
    {
		const SceneObject* child = i->get();
		LASS_ASSERT(child);
		result += child->boundingBox();
	}
	return result;
}



const TScalar List::doArea() const
{
	TScalar result = 0;
	for (TChildren::const_iterator i = children_.begin(); i != children_.end(); ++i)
	{
		const SceneObject* child = i->get();
		LASS_ASSERT(child);
		result += child->area();
	}
	return result;
}



const TPyObjectPtr List::doGetState() const
{
	return python::makeTuple(children_);
}



void List::doSetState(const TPyObjectPtr& iState)
{
	LASS_ENFORCE(python::decodeTuple(iState, children_));
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
