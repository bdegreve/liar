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

#include "shaders_common.h"
#include "fog.h"
#include "../kernel/sample.h"
#include <lass/prim/impl/plane_3d_impl_detail.h>
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(Fog, "")
PY_CLASS_CONSTRUCTOR_0(Fog)
PY_CLASS_CONSTRUCTOR_2(Fog, TScalar, TScalar)
PY_CLASS_MEMBER_RW(Fog, extinction, setExtinction)
PY_CLASS_MEMBER_RW(Fog, assymetry, setAssymetry)
PY_CLASS_MEMBER_RW(Fog, color, setColor)
PY_CLASS_MEMBER_RW(Fog, numScatterSamples, setNumScatterSamples)

// --- public --------------------------------------------------------------------------------------

Fog::Fog()
{
	init();
}



Fog::Fog(TScalar extinction, TScalar assymetry)
{
	init(extinction, assymetry);
}



TScalar Fog::extinction() const
{
	return extinction_;
}



void Fog::setExtinction(TScalar extinction)
{
	extinction_ = std::max<TScalar>(extinction, 0);
}



TScalar Fog::assymetry() const
{
	return assymetry_;
}



void Fog::setAssymetry(TScalar g)
{
	assymetry_ = num::clamp<TScalar>(g, -1, 1);
}



/** diffuse color of particles.
 *  @code
 *  sigma_e = sigma_a + sigma_s // extinction = scattering + absorption.
 *  -> sigma_s = color * sigma_e
 *  -> sigma_a = (1 - color) * sigma_e
 *  @endcode
 */
const XYZ& Fog::color() const
{
	return color_;
}



void Fog::setColor(const XYZ& color)
{
	color_ = color;
}



void Fog::setNumScatterSamples(size_t n)
{
	numSamples_ = n;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

size_t Fog::doNumScatterSamples() const
{
	return numSamples_;
}



const XYZ Fog::doTransmittance(const BoundedRay& ray) const
{
	const TScalar d = ray.farLimit() - ray.nearLimit();
	LASS_ASSERT(d >= 0 && extinction_ >= 0);
	const TScalar thickness = extinction_ * d;
	return XYZ(num::exp(-thickness)); 
}



const XYZ Fog::doScatterOut(const BoundedRay& ray) const
{
	return extinction_ * transmittance(ray);
}



const XYZ Fog::doSampleScatterOut(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const
{
	const TScalar dMax = ray.farLimit() - ray.nearLimit();
	const TScalar attMax = 1 - num::exp(-extinction_ * dMax);
	if (attMax <= 0)
	{
		pdf = 0;
		return XYZ();
	}
	TScalar p;
	const TScalar d = num::uniformToExponential(sample * attMax, extinction_, p);
	tScatter = std::min(ray.nearLimit() + d, ray.farLimit());
	pdf = p / attMax;
	return XYZ(p);
}


const XYZ Fog::doSampleScatterOutOrTransmittance(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const
{
	pdf = 1;
	const TScalar dMax = ray.farLimit() - ray.nearLimit();
	const TScalar d = num::uniformToExponential(sample, extinction_, pdf);
	if (d > dMax)
	{
		// full transmission
		tScatter = ray.farLimit();
		pdf = num::exp(-extinction_ * dMax); // = 1 - cdf(tMax);
		return XYZ(pdf);
	}

	// the photon has hit a particle. we always assume it's scattered.
	// the callee has to russian roulette for absorption himself.
	tScatter = ray.nearLimit() + d;
	return XYZ(pdf);
}


const XYZ Fog::doPhase(const TPoint3D&, const TVector3D& dirIn, const TVector3D& dirOut, TScalar& pdf) const
{
	const TScalar cosTheta = dot(dirIn, dirOut);
	const TScalar g = assymetry_;

	const TScalar g2 = num::sqr(g);
	const TScalar a = 1 - g2;
	const TScalar b = 1 + g2;
	const TScalar c = b - 2 * g * cosTheta;
	TScalar p = a / (4 * TNumTraits::pi * num::pow(c, 1.5));
	if (num::isInf(p))
	{
		p = 1;
	}
	pdf = p;
	return color_ * p;
}



const XYZ Fog::doSamplePhase(const TPoint2D& sample, const TPoint3D&, const TVector3D& dirIn, TVector3D& dirOut, TScalar& pdf) const
{
	const TScalar g = assymetry_;
	const TScalar p = 2 * sample.x - 1;
	
	TScalar cosTheta = g;
	pdf = 1;
	if (g == 0)
	{
		cosTheta = p;
		pdf = 1 / (4 * TNumTraits::pi);
	}
	else if (g * p > -1)
	{
		const TScalar g2 = num::sqr(g);
		const TScalar a = 1 - g2;
		const TScalar b = 1 + g2;
		const TScalar c = b - 2 * g * cosTheta;
		cosTheta = ( b - num::sqr(a / (1 + g * p)) ) / (2 * g);
		pdf = a / (4 * TNumTraits::pi * num::pow(c, 1.5));
	}

	TVector3D u, v;
	generateOrthonormal(dirIn, u, v);
	const TScalar phi = 2 * TNumTraits::pi * sample.y;
	const TScalar sinTheta = num::sqrt(std::max<TScalar>(1 - num::sqr(cosTheta), 0));
	dirOut = cosTheta * dirIn + (sinTheta * num::cos(phi)) * u + (sinTheta * num::sin(phi)) * v;

	return color_ * pdf;
}



void Fog::init(TScalar extinction, TScalar assymetry, const XYZ& color, size_t numSamples)
{
	setExtinction(extinction);
	setAssymetry(assymetry);
	setColor(color);
	setNumScatterSamples(numSamples);
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
