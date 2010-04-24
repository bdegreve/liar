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

#include "kernel_common.h"
#include "intersection.h"
#include "intersection_context.h"
#include "scene_object.h"
#include <lass/stde/extended_iterator.h>

namespace liar
{
namespace kernel
{

Intersection Intersection::empty_;



// --- public --------------------------------------------------------------------------------------

Intersection::Intersection():
	currentLevel_(0),
	solidEvent_(seNoEvent)
{
	intersectionStack_.reserve(4);
}



Intersection::Intersection(const SceneObject* object, TScalar t, SolidEvent event, TSpecialField special):
	currentLevel_(0),
	solidEvent_(event)
{
	intersectionStack_.reserve(4);
	push(object, t, special);
}



void Intersection::push(const SceneObject* object, TScalar t, TSpecialField special)
{
	LASS_ASSERT(t > 0);
	intersectionStack_.push_back(IntersectionInfo(object, t, special));
	currentLevel_ = intersectionStack_.size() - 1;
	LASS_ASSERT(intersectionStack_[currentLevel_].t == t);
}



const SceneObject* Intersection::object() const
{
	return intersectionStack_[currentLevel_].object;
}



TScalar Intersection::t() const
{
	if (isEmpty())
	{
		return TNumTraits::infinity;
	}
	LASS_ASSERT(intersectionStack_[currentLevel_].t > 0);
	return intersectionStack_[currentLevel_].t;
}



Intersection::TSpecialField Intersection::specialField() const
{
	return intersectionStack_[currentLevel_].special;
}


size_t Intersection::level() const
{
	return currentLevel_;
}



bool Intersection::isEmpty() const
{
	LASS_ASSERT(intersectionStack_.empty() || intersectionStack_[currentLevel_].t > 0);
	return intersectionStack_.empty();
}



/** return an empty intersection.
 */
const Intersection& Intersection::empty()
{
	return empty_;
}



/** swap data with other intersection
 */
void Intersection::swap(Intersection& other)
{
	std::swap(intersectionStack_, other.intersectionStack_);
	std::swap(currentLevel_, other.currentLevel_);
	std::swap(solidEvent_, other.solidEvent_);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

/** ascend in object stack
 */
void Intersection::descend() const
{
	LASS_ASSERT(currentLevel_ > 0);
	--currentLevel_;
}



/** descend in object stack
 */
void Intersection::ascend() const
{
	++currentLevel_;
	LASS_ASSERT(currentLevel_ != intersectionStack_.size());
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
