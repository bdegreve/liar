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
#include "beer.h"

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS(Beer)
PY_CLASS_CONSTRUCTOR_0(Beer)
PY_CLASS_CONSTRUCTOR_1(Beer, const Spectrum&)
PY_CLASS_MEMBER_RW(Beer, transparency, setTransparency)

// --- public --------------------------------------------------------------------------------------

Beer::Beer():
	transparency_(1)
{
}



Beer::Beer(const Spectrum& transparency):
	transparency_(transparency)
{
}



const Spectrum& Beer::transparency() const
{
	return transparency_;
}



void Beer::setTransparency(const Spectrum& transparency)
{
	transparency_ = transparency;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const Spectrum Beer::doTransparency(const BoundedRay& ray) const
{
	const TScalar t = ray.farLimit() - ray.nearLimit();
	LASS_ASSERT(t >= 0);
	return pow(transparency_, t);
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
