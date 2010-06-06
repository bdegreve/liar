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
#include "beer.h"

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(Beer, "Beer's Law")
PY_CLASS_CONSTRUCTOR_0(Beer)
PY_CLASS_CONSTRUCTOR_1(Beer, const XYZ&)
PY_CLASS_MEMBER_RW(Beer, transparency, setTransparency)

// --- public --------------------------------------------------------------------------------------

Beer::Beer():
	transparency_(XYZ(1, 1, 1))
{
}



Beer::Beer(const XYZ& transparency):
	transparency_(transparency)
{
}



const XYZ& Beer::transparency() const
{
	return transparency_;
}



void Beer::setTransparency(const XYZ& transparency)
{
	transparency_ = transparency;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const XYZ Beer::doTransmittance(const BoundedRay& ray) const
{
	const TScalar t = ray.farLimit() - ray.nearLimit();
	LASS_ASSERT(t >= 0);
	return pow(transparency_, t);
}



const XYZ Beer::doScatterOut(const BoundedRay&) const
{
	return XYZ(0);
}



const XYZ Beer::doSampleScatterOut(TScalar, const BoundedRay&, TScalar&, TScalar& pdf) const
{
	pdf = 0;
	return XYZ(0);
}



/** As we don't do any scattering in Beer, all incoming photons exit at the end (albeit attenuated)
 */
const XYZ Beer::doSampleScatterOutOrTransmittance(TScalar, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const
{
	tScatter = ray.farLimit();
	pdf = 1;
	return transmittance(ray);
}



const XYZ Beer::doPhase(const TPoint3D&, const TVector3D&, const TVector3D&, TScalar& pdf) const
{
	pdf = 0;
	return XYZ(0);
}



const XYZ Beer::doSamplePhase(const TPoint2D&, const TPoint3D&, const TVector3D&, TVector3D&, TScalar& pdf) const
{
	pdf = 0;
	return XYZ(0);
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
