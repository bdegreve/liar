/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2024  Bram de Greve (bramz@users.sourceforge.net)
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
#include "microfacet_trowbridge_reitz.h"
#include "../kernel/ray_tracer.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(MicrofacetTrowbridgeReitz, "TrowbridgeReitz Microfacet Distribution")
PY_CLASS_CONSTRUCTOR_0(MicrofacetTrowbridgeReitz)


TMicrofacetDistributionPtr MicrofacetTrowbridgeReitz::instance()
{
	static TMicrofacetDistributionPtr instance(new MicrofacetTrowbridgeReitz());
	return instance;
}



MicrofacetTrowbridgeReitz::TValue
MicrofacetTrowbridgeReitz::doD(const TVector3D& h, TValue alphaU, TValue alphaV, TValue& pdfH) const
{
	// B. Burley, Physically-Based Shading at Disney, 2012, Appendix B.2

	constexpr TValue pi = num::NumTraits<TValue>::pi;
	const TValue hx = static_cast<TValue>(h.x);
	const TValue hy = static_cast<TValue>(h.y);
	const TValue hz = static_cast<TValue>(h.z);
	LASS_ASSERT(hz > 0);
	const TValue d = num::inv(pi * alphaU * alphaV * num::sqr(num::sqr(hx / alphaU) + num::sqr(hy / alphaV) + num::sqr(hz)));
	pdfH = d * hz;
	return d;
}



TVector3D
MicrofacetTrowbridgeReitz::doSampleH(const TPoint2D& sample, TValue alphaU, TValue alphaV) const
{
	// B. Burley, Physically-Based Shading at Disney, 2012, Appendix B.2

	LASS_ASSERT(sample.y < TNumTraits::one);
	const TScalar phi = samplePhi(static_cast<TValue>(sample.x), alphaU, alphaV);
	const TScalar s = num::sqrt(sample.y / (TNumTraits::one - sample.y));
	return TVector3D(s * alphaU * num::cos(phi), s * alphaV * num::sin(phi), 1).normal();
}



MicrofacetTrowbridgeReitz::TValue
MicrofacetTrowbridgeReitz::doLambda(const TVector3D& omega, TValue alphaU, TValue alphaV) const
{
	// - "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs", E. Heitz, 2014, p86.
	// - "Microfacet Models for Refraction through Rough Surfaces", Walter et al., 2007

	const TValue invA2 = static_cast<TValue>((num::sqr(omega.x * alphaU) + num::sqr(omega.y * alphaV)) / num::sqr(omega.z));
	return static_cast<TValue>(num::sqrt(1 + invA2) - 1) / 2;
}


}

}
