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

/** @class liar::scenery::List
 *  @brief compound of some child scenery in a linear list.
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_LIST_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_LIST_H

#include "scenery_common.h"
#include "../kernel/scene_object.h"

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL List: public kernel::SceneObject
{
    PY_HEADER(kernel::SceneObject)
public:

    typedef std::vector<kernel::TSceneObjectPtr> TChildren;

    List();
    List(const TChildren& iChildren); // for python

    template <typename InputIterator> List(InputIterator iBegin, InputIterator iEnd):
        SceneComposite(&Type)
    {
        add(iBegin, iEnd);
    }

    void add(const kernel::TSceneObjectPtr& iChild);
	void add(const TChildren& iChildren); // for python

	template <typename InputIterator> void add(InputIterator iBegin, InputIterator iEnd)
	{
        std::copy(iBegin, iEnd, std::back_inserter(children_));
	}

private:

    void doAccept(lass::util::VisitorBase& ioVisitor);
    void doIntersect(const TRay3D& iRay, kernel::Intersection& oResult) const;
	const bool doIsIntersecting(const TRay3D& iRay, TScalar iMaxT,
		const SceneObject* iExcludeA, const SceneObject* iExcludeB) const;
	void doLocalContext(const TRay3D& iRay, const kernel::Intersection& iIntersection, 
		kernel::IntersectionContext& oResult) const;
    const TAabb3D doBoundingBox() const;

    TChildren children_;   
};



}

}

#endif

// EOF
