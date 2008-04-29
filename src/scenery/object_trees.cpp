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
#include "object_trees.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(AabbTree)
PY_CLASS_CONSTRUCTOR_0(AabbTree)
PY_CLASS_CONSTRUCTOR_1(AabbTree, const AabbTree::TChildren&)
PY_CLASS_METHOD_QUALIFIED_1(AabbTree, add, void, const TSceneObjectPtr&)
PY_CLASS_METHOD_QUALIFIED_1(AabbTree, add, void, const AabbTree::TChildren&)

PY_DECLARE_CLASS(AabpTree)
PY_CLASS_CONSTRUCTOR_0(AabpTree)
PY_CLASS_CONSTRUCTOR_1(AabpTree, const AabpTree::TChildren&)
PY_CLASS_METHOD_QUALIFIED_1(AabpTree, add, void, const TSceneObjectPtr&)
PY_CLASS_METHOD_QUALIFIED_1(AabpTree, add, void, const AabpTree::TChildren&)

PY_DECLARE_CLASS(OctTree)
PY_CLASS_CONSTRUCTOR_0(OctTree)
PY_CLASS_CONSTRUCTOR_1(OctTree, const OctTree::TChildren&)
PY_CLASS_METHOD_QUALIFIED_1(OctTree, add, void, const TSceneObjectPtr&)
PY_CLASS_METHOD_QUALIFIED_1(OctTree, add, void, const OctTree::TChildren&)

}

}

// EOF
