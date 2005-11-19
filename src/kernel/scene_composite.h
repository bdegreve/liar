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

/** @class liar::SceneComposite
 *  @brief base class of all compound object (objects that have children)
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNALL_SCENE_COMPOSITE_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNALL_SCENE_COMPOSITE_H

#include "kernel_common.h"
#include "scene_object.h"

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL SceneComposite: public SceneObject
{
    PY_HEADER(SceneObject)
public:

protected:
    SceneComposite(PyTypeObject* iType);

private:
    virtual void doAccept(util::VisitorBase& iVisitor);
    virtual void doIntersect(const TRay3D& iRay, Intersection& oResult) const = 0;
    
    virtual void doAcceptInChildren(util::VisitorBase& iVisitor) = 0;
};



}

}

#endif

// EOF
