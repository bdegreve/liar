/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
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
 *  http://liar.bramz.org
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

AabbTree::AabbTree()
{
}



AabbTree::AabbTree(const TChildren& children):
    children_(children)
{
}



void AabbTree::add(const TSceneObjectPtr& child)
{
    children_.push_back(child);
}



void AabbTree::add(const TChildren& children)
{
    children_.insert(children_.end(), children.begin(), children.end());
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void AabbTree::doAccept(util::VisitorBase& visitor)
{
	doVisit(*this, visitor);
	const TChildren::const_iterator end = children_.end();
    for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
    {
        (*i)->accept(visitor);
    }
	doVisitOnExit(*this, visitor);
}



void AabbTree::doPreProcess(const TSceneObjectPtr& scene, const TimePeriod& period)
{
	const TChildren::const_iterator end = children_.end();
    for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
    {
        (*i)->preProcess(scene, period);
    }
	tree_.reset(children_.begin(), children_.end());
}



void AabbTree::doIntersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const
{
	Intersection treeResult;
    Info info;
	info.sample = &sample;
	info.intersectionResult = &treeResult;
	TScalar t;
	if (tree_.intersect(ray, t, ray.nearLimit(), &info) != children_.end() && t < ray.farLimit())
	{
		LASS_ASSERT(treeResult);
        treeResult.push(this);
    }
	else
	{
		LASS_ASSERT(!treeResult);
	}
    result.swap(treeResult);
}



const bool AabbTree::doIsIntersecting(const Sample& sample, const BoundedRay& ray) const
{
    Info info;
	info.sample = &sample;
	info.intersectionResult = 0;
	return tree_.intersects(ray, ray.nearLimit(), ray.farLimit(), &info);
}



void AabbTree::doLocalContext(
		const Sample& sample, const BoundedRay& ray, 
		const Intersection& intersection, IntersectionContext& result) const
{
    IntersectionDescendor descend(intersection);
    intersection.object()->localContext(sample, ray, intersection, result);
}



const bool AabbTree::doContains(const Sample& sample, const TPoint3D& point) const
{
    Info info;
	info.sample = &sample;
	info.intersectionResult = 0;
	return tree_.contains(point, &info);
}



const TAabb3D AabbTree::doBoundingBox() const
{
	return tree_.aabb();
}



const TScalar AabbTree::doArea() const
{
	TScalar result = 0;
	for (TChildren::const_iterator i = children_.begin(); i != children_.end(); ++i)
	{
		result += (*i)->area();
	}
	return result;
}



const TPyObjectPtr AabbTree::doGetState() const
{
	return python::makeTuple(children_);
}



void AabbTree::doSetState(const TPyObjectPtr& state)
{
	LASS_ENFORCE(python::decodeTuple(state, children_));
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
