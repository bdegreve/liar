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

/** @class liar::shaders::Exponential
 *  @brief Fog with exponentially decreasing density
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SHADERS_EXPONENTIAL_H
#define LIAR_GUARDIAN_OF_INCLUSION_SHADERS_EXPONENTIAL_H

#include "shaders_common.h"
#include "fog.h"

namespace liar
{
namespace shaders
{

class LIAR_SHADERS_DLL ExponentialFog: public Fog
{
	PY_HEADER(Fog)
public:

	ExponentialFog();
	ExponentialFog(TScalar extinction, TScalar assymetry);
	ExponentialFog(TScalar extinction, TScalar assymetry, TScalar decay);

	const TPoint3D& origin() const;
	void setOrigin(const TPoint3D& origin);

	const TVector3D& up() const;
	void setUp(const TVector3D& up);

	TScalar decay() const;
	void setDecay(TScalar decay);

private:

	const XYZ doTransmittance(const BoundedRay& ray) const;
	const XYZ doEmission(const BoundedRay& ray) const;
	const XYZ doScatterOut(const BoundedRay& ray) const;
	const XYZ doSampleScatterOut(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const;
	const XYZ doSampleScatterOutOrTransmittance(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const;

	void init(TScalar decay = 1);
	TScalar alpha(const BoundedRay& ray) const;
	TScalar beta(const BoundedRay& ray) const;

	TPoint3D origin_;
	TVector3D up_;
	TScalar decay_;
};

}

}

#endif

// EOF

