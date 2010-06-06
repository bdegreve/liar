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

/** @class liar::shaders::Beer
 *  @brief Not a "beer" medium, but a "Beer's law medium" ;)
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SHADERS_BEER_H
#define LIAR_GUARDIAN_OF_INCLUSION_SHADERS_BEER_H

#include "shaders_common.h"
#include "../kernel/medium.h"

namespace liar
{
namespace shaders
{

class LIAR_SHADERS_DLL Beer: public Medium
{
    PY_HEADER(Medium)
public:

	Beer();
	Beer(const XYZ& transparency);

	const XYZ& transparency() const;
	void setTransparency(const XYZ& transparency);

private:

	const XYZ doTransmittance(const BoundedRay& ray) const;
	const XYZ doScatterOut(const BoundedRay& ray) const;
	const XYZ doSampleScatterOut(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const;
	const XYZ doSampleScatterOutOrTransmittance(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const;
	const XYZ doPhase(const TPoint3D&, const TVector3D&, const TVector3D&, TScalar&) const;
	const XYZ doSamplePhase(const TPoint2D& sample, const TPoint3D& position, const TVector3D& dirIn, TVector3D& dirOut, TScalar& pdf) const;

	XYZ transparency_;
};

}

}

#endif

// EOF

