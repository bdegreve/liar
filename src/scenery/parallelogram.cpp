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

#include "scenery_common.h"
#include "parallelogram.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(Parallelogram)
PY_CLASS_CONSTRUCTOR_3(Parallelogram, const TPoint3D&, const TVector3D&, const TVector3D&)



// --- public --------------------------------------------------------------------------------------

Parallelogram::Parallelogram(const TPoint3D& iSupport, const TVector3D& iSizeU, 
							 const TVector3D& iSizeV):
    SceneObject(&Type),
    parallelogram_(iSupport, iSizeU, iSizeV),
	normal_(cross(iSizeU, iSizeV).normal())
{
	invArea_ = num::inv(parallelogram_.area());
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void Parallelogram::doIntersect(const kernel::Sample& iSample, const kernel::BoundedRay& iRay,
		kernel::Intersection& oResult) const
{
    TScalar t;
	const prim::Result hit = prim::intersect(
		parallelogram_, iRay.unboundedRay(), t, iRay.nearLimit());
	if (hit == prim::rOne && iRay.inRange(t))
	{
		oResult = kernel::Intersection(this, t, seNoEvent);
	}
	else
	{
		oResult = kernel::Intersection::empty();
	}
}



const bool Parallelogram::doIsIntersecting(const kernel::Sample& iSample, 
									const kernel::BoundedRay& iRay) const
{
    TScalar t;
	const prim::Result hit = prim::intersect(
		parallelogram_, iRay.unboundedRay(), t, iRay.nearLimit());
	return hit == prim::rOne && iRay.inRange(t);
}



void Parallelogram::doLocalContext(const kernel::Sample& iSample, const BoundedRay& iRay,
		const kernel::Intersection& iIntersection, 
		kernel::IntersectionContext& oResult) const
{
    TPoint2D uv;
	TScalar t;
	const prim::Result hit = prim::intersect(
		parallelogram_, iRay.unboundedRay(), uv.x, uv.y, t, iRay.nearLimit());
	LASS_ASSERT(hit == prim::rOne && iRay.inRange(t));
	LASS_ASSERT(t == iIntersection.t());

	oResult.setPoint(iRay.point(iIntersection.t()));
    oResult.setT(iIntersection.t());
	oResult.setUv(uv);
    oResult.setDPoint_dU(parallelogram_.sizeU());
    oResult.setDPoint_dV(parallelogram_.sizeV());
    oResult.setNormal(normal_);
    oResult.setDNormal_dU(TVector3D());
    oResult.setDNormal_dV(TVector3D());
}



const bool Parallelogram::doContains(const kernel::Sample& iSample, const TPoint3D& iPoint) const
{
	return false;
}



const bool Parallelogram::doHasSurfaceSampling() const
{
	return true;
}



const TPoint3D Parallelogram::doSampleSurface(const TVector2D& iSample, TVector3D& oNormal,
											  TScalar& oPdf) const
{
	oNormal = normal_;
	oPdf = invArea_;
	return parallelogram_.point(iSample.x, iSample.y);
}



const TAabb3D Parallelogram::doBoundingBox() const
{
	return prim::aabb(parallelogram_);
}



const TScalar Parallelogram::doArea() const
{
	return parallelogram_.area();
}



const TPyObjectPtr Parallelogram::doGetState() const
{
	return python::makeTuple(
		parallelogram_.support(), parallelogram_.sizeU(), parallelogram_.sizeV());
}



void Parallelogram::doSetState(const TPyObjectPtr& iState)
{
	TPoint3D support;
	TVector3D sizeU;
	TVector3D sizeV;
	LASS_ENFORCE(python::decodeTuple(iState, support, sizeU, sizeV));
	parallelogram_ = TParallelogram3D(support, sizeU, sizeV);
	normal_ = cross(sizeU, sizeV).normal();
	invArea_ = num::inv(parallelogram_.area());
}



// --- free ----------------------------------------------------------------------------------------



// --- python --------------------------------------------------------------------------------------

}

}

// EOF
