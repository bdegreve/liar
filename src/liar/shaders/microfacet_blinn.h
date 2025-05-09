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

/** @class liar::shaders::MicrofacetBlinn
 *  @brief MicrofacetBlinn Microfacet Distribution
 *
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SHADERS_MICROFACET_BLINN_H
#define LIAR_GUARDIAN_OF_INCLUSION_SHADERS_MICROFACET_BLINN_H

#include "shaders_common.h"
#include "../kernel/microfacet.h"

namespace liar
{
namespace shaders
{

class LIAR_SHADERS_DLL MicrofacetBlinn: public MicrofacetDistribution
{
	PY_HEADER(MicrofacetDistribution)
public:

	MicrofacetBlinn() = default;

	static TMicrofacetDistributionRef instance();

private:

	TValue doD(const TVector3D& omegaHalf, TValue alphaX, TValue alphaY, TValue& pdfH) const override;
	TVector3D doSampleH(const TPoint2D& sample, TValue alphaU, TValue alphaV) const override;
	TValue doLambda(const TVector3D& omega, TValue alphaX, TValue alphaY) const override;
};

}

}

#endif

// EOF
