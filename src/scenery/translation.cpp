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
#include "translation.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(Translation)
PY_CLASS_CONSTRUCTOR_2(Translation, const TSceneObjectPtr&, const TVector3D&)
PY_CLASS_MEMBER_RW(Translation, child, setChild)
PY_CLASS_MEMBER_RW(Translation, localToWorld, setLocalToWorld)
PY_CLASS_MEMBER_R(Translation, worldToLocal)


// --- public --------------------------------------------------------------------------------------

Translation::Translation(const TSceneObjectPtr& child, 
							   const TVector3D& localToWorld):
    child_(child),
    localToWorld_(localToWorld)
{
}



const TSceneObjectPtr& Translation::child() const
{
	return child_;
}



void Translation::setChild(const TSceneObjectPtr& child)
{
	child_ = child;
}



const TVector3D& Translation::localToWorld() const
{
	return localToWorld_;
}



void Translation::setLocalToWorld(const TVector3D& localToWorld)
{
	localToWorld_ = localToWorld;
}



const TVector3D Translation::worldToLocal() const
{
	return -localToWorld_;
}



// --- protected -----------------------------------------------------------------------------------

// --- private -------------------------------------------------------------------------------------

void Translation::doPreProcess(const TSceneObjectPtr& scene, const TimePeriod& period)
{
	child_->preProcess(scene, period);
}



void Translation::doAccept(util::VisitorBase& visitor)
{
    doVisit(*this, visitor);
    child_->accept(visitor);
	doVisitOnExit(*this, visitor);
}



void Translation::doIntersect(const Sample& sample, const BoundedRay& ray, 
							  Intersection& result) const
{
	const BoundedRay localRay = translate(ray, -localToWorld_);
	child_->intersect(sample, localRay, result);
    if (result)
    {
        result.push(this);
    }
}



const bool Translation::doIsIntersecting(const Sample& sample, 
										 const BoundedRay& ray) const
{
	const BoundedRay localRay = translate(ray, -localToWorld_);
	return child_->isIntersecting(sample, localRay);
}



void Translation::doLocalContext(const Sample& sample, const BoundedRay& ray,
								 const Intersection& intersection, 
								 IntersectionContext& result) const
{
	IntersectionDescendor descendor(intersection);
	LASS_ASSERT(intersection.object() == child_.get());

	const TRay3D localRay(ray.support() - localToWorld_, ray.direction());
	child_->localContext(sample, localRay, intersection, result);
	result.translateBy(localToWorld_);
}



void Translation::doLocalSpace(TTime time, TTransformation3D& localToWorld) const 
{
	localToWorld = concatenate(TTransformation3D::translation(localToWorld_), localToWorld);
}



const bool Translation::doContains(const Sample& sample, const TPoint3D& point) const
{
	const TPoint3D localPoint = point - localToWorld_;
	return child_->contains(sample, localPoint);
}



const TAabb3D Translation::doBoundingBox() const
{
	const TAabb3D localBox = child_->boundingBox();
	return TAabb3D(localBox.min() + localToWorld_, localBox.max() + localToWorld_);
}



const TScalar Translation::doArea() const
{
	return child_->area();
}



const TPyObjectPtr Translation::doGetState() const
{
	return python::makeTuple(child_, localToWorld_);
}



void Translation::doSetState(const TPyObjectPtr& state)
{
	LASS_ENFORCE(python::decodeTuple(state, child_, localToWorld_));
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
