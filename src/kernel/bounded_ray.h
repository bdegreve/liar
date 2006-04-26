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

/** @class liar::BoundedRay
 *	@brief a 3D ray associated with a near and far clipping limit
 *  @author Bram de Greve [BdG]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_BOUNDED_RAY_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_BOUNDED_RAY_H

#include "kernel_common.h"

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL BoundedRay
{
public:

	BoundedRay();
    BoundedRay(const TRay3D& iUnboundedRay, 
		const TScalar iNearLimit = liar::TNumTraits::zero, 
		const TScalar iFarLimit = liar::TNumTraits::infinity);
	BoundedRay(const TPoint3D& iSupport, const TVector3D& iDirection, 
		const TScalar iNearLimit = liar::TNumTraits::zero, 
		const TScalar iFarLimit = liar::TNumTraits::infinity);
	BoundedRay(const TPoint3D& iSupport, const TVector3D& iDirection, 
		const TScalar iNearLimit, const TScalar iFarLimit,
		prim::IsAlreadyNormalized);
	BoundedRay(const TPoint3D& iSupport, const TPoint3D& iLookAt, 
		const TScalar iNearLimit = liar::TNumTraits::zero, 
		const TScalar iFarLimit = liar::TNumTraits::infinity);

	const TRay3D& unboundedRay() const { return unboundedRay_; }
	const TScalar nearLimit() const { return nearLimit_; }
	const TScalar farLimit() const { return farLimit_; }

	const TPoint3D& support() const { return unboundedRay_.support(); }
	TPoint3D& support() { return unboundedRay_.support(); }
	const TVector3D& direction() const { return unboundedRay_.direction(); }

	const TPoint3D point(TScalar iT) const { return unboundedRay_.point(iT); }
	const bool inRange(TScalar iT) const 
	{ 
		//return num::almostInOpenRange(iT, nearLimit_, farLimit_, tolerance); 
		return iT > nearLimit_ * (TNumTraits::one + tolerance) &&
			iT < farLimit_ * (TNumTraits::one - tolerance);
	}

private:

    TRay3D unboundedRay_;
    TScalar nearLimit_;
	TScalar farLimit_;
};

LIAR_KERNEL_DLL BoundedRay transform(const BoundedRay& iRay, const TTransformation3D& iTransformation);
LIAR_KERNEL_DLL BoundedRay transform(const BoundedRay& iRay, const TTransformation3D& iTransformation, TScalar& ioParameter);
LIAR_KERNEL_DLL BoundedRay translate(const BoundedRay& iRay, const TVector3D& iOffset);

}

}

#endif

// EOF
