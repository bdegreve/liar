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

/** @class liar::Intersection
 *  @brief represents intersection between ray and object
 *  @author Bram de Greve [BdG]
 */

/** @class liar::IntersectionDescendor
 *  @brief automatic descendor of intersection object stack
 *  @author Bram de Greve [BdG]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_INTERSECTION_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_INTERSECTION_H

#include "kernel_common.h"
#include "solid_event.h"
#include <lass/util/non_copyable.h>
#include <lass/num/safe_bool.h>

namespace liar
{
namespace kernel
{

class SceneObject;
class RayTracer;

class LIAR_KERNEL_DLL Intersection
{
public:

	typedef std::size_t TSpecialField;

    Intersection();
    Intersection(const SceneObject* iObject, TScalar iT, SolidEvent iEvent, 
		TSpecialField iSpecial = 0);

	void push(const SceneObject* iObject) { push(iObject, t(), 0); }
    void push(const SceneObject* iObject, TScalar iT, TSpecialField iSpecial = 0);
	const SceneObject* const object() const;
	const TScalar t() const;
	const TSpecialField specialField() const;

	const SolidEvent solidEvent() const { return solidEvent_; }
	void setSolidEvent(SolidEvent iEvent) { solidEvent_ = iEvent; }
	void flipSolidEvent() { solidEvent_ = flip(solidEvent_); }

    const bool isEmpty() const;
	const bool operator!() const { return isEmpty(); }
	operator num::SafeBool() const { return isEmpty() ? num::safeFalse : num::safeTrue; }

    void swap(Intersection& iOther);

    static const Intersection& empty();

private:

    friend class IntersectionDescendor;

	struct IntersectionInfo
	{
		TScalar t;
		const SceneObject* object;
		TSpecialField special;

		IntersectionInfo(const SceneObject* iObject, TScalar iT, TSpecialField iSpecial): 
			t(iT), object(iObject), special(iSpecial)
		{
		}
	};
		
	typedef std::vector<IntersectionInfo> TIntersectionStack;

    void descend() const;
    void ascend() const;

    TIntersectionStack intersectionStack_;
    mutable size_t currentLevel_;
	SolidEvent solidEvent_;

    static Intersection empty_;
};



class LIAR_KERNEL_DLL IntersectionDescendor: public util::NonCopyable
{
public:
    IntersectionDescendor(const Intersection& iIntersection): intersection_(iIntersection) 
    { 
        intersection_.descend(); 
    }
    ~IntersectionDescendor()
    {
        intersection_.ascend();
    }
private:
    const Intersection& intersection_;
};



}

}

#endif

// EOF
