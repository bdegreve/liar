/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2025  Bram de Greve (bramz@users.sourceforge.net)
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

#include "scenery_common.h"
#include "transformation.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_DOC(Transformation, "transformation of local space")
PY_CLASS_CONSTRUCTOR_2(Transformation, const TSceneObjectRef&, const TTransformation3D&)
PY_CLASS_CONSTRUCTOR_2(Transformation, const TSceneObjectRef&, const TPyTransformation3DRef&)
PY_CLASS_MEMBER_RW(Transformation, child, setChild)
PY_CLASS_MEMBER_RW(Transformation, localToWorld, setLocalToWorld)
PY_CLASS_MEMBER_R(Transformation, worldToLocal)


// --- public --------------------------------------------------------------------------------------

Transformation::Transformation(const TSceneObjectRef& child, const TTransformation3D& localToWorld):
	child_(child),
	localToWorld_(localToWorld),
	worldToLocal_(localToWorld.inverseMatrix())
{
}



Transformation::Transformation(const TSceneObjectRef& child, const TPyTransformation3DRef& localToWorld):
	child_(child),
	localToWorld_(localToWorld->transformation()),
	worldToLocal_(localToWorld_.inverseMatrix())
{
}



const TSceneObjectRef& Transformation::child() const
{
	return child_;
}



void Transformation::setChild(const TSceneObjectRef& child)
{
	child_ = child;
}



const TTransformation3D& Transformation::localToWorld() const
{
	return localToWorld_;
}



void Transformation::setLocalToWorld(const TTransformation3D& localToWorld)
{
	localToWorld_ = localToWorld;
	worldToLocal_ = localToWorld.inverseMatrix();
}



const TTransformation3D Transformation::worldToLocal() const
{
	return localToWorld_.inverse();
}



// --- protected -----------------------------------------------------------------------------------

// --- private -------------------------------------------------------------------------------------

void Transformation::doPreProcess(const TimePeriod& period)
{
	child_->preProcess(period);
}



void Transformation::doAccept(util::VisitorBase& visitor)
{
	preAccept(visitor, *this);
	child_->accept(visitor);
	postAccept(visitor, *this);
}



void Transformation::doIntersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const
{
	TScalar tScaler = TNumTraits::one;
	const BoundedRay localRay = transform(ray, worldToLocal_, tScaler);
	child_->intersect(sample, localRay, result);
	if (result)
	{
		result.push(this, result.t() / tScaler); // direction vectors are always normalized.
	}
}



bool Transformation::doIsIntersecting(const Sample& sample, const BoundedRay& ray) const
{
	const BoundedRay localRay = transform(ray, worldToLocal_);
	return child_->isIntersecting(sample, localRay);
}



void Transformation::doLocalContext(const Sample& sample, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const
{
	IntersectionDescendor descendor(intersection);
	LASS_ASSERT(intersection.object() == child_.get());

	const BoundedRay localRay = transform(ray, worldToLocal_);
	child_->localContext(sample, localRay, intersection, result);

	result.transformBy(localToWorld_);
	result.setT(intersection.t());
}



void Transformation::doLocalSpace(TTime, TTransformation3D& localToWorld) const
{
	localToWorld = concatenate(localToWorld_, localToWorld);
}



bool Transformation::doContains(const Sample& sample, const TPoint3D& point) const
{
	const TPoint3D localPoint = worldToLocal_ * point;
	return child_->contains(sample, localPoint);
}



const TAabb3D Transformation::doBoundingBox() const
{
	return transform(child_->boundingBox(), localToWorld_);
}



TScalar Transformation::doArea() const
{
	return child_->area(); // FIXME: scale by determinant of localToWorld_?
}



TScalar Transformation::doArea(const TVector3D& normal) const
{
	return child_->area(normalTransform(normal, worldToLocal())); // FIXME: scale by determinant of localToWorld_?
}



const TPyObjectPtr Transformation::doGetState() const
{
	return python::makeTuple(child_, localToWorld_);
}



void Transformation::doSetState(const TPyObjectPtr& state)
{
	LASS_ENFORCE(python::decodeTuple(state, child_, localToWorld_));
	worldToLocal_ = localToWorld_.inverseMatrix();
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
