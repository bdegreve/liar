/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2023-2025  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::MicrofacetDistribution
 *  @brief basec class of microfacet distributions
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_MICROFACET_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_MICROFACET_H

#include "kernel_common.h"

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL MicrofacetDistribution: public python::PyObjectPlus
{
	PY_HEADER(python::PyObjectPlus)
public:

	using TValue = float;

	virtual ~MicrofacetDistribution() = default;

	TValue D(const TVector3D& omegaHalf, TValue alphaX, TValue alphaY, TValue& pdfH) const;
	TVector3D sampleH(const TPoint2D& sample, TValue alphaU, TValue alphaV) const;
	TValue G1(const TVector3D& omega, const TVector3D& omegaHalf, TValue alphaX, TValue alphaY) const;
	TValue lambda(const TVector3D& omega, TValue alphaX, TValue alphaY) const;


	const TPyObjectPtr reduce() const;
	const TPyObjectPtr getState() const;
	void setState(const TPyObjectPtr& state);

protected:

	MicrofacetDistribution() = default;

	TValue samplePhi(TValue sample, TValue alphaU, TValue alphaV) const;

private:

	virtual TValue doD(const TVector3D& omegaHalf, TValue alphaX, TValue alphaY, TValue& pdfH) const = 0;
	virtual TVector3D doSampleH(const TPoint2D& sample, TValue alphaU, TValue alphaV) const = 0;
	virtual TValue doLambda(const TVector3D& omega, TValue alphaX, TValue alphaY) const = 0;

	virtual const TPyObjectPtr doGetState() const;
	virtual void doSetState(const TPyObjectPtr& state);
};

typedef PyObjectRef<MicrofacetDistribution> TMicrofacetDistributionRef;

}

}

#endif

// EOF
