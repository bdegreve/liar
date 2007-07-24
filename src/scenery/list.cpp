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

List::List()
{
}



List::List(const TChildren& children):
	children_(children)
{
}



void List::add(const TSceneObjectPtr& child)
{
	children_.push_back(child);
}



void List::add(const TChildren& children)
{
	children_.insert(children_.end(), children.begin(), children.end());
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void List::doAccept(util::VisitorBase& visitor)
{
	doVisit(*this, visitor);
	const TChildren::const_iterator end = children_.end();
	for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
	{
		(*i)->accept(visitor);
	}
	doVisitOnExit(*this, visitor);
}



void List::doPreProcess(const TSceneObjectPtr& scene, const TimePeriod& period)
{
	const TChildren::const_iterator end = children_.end();
	for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
	{
		(*i)->preProcess(scene, period);
	}
}



void List::doIntersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const
{
	Intersection listResult = Intersection::empty();
	const TChildren::const_iterator end = children_.end();
	for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
	{
		Intersection temp;
		const SceneObject* child = i->get();
		LASS_ASSERT(child);
		child->intersect(sample, ray, temp);
		if (temp)
		{
			if (!listResult)
			{
				listResult.swap(temp);
			}
			else if (temp.t() < listResult.t())
			{
				listResult.swap(temp);
			}
		}
	}
	if (listResult)
	{
		listResult.push(this);
	}
	result.swap(listResult);
}



const bool List::doIsIntersecting(const Sample& sample, const BoundedRay& ray) const
{
	const TChildren::const_iterator end = children_.end();
	for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
	{
		const SceneObject* child = i->get();
		LASS_ASSERT(child);
		if (child->isIntersecting(sample, ray))
		{
			return true;
		}
	}
	return false;
}



void List::doLocalContext(
		const Sample& sample, const BoundedRay& ray, const Intersection& intersection,
		IntersectionContext& result) const
{
	IntersectionDescendor descend(intersection);
	intersection.object()->localContext(sample, ray, intersection, result);
}



const bool List::doContains(const Sample& sample, const TPoint3D& point) const
{
	const TChildren::const_iterator end = children_.end();
	for (TChildren::const_iterator i = children_.begin(); i != end; ++i)
	{
		const SceneObject* child = i->get();
		LASS_ASSERT(child);
		if (child->contains(sample, point))
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



void List::doSetState(const TPyObjectPtr& state)
{
	LASS_ENFORCE(python::decodeTuple(state, children_));
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
