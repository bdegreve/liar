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
#include "exponential_fog.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace mediums
{

PY_DECLARE_CLASS_DOC(ExponentialFog, "")
PY_CLASS_CONSTRUCTOR_0(ExponentialFog)
PY_CLASS_CONSTRUCTOR_2(ExponentialFog, ExponentialFog::TValue, ExponentialFog::TValue)
PY_CLASS_CONSTRUCTOR_3(ExponentialFog, ExponentialFog::TValue, ExponentialFog::TValue, ExponentialFog::TValue)
PY_CLASS_MEMBER_RW(ExponentialFog, extinction, setExtinction)
PY_CLASS_MEMBER_RW(ExponentialFog, assymetry, setAssymetry)
PY_CLASS_MEMBER_RW(ExponentialFog, color, setColor)
PY_CLASS_MEMBER_RW(ExponentialFog, numScatterSamples, setNumScatterSamples)
PY_CLASS_MEMBER_RW(ExponentialFog, origin, setOrigin)
PY_CLASS_MEMBER_RW(ExponentialFog, up, setUp)
PY_CLASS_MEMBER_RW(ExponentialFog, decay, setDecay)

// --- public --------------------------------------------------------------------------------------

ExponentialFog::ExponentialFog():
	Fog()
{
	init();
}



ExponentialFog::ExponentialFog(TValue extinction, TValue assymetry) :
	Fog(extinction, assymetry)
{
	init();
}



ExponentialFog::ExponentialFog(TValue extinction, TValue assymetry, TValue decay) :
	Fog(extinction, assymetry)
{
	init(decay);
}



const TPoint3D& ExponentialFog::origin() const
{
	return origin_;
}



void ExponentialFog::setOrigin(const TPoint3D& origin)
{
	origin_ = origin;
}



const TVector3D& ExponentialFog::up() const
{
	return up_;
}



void ExponentialFog::setUp(const TVector3D& up)
{
	up_ = up.normal();
}



ExponentialFog::TValue ExponentialFog::decay() const
{
	return decay_;
}



void ExponentialFog::setDecay(TValue decay)
{
	decay_ = std::max<TValue>(decay, 0);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

namespace
{

typedef ExponentialFog::TValue TValue;

inline TValue sigma(TValue alpha, TValue beta, TValue t)
{
	return alpha * num::exp(-beta * t);
}

template <typename T>
inline T tau(T alpha, TValue beta, TValue d)
{
	const TValue bd = beta * d;
	if (num::abs(bd) < 1e-5f)
	{
		return alpha * d * (1 - bd / 2);
	}
	return alpha / beta * -num::expm1(-bd);
}

inline TValue trans(TValue alpha, TValue beta, TValue d)
{
	return num::exp(-tau(alpha, beta, d));
}

inline TValue invCdf(TValue uniform, TValue alpha, TValue beta, TValue& pdf)
{
	if (alpha == 0)
	{
		pdf = 0;
		return num::NumTraits<TValue>::infinity;
	}
	if (beta == 0)
	{
		return num::uniformToExponential(uniform, alpha, pdf);
	}
	TValue d = -num::log1p( num::log1p(std::max<TValue>(-1, -uniform) ) * (beta / alpha)) / beta;
	pdf = alpha * num::exp(-beta * d - tau(alpha, beta, d));
	return d;
}

}

const Spectral ExponentialFog::doTransmittance(const Sample&, const BoundedRay& ray) const
{
	const TValue d = static_cast<TValue>(ray.farLimit() - ray.nearLimit());
	const TValue a = alpha(ray);
	const TValue b = beta(ray);
	LASS_ASSERT(d >= 0 && decay_ >= 0);
	return Spectral(trans(a, b, d));
}



const Spectral ExponentialFog::doEmission(const Sample& sample, const BoundedRay& ray) const
{
	if (!emission()->evaluate(sample, SpectralType::Illuminant))
	{
		return Spectral();
	}

	const TValue d = static_cast<TValue>(ray.farLimit() - ray.nearLimit());
	const TValue a = alpha(ray);
	const TValue b = beta(ray);
	LASS_ASSERT(d >= 0 && a >= 0 && decay_ >= 0);

	if (extinction() == 0) // instead special case for thickness < 1e-5?
	{
		return tau(emission()->evaluate(sample, SpectralType::Illuminant), b, d);
	}

	const TValue thickness = tau(a, b, d);
	LASS_ASSERT(thickness >= 0);

	const TValue absorptance = -num::expm1(-thickness); // = 1 - transmittance(ray)
	return emission()->evaluate(sample, SpectralType::Illuminant) * (absorptance / extinction());
}


const Spectral ExponentialFog::doScatterOut(const Sample&, const BoundedRay& ray) const
{
	const TValue d = static_cast<TValue>(ray.farLimit() - ray.nearLimit());
	const TValue a = alpha(ray);
	const TValue b = beta(ray);
	LASS_ASSERT(d >= 0 && decay_ >= 0);
	return Spectral(sigma(a, b, d) * trans(a, b, d));
}



const Spectral ExponentialFog::doSampleScatterOut(TScalar scatterSample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const
{
	const TValue dMax = static_cast<TValue>(ray.farLimit() - ray.nearLimit());
	const TValue a = alpha(ray);
	const TValue b = beta(ray);
	LASS_ASSERT(dMax >= 0 && a >= 0 && decay_ >= 0);

	const TValue attMax = -num::expm1(-tau(a, b, dMax)); // = 1 - trans(dMax, a, b);
	if (attMax <= 0)
	{
		pdf = 0;
		return Spectral();
	}

	TValue p;
	const TValue d = invCdf(static_cast<TValue>(scatterSample) * attMax, a, b, p);
	tScatter = std::min(ray.nearLimit() + d, ray.farLimit());
	pdf = p / attMax;
	return Spectral(p);
}


const Spectral ExponentialFog::doSampleScatterOutOrTransmittance(const Sample&, TScalar scatterSample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const
{
	const TValue dMax = static_cast<TValue>(ray.farLimit() - ray.nearLimit());
	const TValue a = alpha(ray);
	const TValue b = beta(ray);
	LASS_ASSERT(dMax >= 0 && a >= 0 && decay_ >= 0);

	const TValue attMax = b > 0 ? -num::expm1(-a / b) : 1;
	if (attMax <= 0)
	{
		pdf = 0;
		return Spectral();
	}

	TValue p;
	const TValue d = invCdf(static_cast<TValue>(scatterSample) * attMax, a, b, p);
	if (d > dMax)
	{
		// full transmission
		tScatter = ray.farLimit();
		pdf = trans(a, b, dMax); // = 1 - cdf(tMax);
		return Spectral(static_cast<TValue>(pdf));
	}

	// the photon has hit a particle. we always assume it's scattered.
	// the callee has to russian roulette for absorption himself.
	tScatter = ray.nearLimit() + d;
	pdf = p / attMax;
	return Spectral(p);
}



void ExponentialFog::init(TValue decay)
{
	setOrigin(TPoint3D(0, 0, 0));
	setUp(TVector3D(0, 0, 1));
	setDecay(decay);
}



inline ExponentialFog::TValue ExponentialFog::alpha(const BoundedRay& ray) const
{
	const TValue h1 = static_cast<TValue>(dot(ray.nearPoint() - origin_, up_));
	return extinction() * num::exp(-decay_ * h1);
}



inline ExponentialFog::TValue ExponentialFog::beta(const BoundedRay& ray) const
{
	const TValue cosTheta = static_cast<TValue>(dot(ray.direction(), up_));
	LASS_ASSERT(cosTheta >= -1 && cosTheta <= 1);
	return decay_ * cosTheta;
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
