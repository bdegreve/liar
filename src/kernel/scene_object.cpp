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
#include "scene_object.h"
#include "intersection.h"
#include "intersection_context.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS(SceneObject)
PY_CLASS_MEMBER_RW(SceneObject, "shader", shader, setShader)
PY_CLASS_MEMBER_RW(SceneObject, "interior", interior, setInterior)

TShaderPtr SceneObject::defaultShader_(0);

// --- public --------------------------------------------------------------------------------------

SceneObject::~SceneObject()
{
}



const TShaderPtr& SceneObject::shader() const
{
    return shader_;
}



void SceneObject::setShader(const TShaderPtr& iShader)
{
    shader_ = iShader;
}



const TMediumPtr& SceneObject::interior() const
{
	return interior_;
}



void SceneObject::setInterior(const TMediumPtr& iMedium)
{
	interior_ = iMedium;
}



const TShaderPtr& SceneObject::defaultShader()
{
    return defaultShader_;
}



void SceneObject::setDefaultShader(const TShaderPtr& iDefaultShader)
{
    defaultShader_ = iDefaultShader;
}


// --- protected -----------------------------------------------------------------------------------

SceneObject::SceneObject(PyTypeObject* iType):
    python::PyObjectPlus(iType),
    shader_(defaultShader_)
{
}



// --- private -------------------------------------------------------------------------------------

void SceneObject::doPreProcess(const TimePeriod& iPeriod)
{
	// not all objects need this
}



const TPoint3D SceneObject::doSampleSurface(const TVector2D& iSample, TVector3D& oNormal) const
{
	LASS_THROW("surface sampling is unimplemented for scene objects '" << 
		typeid(*this).name() << "'.");
}



const TPoint3D SceneObject::doSampleSurface(const TVector2D& iSample, const TPoint3D& iTarget, 
		TVector3D& oNormal) const
{
	return doSampleSurface(iSample, oNormal);
}



void SceneObject::doLocalSpace(TTime iTime, TTransformation3D& ioLocalToWorld) const
{
	// most objects don't have a local space matrix.
}



/** return true if object can change with time.
 *  By default, objects don't.
 */
const bool SceneObject::doHasMotion() const
{
	return false;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF