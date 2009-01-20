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
#include "scene_object.h"
#include "intersection.h"
#include "intersection_context.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS(SceneObject)
PY_CLASS_MEMBER_RW(SceneObject, shader, setShader)
PY_CLASS_MEMBER_RW(SceneObject, isOverridingShader, setOverridingShader)
PY_CLASS_MEMBER_RW(SceneObject, interior, setInterior)
PY_CLASS_MEMBER_RW(SceneObject, isOverridingInterior, setOverridingInterior)
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



void SceneObject::setShader(const TShaderPtr& shader)
{
    shader_ = shader;
}



const bool SceneObject::isOverridingShader() const
{
    return isOverridingShader_;
}



void SceneObject::setOverridingShader(bool enabled)
{
    isOverridingShader_ = enabled;
}



const TMediumPtr& SceneObject::interior() const
{
	return interior_;
}



void SceneObject::setInterior(const TMediumPtr& medium)
{
	interior_ = medium;
}



const bool SceneObject::isOverridingInterior() const
{
    return isOverridingInterior_;
}



void SceneObject::setOverridingInterior(bool enabled)
{
    isOverridingInterior_ = enabled;
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
		python::fromNakedToSharedPtrCast<PyObject>(reinterpret_cast<PyObject*>(this->_lassPyGetType())), 
		python::makeTuple(), this->getState());
}



const TPyObjectPtr SceneObject::getState() const
{
	return python::makeTuple(shader_, interior_, doGetState());
}



void SceneObject::setState(const TPyObjectPtr& state)
{
	TPyObjectPtr derivedState;
	LASS_ENFORCE(python::decodeTuple(state, shader_, interior_, derivedState));
	doSetState(derivedState);
}



// --- protected -----------------------------------------------------------------------------------

SceneObject::SceneObject():
    shader_(defaultShader_),
	isOverridingShader_(false),
	isOverridingInterior_(false)
{
}



// --- private -------------------------------------------------------------------------------------

void SceneObject::doPreProcess(const TSceneObjectPtr& scene, const TimePeriod& period)
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



void SceneObject::doFun(const TRay3D& ray, BoundedRay& shadowRay, TScalar& pdf) const
{
	LASS_ASSERT(hasSurfaceSampling() == false);
	LASS_THROW("surface sampling is unimplemented for scene objects '" << 
		typeid(*this).name() << "'.");
}



/** If an object can sample its surface, it must at least override this function.
 *  Using the (u,v) coordinates in @a sample, it must generate a point on that surface,
 *  store the normal of the surface in that point in @a normal, store its probability
 *  density in @a pdf and return that point.
 */
const TPoint3D SceneObject::doSampleSurface(const TPoint2D& sample, TVector3D& normal,
		TScalar& pdf) const
{
	LASS_ASSERT(hasSurfaceSampling() == false);
	LASS_THROW("surface sampling is unimplemented for scene objects '" << 
		typeid(*this).name() << "'.");
}



/** Some objects may have a better strategy for sampling a point on its surface when they
 *  know the target (for a shadow ray).  If they have, they can override this function,
 *  if not, the more general one will be called by default.
 *
 */
const TPoint3D SceneObject::doSampleSurface(const TPoint2D& sample, const TPoint3D& target, 
		TVector3D& normal, TScalar& pdf) const
{
	const TPoint3D result = doSampleSurface(sample, normal, pdf);
	
	TVector3D toLight = result - target;
	const TScalar squaredDistance = toLight.squaredNorm();
	toLight /= num::sqrt(squaredDistance);

	const TScalar cosTheta = dot(normal, toLight);

	pdf *= squaredDistance / num::abs(cosTheta);
	return result;
}



/** Some other objects may have an even better strategy for sampling a point on its 
 *  surface when they know the target point and surface normal in that point.
 *  If they have, they can override this function, if not, the more general one will 
 *  be called by default.
 */
const TPoint3D SceneObject::doSampleSurface(const TPoint2D& sample, const TPoint3D& target, 
		const TVector3D& targetNormal, TVector3D& normal, TScalar& pdf) const
{
	return doSampleSurface(sample, target, normal, pdf);
}



void SceneObject::doLocalSpace(TTime time, TTransformation3D& localToWorld) const
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
