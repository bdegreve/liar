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
    t_(-TNumTraits::one)
{
    currentLevel_ = objectStack_.end();
}



Intersection::Intersection(const SceneObject* iObject, TScalar iT):
    t_(iT)
{
    push(iObject);
}



void Intersection::push(const SceneObject* iObject)
{
    objectStack_.push_back(iObject);
    currentLevel_ = stde::prior(objectStack_.end()); 
}



const SceneObject* const Intersection::object() const
{
    return *currentLevel_;
}



const TScalar Intersection::t() const
{
    return t_;
}



const bool Intersection::isEmpty() const
{
    return t_ <= 0;
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
    std::swap(objectStack_, iOther.objectStack_);
    std::swap(currentLevel_, iOther.currentLevel_);
    std::swap(t_, iOther.t_);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

/** ascend in object stack
 */
void Intersection::descend() const
{
    LASS_ASSERT(currentLevel_ != objectStack_.begin());
    --currentLevel_;
}



/** descend in object stack
 */
void Intersection::ascend() const
{
    ++currentLevel_;
    LASS_ASSERT(currentLevel_ != objectStack_.end());
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
