/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::BoundedRay
 *	@brief a 3D ray associated with a near and far clipping limit
 *  @author Bram de Greve [Bramz]
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
	BoundedRay(const TRay3D& unboundedRay,
		TScalar nearLimit = liar::TNumTraits::zero, TScalar farLimit = liar::TNumTraits::infinity);
	BoundedRay(const TPoint3D& support, const TVector3D& direction,
		TScalar nearLimit = liar::TNumTraits::zero, TScalar farLimit = liar::TNumTraits::infinity);
	BoundedRay(const TPoint3D& support, const TVector3D& normalizedDirection,
		TScalar nearLimit, TScalar farLimit, prim::IsAlreadyNormalized);
	BoundedRay(const TPoint3D& support, const TPoint3D& iLookAt,
		TScalar nearLimit = liar::TNumTraits::zero, TScalar farLimit = liar::TNumTraits::infinity);

	BoundedRay operator-() const;

	const TRay3D& unboundedRay() const { return unboundedRay_; }
	TScalar nearLimit() const { return nearLimit_; }
	TScalar farLimit() const { return farLimit_; }

	const TPoint3D& support() const { return unboundedRay_.support(); }
	TPoint3D& support() { return unboundedRay_.support(); }
	const TVector3D& direction() const { return unboundedRay_.direction(); }

	const TPoint3D point(TScalar t) const { return unboundedRay_.point(t); }
	const TPoint3D nearPoint() const { return point(nearLimit_); }
	const TPoint3D farPoint() const { return point(nearLimit_); }

	/** return true if @a t is between the scalar bounds of the ray */
	bool inRange(TScalar t) const
	{
		//return num::almostInOpenRange(t, nearLimit_, farLimit_, tolerance);
		return t > nearLimit_ * (TNumTraits::one - tolerance) &&
			t < farLimit_ * (TNumTraits::one + tolerance);
	}

	bool isEmpty() const { return nearLimit_ > farLimit_; }
	bool operator!() const { return isEmpty(); }

private:

	TRay3D unboundedRay_;
	TScalar nearLimit_;
	TScalar farLimit_;
};

LIAR_KERNEL_DLL BoundedRay transform(const BoundedRay& ray, const TTransformation3D& transformation);
LIAR_KERNEL_DLL BoundedRay transform(const BoundedRay& ray, const TTransformation3D& transformation, TScalar& parameter);
LIAR_KERNEL_DLL BoundedRay translate(const BoundedRay& ray, const TVector3D& offset);
LIAR_KERNEL_DLL BoundedRay bound(const BoundedRay& ray, TScalar nearLimit, TScalar farLimit = TNumTraits::infinity);
LIAR_KERNEL_DLL BoundedRay bound(const BoundedRay& ray, const TAabb3D& box);

}

}

#endif

// EOF
