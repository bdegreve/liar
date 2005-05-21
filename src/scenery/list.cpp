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
PY_CLASS_METHOD_QUALIFIED_1(List, add, void, const kernel::TSceneObjectPtr&)
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



void List::add(const kernel::TSceneObjectPtr& iChild)
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



void List::doIntersect(const TRay3D& iRay, kernel::Intersection& oResult) const
{
    kernel::Intersection result = kernel::Intersection::empty();
	const TChildren::const_iterator end = children_.end();
    for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
    {
        TChildren::value_type child = *i;
        kernel::Intersection temp;
        child->intersect(iRay, temp);
        if (!temp.isEmpty())
        {
            if (result.isEmpty() || temp.t() < result.t())
            {
                result.swap(temp);
            }
        }
    }
    if (!result.isEmpty())
    {
        result.push(this);
    }
    oResult.swap(result);
}



const bool List::doIsIntersecting(const TRay3D& iRay, TScalar iMaxT,
								  const SceneObject* iExcludeA, const SceneObject* iExcludeB) const
{
	const TChildren::const_iterator end = children_.end();
    for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
    {
		if ((*i)->isIntersecting(iRay, iMaxT, iExcludeA, iExcludeB))
		{
			return true;
		}
	}
	return false;
}



void List::doLocalContext(const TRay3D& iRay, 
                          const kernel::Intersection& iIntersection,
                          kernel::IntersectionContext& oResult) const
{
    kernel::IntersectionDescendor descend(iIntersection);
    iIntersection.object()->localContext(iRay, iIntersection, oResult);
}



const TAabb3D List::doBoundingBox() const
{
	TAabb3D result;
	const TChildren::const_iterator end = children_.end();
    for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
    {
		result += (*i)->boundingBox();
	}
	return result;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
