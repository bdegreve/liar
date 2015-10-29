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

/** @class liar::mediums::Transformation
 *  @brief applies world to local transformation to rays traveling through medium.
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_MEDIUMS_TRANSFORMATION_H
#define LIAR_GUARDIAN_OF_INCLUSION_MEDIUMS_TRANSFORMATION_H

#include "mediums_common.h"
#include "../kernel/medium.h"
#include "../kernel/transformation.h"

namespace liar
{
namespace mediums
{

class LIAR_MEDIUMS_DLL Transformation: public Medium
{
	PY_HEADER(Medium)
public:

	Transformation(const TMediumPtr& child, const TTransformation3D& localToWorld);
	Transformation(const TMediumPtr& child, const TPyTransformation3DPtr& localToWorld);

	const TMediumPtr& child() const;
	void setChild(const TMediumPtr& child);

	const TTransformation3D& localToWorld() const;
	void setLocalToWorld(const TTransformation3D& localToWorld);

private:
	size_t doNumScatterSamples() const;
	const Spectral doTransmittance(const BoundedRay& ray) const;
	const Spectral doEmission(const BoundedRay& ray) const;
	const Spectral doScatterOut(const BoundedRay& ray) const;
	const Spectral doSampleScatterOut(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const;
	const Spectral doSampleScatterOutOrTransmittance(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const;
	const Spectral doPhase(const TPoint3D&, const TVector3D&, const TVector3D&, TScalar& pdf) const;
	const Spectral doSamplePhase(const TPoint2D& sample, const TPoint3D& position, const TVector3D& dirIn, TVector3D& dirOut, TScalar& pdf) const;

	bool bound(const BoundedRay& ray, BoundedRay& bounded) const;

	TMediumPtr child_;
	TTransformation3D localToWorld_;
	TTransformation3D worldToLocal_;
};

}

}

#endif

// EOF
