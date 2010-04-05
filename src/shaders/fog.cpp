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

#include "shaders_common.h"
#include "fog.h"
#include "../kernel/sample.h"

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(Fog, "")
PY_CLASS_CONSTRUCTOR_0(Fog)
PY_CLASS_CONSTRUCTOR_3(Fog, const XYZ&, const XYZ&, TScalar)
PY_CLASS_MEMBER_RW(Fog, scattering, setScattering)
PY_CLASS_MEMBER_RW(Fog, assymetry, setAssymetry)
PY_CLASS_MEMBER_RW(Fog, numScatterSamples, setNumScatterSamples)

// --- public --------------------------------------------------------------------------------------

Fog::Fog():
	absorption_(0, 0, 0),
	scattering_(0, 0, 0),
	assymetry_(0),
	numSamples_(1)
{
}



Fog::Fog(const XYZ& absorption, const XYZ& scattering, TScalar assymetry):
	absorption_(absorption),
	scattering_(scattering),
	assymetry_(assymetry),
	numSamples_(1)
{
}



const XYZ& Fog::absorption() const
{
	return absorption_;
}



void Fog::setAbsorption(const XYZ& absorption)
{
	absorption_ = absorption;
}



const XYZ& Fog::scattering() const
{
	return scattering_;
}



void Fog::setScattering(const XYZ& scattering)
{
	scattering_ = scattering;
}



TScalar Fog::assymetry() const
{
	return assymetry_;
}



void Fog::setAssymetry(TScalar g)
{
	assymetry_ = num::clamp<TScalar>(g, -1, 1);
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



const XYZ Fog::doTransparency(const BoundedRay& ray) const
{
	const TScalar t = ray.farLimit() - ray.nearLimit();
	LASS_ASSERT(t >= 0);
	return exp((absorption_ + scattering_) * -t); 
}



const XYZ Fog::doPhase(const TPoint3D& pos, const TVector3D& dirIn, const TVector3D& dirOut) const
{
	const TScalar cosTheta = dot(dirIn, dirOut);
	const TScalar g = assymetry_;

	const TScalar p = (1 - num::sqr(g)) / (4 * TNumTraits::pi * num::pow(1 + num::sqr(g) - 2 * g * cosTheta, 1.5));

	return scattering_ * p;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
