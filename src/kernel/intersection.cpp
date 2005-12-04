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
	solidEvent_(seNoEvent),
	currentLevel_(0)
{
	intersectionStack_.reserve(4);
}



Intersection::Intersection(const SceneObject* iObject, TScalar iT, SolidEvent iEvent, TSpecialField iSpecial):
	solidEvent_(iEvent),
	currentLevel_(0)
{
	intersectionStack_.reserve(4);
	push(iObject, iT, iSpecial);
}



void Intersection::push(const SceneObject* iObject, TScalar iT, TSpecialField iSpecial)
{
	LASS_ASSERT(iT > 0);
    intersectionStack_.push_back(IntersectionInfo(iObject, iT, iSpecial));
	currentLevel_ = intersectionStack_.size() - 1;
	LASS_ASSERT(intersectionStack_[currentLevel_].t == iT);
}



const SceneObject* const Intersection::object() const
{
    return intersectionStack_[currentLevel_].object;
}



const TScalar Intersection::t() const
{
	LASS_ASSERT(intersectionStack_[currentLevel_].t > 0);
    return intersectionStack_[currentLevel_].t;
}



const Intersection::TSpecialField Intersection::specialField() const
{
    return intersectionStack_[currentLevel_].special;
}



const bool Intersection::isEmpty() const
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
void Intersection::swap(Intersection& iOther)
{
    std::swap(intersectionStack_, iOther.intersectionStack_);
    std::swap(currentLevel_, iOther.currentLevel_);
	std::swap(solidEvent_, iOther.solidEvent_);
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
