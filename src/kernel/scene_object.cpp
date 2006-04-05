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
PY_CLASS_METHOD_NAME(SceneObject, reduce, "__reduce__")
PY_CLASS_METHOD_NAME(SceneObject, getState, "__getstate__")
PY_CLASS_METHOD_NAME(SceneObject, setState, "__setstate__")

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



const TPyObjectPtr SceneObject::reduce() const
{
	return python::makeTuple(
		reinterpret_cast<PyObject*>(this->GetType()), python::makeTuple(), this->getState());
}



const TPyObjectPtr SceneObject::getState() const
{
	return python::makeTuple(shader_, interior_, doGetState());
}



void SceneObject::setState(const TPyObjectPtr& iState)
{
	TPyObjectPtr state;
	LASS_ENFORCE(python::decodeTuple(iState, shader_, interior_, state));
	doSetState(state);
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



/** By default, objects don't support surface sampling.
 *  If however, an object can, it must implement doSampleSurface to sample the surface
 *  and override this function to return true
 */
const bool SceneObject::doHasSurfaceSampling() const
{
	return false;
}



/** If an object can sample its surface, it must at least override this function.
 *  Using the (u,v) coordinates in @a iSample, it must generate a point on that surface,
 *  store the normal of the surface in that point in @a oNormal, store its probability
 *  density in @a oPdf and return that point.
 */
const TPoint3D SceneObject::doSampleSurface(const TVector2D& iSample, TVector3D& oNormal,
		TScalar& oPdf) const
{
	LASS_ASSERT(hasSurfaceSampling() == false);
	LASS_THROW("surface sampling is unimplemented for scene objects '" << 
		typeid(*this).name() << "'.");
}



/** Some objects may have a better strategy for sampling a point on its surface when they
 *  know the target (for a shadow ray).  If they have, they can override this function,
 *  if not, the more general one will be called by default.
 */
const TPoint3D SceneObject::doSampleSurface(const TVector2D& iSample, const TPoint3D& iTarget, 
		TVector3D& oNormal, TScalar& oPdf) const
{
	return doSampleSurface(iSample, oNormal, oPdf);
}



/** Some other objects may have an even better strategy for sampling a point on its 
 *  surface when they know the target point and surface normal in that point.
 *  If they have, they can override this function, if not, the more general one will 
 *  be called by default.
 */
const TPoint3D SceneObject::doSampleSurface(const TVector2D& iSample, const TPoint3D& iTarget, 
		const TVector3D& iTargetNormal, TVector3D& oNormal, TScalar& oPdf) const
{
	return doSampleSurface(iSample, iTarget, oNormal, oPdf);
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