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

/** @class liar::mediums::Bounded
 *  @brief bounds a medium with an AABB
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_MEDIUMS_BOUNDED_H
#define LIAR_GUARDIAN_OF_INCLUSION_MEDIUMS_BOUNDED_H

#include "mediums_common.h"
#include "../kernel/medium.h"

namespace liar
{
namespace mediums
{

class LIAR_MEDIUMS_DLL Bounded: public Medium
{
	PY_HEADER(Medium)
public:

	Bounded();
	Bounded(const TMediumPtr& child, const TAabb3D& bounds);

	const TMediumPtr& child() const;
	void setChild(const TMediumPtr& child);

	const TAabb3D& bounds() const;
	void setBounds(const TAabb3D& bounds);

private:
	size_t doNumScatterSamples() const;
	const Spectrum doTransmittance(const BoundedRay& ray) const;
	const Spectrum doEmission(const BoundedRay& ray) const;
	const Spectrum doScatterOut(const BoundedRay& ray) const;
	const Spectrum doSampleScatterOut(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const;
	const Spectrum doSampleScatterOutOrTransmittance(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const;
	const Spectrum doPhase(const TPoint3D&, const TVector3D&, const TVector3D&, TScalar& pdf) const;
	const Spectrum doSamplePhase(const TPoint2D& sample, const TPoint3D& position, const TVector3D& dirIn, TVector3D& dirOut, TScalar& pdf) const;

	bool bound(const BoundedRay& ray, BoundedRay& bounded) const;

	TMediumPtr child_;
	TAabb3D bounds_;
};

}

}

#endif

// EOF
