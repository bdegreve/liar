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
#include "aabb_tree.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(AabbTree)
PY_CLASS_CONSTRUCTOR_0(AabbTree)
PY_CLASS_CONSTRUCTOR_1(AabbTree, const AabbTree::TChildren&)
PY_CLASS_METHOD_QUALIFIED_1(AabbTree, add, void, const TSceneObjectPtr&)
PY_CLASS_METHOD_QUALIFIED_1(AabbTree, add, void, const AabbTree::TChildren&)


// --- public --------------------------------------------------------------------------------------

AabbTree::AabbTree():
    SceneObject(&Type)
{
}



AabbTree::AabbTree(const TChildren& iChildren):
    SceneObject(&Type),
    children_(iChildren)
{
}



void AabbTree::add(const TSceneObjectPtr& iChild)
{
    children_.push_back(iChild);
}



void AabbTree::add(const TChildren& iChildren)
{
    children_.insert(children_.end(), iChildren.begin(), iChildren.end());
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void AabbTree::doAccept(util::VisitorBase& ioVisitor)
{
	doVisit(*this, ioVisitor);
	const TChildren::const_iterator end = children_.end();
    for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
    {
        (*i)->accept(ioVisitor);
    }
	doVisitOnExit(*this, ioVisitor);
}



void AabbTree::doPreProcess(const TimePeriod& iPeriod)
{
	const TChildren::const_iterator end = children_.end();
    for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
    {
        (*i)->preProcess(iPeriod);
    }
	tree_.reset(children_.begin(), children_.end());
}



void AabbTree::doIntersect(const Sample& iSample, const BoundedRay& iRay,
					   Intersection& oResult) const
{
	Intersection result;
    Info info;
	info.sample = &iSample;
	info.intersectionResult = &result;
	TScalar t;
	if (tree_.intersect(iRay, t, iRay.nearLimit(), &info) != children_.end())
	{
		LASS_ASSERT(result);
        result.push(this);
    }
	else
	{
		LASS_ASSERT(!result);
	}
    oResult.swap(result);
}



const bool AabbTree::doIsIntersecting(const Sample& iSample, 
								  const BoundedRay& iRay) const
{
    Info info;
	info.sample = &iSample;
	info.intersectionResult = 0;
	return tree_.intersects(iRay, iRay.nearLimit(), iRay.farLimit(), &info);
}



void AabbTree::doLocalContext(const Sample& iSample, const BoundedRay& iRay,
                          const Intersection& iIntersection,
                          IntersectionContext& oResult) const
{
    IntersectionDescendor descend(iIntersection);
    iIntersection.object()->localContext(iSample, iRay, iIntersection, oResult);
}



const bool AabbTree::doContains(const Sample& iSample, const TPoint3D& iPoint) const
{
    Info info;
	info.sample = &iSample;
	info.intersectionResult = 0;
	return tree_.contains(iPoint, &info);
}



const TAabb3D AabbTree::doBoundingBox() const
{
	return tree_.aabb();
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
