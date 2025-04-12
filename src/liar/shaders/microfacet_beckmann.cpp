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
#include "microfacet_beckmann.h"
#include "../kernel/ray_tracer.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(MicrofacetBeckmann, "Beckmann Microfacet Distribution")
PY_CLASS_CONSTRUCTOR_0(MicrofacetBeckmann)

TMicrofacetDistributionRef MicrofacetBeckmann::instance()
{
	static TMicrofacetDistributionRef instance(new MicrofacetBeckmann());
	return instance;
}



// An Overview of BRDF Models, Rosana Montesand Carlos Ureña, Technical Report LSI-2012-001
// Dept. Lenguajes y Sistemas Informáticos University of Granada, Granada, Spain

// BECKMANN P., SPIZZICHINO A.: The Scattering of Electromagnetic Waves from Rough Surfaces.
// Pergamon Press, New York, 1963. Reprinted in 1987 by Artech House Publishers, Norwood, Massachusetts


MicrofacetBeckmann::TValue
MicrofacetBeckmann::doD(const TVector3D& h, TValue alphaU, TValue alphaV, TValue& pdfH) const
{
	// original isotropic Beckmann distribution
	// D = 1 / (m^2 cos^4 theta) * exp -{ (tan^2 theta) / m^2 }
	//
	// anisotropic roughness: 1/m^2 -> cos^2 phi / m_x^2 + sin^2 phi / m_y^2
	// and cos^2 phi = h_x^2 / sin^2 theta

	using TValueTraits = num::NumTraits<TValue>;

	const TValue hx = static_cast<TValue>(h.x);
	const TValue hy = static_cast<TValue>(h.y);
	const TValue hz = static_cast<TValue>(h.z);
	const TValue cosTheta2 = num::sqr(hz);
	if (cosTheta2 == TValueTraits::zero)
		return TValueTraits::zero;
	const TValue d = num::exp(-(num::sqr(hx / alphaU) + num::sqr(hy / alphaV)) / cosTheta2)
		/ (TValueTraits::pi * alphaU * alphaV * num::sqr(cosTheta2));
	pdfH = d * hz;
	return d;
}



TVector3D MicrofacetBeckmann::doSampleH(const TPoint2D& sample, TValue alphaU, TValue alphaV) const
{
	LASS_ASSERT(sample.x < TNumTraits::one);
	const TScalar s = num::log(TNumTraits::one - sample.x);
	LASS_ASSERT(!num::isInf(s));

	TScalar cosPhi;
	TScalar sinPhi;
	TScalar tanTheta2;
	if (alphaU == alphaV)
	{
		const TScalar phi = 2 * TNumTraits::pi * sample.y;
		cosPhi = num::cos(phi);
		sinPhi = num::sin(phi);
		tanTheta2 = -s * num::sqr(alphaU);
	}
	else
	{
		const TScalar phi = samplePhi(static_cast<TValue>(sample.x), alphaU, alphaV);
		cosPhi = num::cos(phi);
		sinPhi = num::sin(phi);
		tanTheta2 = -s / (num::sqr(cosPhi / alphaU) + num::sqr(sinPhi / alphaV));
	}

	const TScalar cosTheta2 = num::inv(1 + tanTheta2);
	const TScalar cosTheta = num::sqrt(cosTheta2);
	const TScalar sinTheta = num::sqrt(std::max(1 - cosTheta2, TNumTraits::zero));

	return TVector3D(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta);
}



MicrofacetBeckmann::TValue
MicrofacetBeckmann::doLambda(const TVector3D& omega, TValue alphaU, TValue alphaV) const
{
	// approximation from "Microfacet Models for Refraction through Rough Surfaces", Walter et al., 2007
	// anistropic verion from "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs", Heitz, 2014, p86.

	// a = 1 / (alpha * tanTheta)
	const TValue a2 = static_cast<TValue>(num::sqr(omega.z) / (num::sqr(omega.x * alphaU) + num::sqr(omega.y * alphaV)));
	LIAR_ASSERT_POSITIVE_FINITE(a2);
	const TValue a = num::sqrt(a2);

	return a < 1.6f
		? (3.535f * a + 2.181f * a2) / (1 + 2.276f * a + 2.577f * a2)
		: 1;
}


}

}
