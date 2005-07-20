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
#include "bounded_ray.h"

namespace liar
{
namespace kernel
{

// --- public --------------------------------------------------------------------------------------

BoundedRay::BoundedRay():
    unboundedRay_(),
	nearLimit_(TNumTraits::zero),
	farLimit_(TNumTraits::zero)
{
}



BoundedRay::BoundedRay(const TRay3D& iUnboundedRay, TScalar iNearLimit, TScalar iFarLimit):
    unboundedRay_(iUnboundedRay),
    nearLimit_(iNearLimit),
    farLimit_(iFarLimit)
{
}



BoundedRay::BoundedRay(const TPoint3D& iSupport, const TVector3D& iDirection, 
					   TScalar iNearLimit, TScalar iFarLimit):
    unboundedRay_(iSupport, iDirection),
    nearLimit_(iNearLimit),
    farLimit_(iFarLimit)
{
}



BoundedRay::BoundedRay(const TPoint3D& iSupport, const TPoint3D& iLookAt, 
					   TScalar iNearLimit, TScalar iFarLimit):
    unboundedRay_(iSupport, iLookAt),
    nearLimit_(iNearLimit),
    farLimit_(iFarLimit)
{
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------



// --- free ----------------------------------------------------------------------------------------

/** @relates BoundedRay
 */
BoundedRay transform(const BoundedRay& iRay, const TTransformation3D& iTransformation)
{
	TScalar tScale = TNumTraits::one;
	TRay3D transformedRay = prim::transform(iRay.unboundedRay(), iTransformation, tScale);
	return BoundedRay(transformedRay, iRay.nearLimit() * tScale, iRay.farLimit() * tScale);
}



/** @relates BoundedRay
 */
BoundedRay transform(const BoundedRay& iRay, const TTransformation3D& iTransformation,
					 TScalar& ioParameter)
{
	TScalar tScale = TNumTraits::one;
	TRay3D transformedRay = prim::transform(iRay.unboundedRay(), iTransformation, tScale);
	ioParameter *= tScale;
	return BoundedRay(transformedRay, iRay.nearLimit() * tScale, iRay.farLimit() * tScale);
}



/** @relates BoundedRay
 */
BoundedRay translate(const BoundedRay& iRay, const TVector3D& iOffset)
{
	BoundedRay result(iRay);
	result.support() += iOffset;
	return result;
}


}

}

// EOF