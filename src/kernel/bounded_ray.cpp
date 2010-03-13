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
#include "bounded_ray.h"

namespace liar
{
namespace kernel
{

// --- public --------------------------------------------------------------------------------------

/** construct an empty bounded ray 
 */
BoundedRay::BoundedRay():
	unboundedRay_(),
	nearLimit_(TNumTraits::zero),
	farLimit_(TNumTraits::zero)
{
}



/** construct from an unbounded ray and two scalar bounds.
 */
BoundedRay::BoundedRay(const TRay3D& unboundedRay, TScalar nearLimit, TScalar farLimit):
	unboundedRay_(unboundedRay),
	nearLimit_(nearLimit),
	farLimit_(farLimit)
{
	LASS_ASSERT(nearLimit_ >= 0 && nearLimit_ <= farLimit_);
}



/** construct from support point, @e unnormalized direction and two scalar bounds.
 *  The direction is automatically normalized
 */
BoundedRay::BoundedRay(const TPoint3D& support, const TVector3D& direction, TScalar nearLimit, TScalar farLimit):
	unboundedRay_(support, direction),
	nearLimit_(nearLimit),
	farLimit_(farLimit)
{
	LASS_ASSERT(nearLimit_ >= 0 && nearLimit_ < farLimit_);
}



/** construct from support point, @e normalized direction and two scalar bounds.
 *  If you use this constructor, you commit yourself to provide it with a @e normalized
 *	direction vector.  This way, the constructor can skip a costly normalization step.
 *
 *	@code
 *	BoundedRay ray(support, normalizedDirection, nearLimit, farLimit, prim::IsAlreadyNormalized());
 *	@endcode
 */
BoundedRay::BoundedRay(const TPoint3D& support, const TVector3D& normalizedDirection, TScalar nearLimit, TScalar farLimit, prim::IsAlreadyNormalized):
	unboundedRay_(support, normalizedDirection, prim::IsAlreadyNormalized()),
	nearLimit_(nearLimit),
	farLimit_(farLimit)
{
	LASS_ASSERT(nearLimit_ >= 0 && nearLimit_ < farLimit_);
}



/** construct from a support point, a look-at point and two scalar bounds.
 *	The direction of the ray will be from @a support @a lookAt.
 */
BoundedRay::BoundedRay(const TPoint3D& support, const TPoint3D& lookAt, TScalar nearLimit, TScalar farLimit):
	unboundedRay_(support, lookAt),
	nearLimit_(nearLimit),
	farLimit_(farLimit)
{
	LASS_ASSERT(nearLimit_ >= 0 && nearLimit_ < farLimit_);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------



// --- free ----------------------------------------------------------------------------------------

/** @relates BoundedRay
 *  Transform a bounded ray, renormalize and adjust its bounds.
 *
 *	The scalar bounds must be adjusted because of the renormalization, so that the 
 *	following equation is valid for both limits (equations shows us the nearLimit only):
 *
 *	@code
 *	BoundedRay newRay = transform(ray, transformation);
 *	LASS_ASSERT(newRay.point(newRay.nearLimit()) == 
 *		prim::transform(ray.point(ray.nearLimit()), transformation));
 *	@endcode.
 */
BoundedRay transform(const BoundedRay& ray, const TTransformation3D& transformation)
{
	TScalar tScale = TNumTraits::one;
	TRay3D transformedRay = prim::transform(ray.unboundedRay(), transformation, tScale);
	return BoundedRay(transformedRay, ray.nearLimit() * tScale, ray.farLimit() * tScale);
}



/** @relates BoundedRay
 *	Transform a bounded ray, renormalize and adjust its bounds and @a parameter.
 *
 *	The scalar bounds and @a parameter must be adjusted because of the renormalization, so
 *	that @a parameter refers to the correct point on the transformed ray:
 *
 *	@code
 *	TScalar newParameter = parameter;
 *	BoundedRay newRay = transform(ray, transformation, newParameter);
 *	LASS_ASSERT(newRay.point(newParameter) == 
 *		prim::transform(ray.point(parameter), transformation));
 *	@endcode.

 */
BoundedRay transform(const BoundedRay& ray, const TTransformation3D& transformation,
					 TScalar& parameter)
{
	TScalar tScale = TNumTraits::one;
	TRay3D transformedRay = prim::transform(ray.unboundedRay(), transformation, tScale);
	parameter *= tScale;
	return BoundedRay(transformedRay, ray.nearLimit() * tScale, ray.farLimit() * tScale);
}



/** @relates BoundedRay
 *  add an offset to the ray.
 */
BoundedRay translate(const BoundedRay& ray, const TVector3D& offset)
{
	BoundedRay result(ray);
	result.support() += offset;
	return result;
}



/** @relates BoundedRay
 */
BoundedRay bound(const BoundedRay& ray, TScalar nearLimit, TScalar farLimit)
{
	return BoundedRay(ray.unboundedRay(), std::max(ray.nearLimit(), nearLimit), std::min(ray.farLimit(), farLimit));
}

}

}

// EOF
