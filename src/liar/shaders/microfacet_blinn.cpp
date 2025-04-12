/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2024-2025  Bram de Greve (bramz@users.sourceforge.net)
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

#include "shaders_common.h"
#include "microfacet_blinn.h"
#include "../kernel/ray_tracer.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(MicrofacetBlinn, "Blinn Microfacet Distribution")
PY_CLASS_CONSTRUCTOR_0(MicrofacetBlinn)


TMicrofacetDistributionRef MicrofacetBlinn::instance()
{
	static TMicrofacetDistributionRef instance(new MicrofacetBlinn());
	return instance;
}



MicrofacetBlinn::TValue
MicrofacetBlinn::doD(const TVector3D& h, TValue alphaU, TValue alphaV, TValue& pdfH) const
{
	using TValueTraits = num::NumTraits<TValue>;

	const TValue hx = static_cast<TValue>(h.x);
	const TValue hy = static_cast<TValue>(h.y);
	const TValue hz = static_cast<TValue>(h.z);
	alphaU = std::max(alphaU, TValue(1e-4f));
	alphaV = std::max(alphaV, TValue(1e-4f));
	const TValue sinTheta2 = std::max(1 - num::sqr(hz), TValue(0));
	const TValue invAlpha2 = sinTheta2 > 0
		? (num::sqr(hx / alphaU) + num::sqr(hy / alphaV)) / sinTheta2
		: .5f / num::sqr(alphaU) + .5f / num::sqr(alphaV);
	const TValue nn = 2 * invAlpha2 - 2;
	const TValue d = num::pow(static_cast<TValue>(h.z), nn) / (TValueTraits::pi * alphaU * alphaV);
	pdfH = d;
	return d;
}



TVector3D
MicrofacetBlinn::doSampleH(const TPoint2D& sample, TValue alphaU, TValue alphaV) const
{
	using TValueTraits = num::NumTraits<TValue>;

	const TValue phi = alphaU == alphaV
		? 2 * TValueTraits::pi * static_cast<TValue>(sample.x)
		: samplePhi(static_cast<TValue>(sample.x), alphaU, alphaV);
	const TValue cosPhi = num::cos(phi);
	const TValue sinPhi = num::sin(phi);

	const TValue A = num::sqr(cosPhi / alphaU) + num::sqr(sinPhi / alphaV);
	const TValue cosTheta = num::pow(1 - static_cast<TValue>(sample.y), num::inv(2 * A));
	const TValue sinTheta = num::sqrt(std::max(TValue(0), 1 - num::sqr(cosTheta)));

	return TVector3D(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta);
}



MicrofacetBlinn::TValue
MicrofacetBlinn::doLambda(const TVector3D& omega, TValue alphaU, TValue alphaV) const
{
	// the same as Beckmann, as suggested by "Microfacet Models for Refraction through Rough Surfaces", Walter et al., 2007
	// anistropic verion from "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs", Heitz, 2014, p86.

	// a = 1 / (alpha * tanTheta)
	const TValue a2 = static_cast<TValue>(num::sqr(omega.z) / (num::sqr(omega.x * alphaU) + num::sqr(omega.y * alphaV)));
	LIAR_ASSERT(a2 >= 0, "a2=" << a2);
	const TValue a = num::sqrt(a2);

	return a < 1.6f
		? (3.535f * a + 2.181f * a2) / (1 + 2.276f * a + 2.577f * a2)
		: 1;
}

}

}
