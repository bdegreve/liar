/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2005  Bram de Greve
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  http://liar.sourceforge.net
 */

#include "textures_common.h"
#include "time.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS(Time)
PY_CLASS_CONSTRUCTOR_0(Time);

// --- public --------------------------------------------------------------------------------------

Time::Time():
	Texture(&Type)
{
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const Spectrum Time::doLookUp(const Sample& iSample, const IntersectionContext& iContext) const
{
	return Spectrum(static_cast<TScalar>(iSample.time()));
}



const TPyObjectPtr Time::doGetState() const
{
	return python::makeTuple();
}



void Time::doSetState(const TPyObjectPtr& iState)
{
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

