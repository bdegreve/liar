/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2021  Bram de Greve (bramz@users.sourceforge.net)
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

#include "mediums_common.h"
#include "fog.h"
#include "../kernel/sample.h"
#include <lass/prim/impl/plane_3d_impl_detail.h>
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace mediums
{

PY_DECLARE_CLASS_DOC(Fog, "")
PY_CLASS_CONSTRUCTOR_0(Fog)
PY_CLASS_CONSTRUCTOR_2(Fog, Fog::TValue, Fog::TValue)
PY_CLASS_MEMBER_RW(Fog, extinction, setExtinction)
PY_CLASS_MEMBER_RW(Fog, assymetry, setAssymetry)
PY_CLASS_MEMBER_RW(Fog, color, setColor)
PY_CLASS_MEMBER_RW(Fog, emission, setEmission)
PY_CLASS_MEMBER_RW(Fog, numScatterSamples, setNumScatterSamples)

// --- public --------------------------------------------------------------------------------------

Fog::Fog()
{
	init();
}



Fog::Fog(TValue extinction, TValue assymetry)
{
	init(extinction, assymetry);
}



Fog::TValue Fog::extinction() const
{
	return extinction_;
}



void Fog::setExtinction(TValue extinction)
{
	extinction_ = std::max<TValue>(extinction, 0);
}



Fog::TValue Fog::assymetry() const
{
	return assymetry_;
}



void Fog::setAssymetry(TValue g)
{
	assymetry_ = num::clamp<TValue>(g, -1, 1);
}



/** diffuse color of particles.
 *  @code
 *  sigma_e = sigma_a + sigma_s // extinction = scattering + absorption.
 *  -> sigma_s = color * sigma_e
 *  -> sigma_a = (1 - color) * sigma_e
 *  @endcode
 */
const TSpectrumPtr& Fog::color() const
{
	return color_;
}



void Fog::setColor(const TSpectrumPtr& color)
{
	color_ = color;
}



const TSpectrumPtr& Fog::emission() const
{
	return emission_;
}



void Fog::setEmission(const TSpectrumPtr& emission)
{
	emission_ = emission;
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



const Spectral Fog::doTransmittance(const Sample&, const BoundedRay& ray) const
{
	const TValue d = static_cast<TValue>(ray.farLimit() - ray.nearLimit());
	LASS_ASSERT(d >= 0 && extinction_ >= 0);
	const TValue thickness = extinction_ * d;
	return Spectral(num::exp(-thickness));
}



const Spectral Fog::doEmission(const Sample& sample, const BoundedRay& ray) const
{
	const TValue d = static_cast<TValue>(ray.farLimit() - ray.nearLimit());
	LASS_ASSERT(d >= 0 && extinction_ >= 0);
	const TValue thickness = extinction_ * d;
	if (thickness < 1e-5f)
	{
		return emission_->evaluate(sample, SpectralType::Illuminant) * d;
	}
	const TValue absorptance = -num::expm1(-thickness); // = 1 - transmittance(ray)
	return emission_->evaluate(sample, SpectralType::Illuminant) * (absorptance / extinction_);

}



const Spectral Fog::doScatterOut(const Sample& sample, const BoundedRay& ray) const
{
	return extinction_ * transmittance(sample, ray);
}



const Spectral Fog::doSampleScatterOut(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const
{
	const TValue dMax = static_cast<TValue>(ray.farLimit() - ray.nearLimit());
	const TValue attMax = 1 - num::exp(-extinction_ * dMax);
	if (attMax <= 0)
	{
		pdf = 0;
		return Spectral();
	}
	TValue p;
	const TValue d = num::uniformToExponential(static_cast<TValue>(sample) * attMax, extinction_, p);
	tScatter = std::min(ray.nearLimit() + d, ray.farLimit());
	pdf = p / attMax;
	return Spectral(p);
}


const Spectral Fog::doSampleScatterOutOrTransmittance(const Sample&, TScalar scatterSample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const
{
	pdf = 1;
	const TScalar dMax = ray.farLimit() - ray.nearLimit();
	const TScalar d = num::uniformToExponential(scatterSample, static_cast<TScalar>(extinction_), pdf);
	if (d > dMax)
	{
		// full transmission
		tScatter = ray.farLimit();
		pdf = num::exp(-extinction_ * dMax); // = 1 - cdf(tMax);
	}
	else
	{
		// the photon has hit a particle. we always assume it's scattered.
		// the callee has to russian roulette for absorption himself.
		tScatter = ray.nearLimit() + d;
	}
	return Spectral(static_cast<TValue>(pdf));
}


const Spectral Fog::doPhase(const Sample& sample, const TPoint3D&, const TVector3D& dirIn, const TVector3D& dirOut, TScalar& pdf) const
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
	return color_->evaluate(sample, SpectralType::Reflectant) * static_cast<TValue>(p);
}



const Spectral Fog::doSamplePhase(const Sample& sample, const TPoint2D& phaseSample, const TPoint3D&, const TVector3D& dirIn, TVector3D& dirOut, TScalar& pdf) const
{
	const TScalar g = assymetry_;
	const TScalar p = 2 * phaseSample.x - 1;

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
	const TScalar phi = 2 * TNumTraits::pi * phaseSample.y;
	const TScalar sinTheta = num::sqrt(std::max<TScalar>(1 - num::sqr(cosTheta), 0));
	dirOut = cosTheta * dirIn + (sinTheta * num::cos(phi)) * u + (sinTheta * num::sin(phi)) * v;

	return color_->evaluate(sample, SpectralType::Reflectant) * static_cast<TValue>(pdf);
}



void Fog::init(TValue extinction, TValue assymetry, const TSpectrumPtr& color, const TSpectrumPtr& emission, size_t numSamples)
{
	setExtinction(extinction);
	setEmission(emission);
	setAssymetry(assymetry);
	setColor(color);
	setNumScatterSamples(numSamples);
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
