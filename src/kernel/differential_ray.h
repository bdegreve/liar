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

/** @class liar::DifferentialRay
 *  @brief a bounded ray plus two rays for screen space differentials
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_DIFFERENTIAL_RAY_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_DIFFERENTIAL_RAY_H

#include "kernel_common.h"
#include "bounded_ray.h"
#include <lass/prim/point_2d.h>

namespace liar
{
namespace kernel
{

class IntersectionContext;

class LIAR_KERNEL_DLL DifferentialRay
{
public:

	DifferentialRay();
	DifferentialRay(const BoundedRay& centralRay, const TRay3D& differentialI, const TRay3D& differentialJ);
	explicit DifferentialRay(const BoundedRay& centralRay);

	const BoundedRay& centralRay() const { return centralRay_; }
	const TRay3D& differentialI() const { return differentialI_; }
	const TRay3D& differentialJ() const { return differentialJ_; }
	bool hasDifferentials() const { return hasDifferentials_; }

	const TPoint3D& support() const { return centralRay_.support(); }		/**< return support point of central ray */
	const TVector3D& direction() const { return centralRay_.direction(); }	/**< return diretion of central ray */
	const TPoint3D point(const TScalar t) const { return centralRay_.point(t); }
	TScalar nearLimit() const { return centralRay_.nearLimit(); }
	TScalar farLimit() const { return centralRay_.farLimit(); }

	bool isValid() const { return centralRay_.unboundedRay().isValid(); }

	friend LIAR_KERNEL_DLL DifferentialRay reflect(const IntersectionContext& context, const DifferentialRay& ray);
	friend LIAR_KERNEL_DLL DifferentialRay refract(const IntersectionContext& context, const DifferentialRay& ray,
		TScalar refractionIndex1over2);

private:

	BoundedRay centralRay_;
	TRay3D differentialI_;
	TRay3D differentialJ_;
	bool hasDifferentials_;
};

LIAR_KERNEL_DLL DifferentialRay transform(const DifferentialRay& ray, const TTransformation3D& transformation);
LIAR_KERNEL_DLL DifferentialRay reflect(const IntersectionContext& context, const DifferentialRay& ray);
LIAR_KERNEL_DLL DifferentialRay refract(const IntersectionContext& context, const DifferentialRay& ray, TScalar refractionIndex1over2);
LIAR_KERNEL_DLL DifferentialRay bound(const DifferentialRay& ray, TScalar nearLimit, TScalar farLimit = TNumTraits::infinity);


}

}

#endif

// EOF
