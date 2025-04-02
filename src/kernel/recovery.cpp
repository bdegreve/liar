/** @file
*  @author Bram de Greve (bramz@users.sourceforge.net)
*
*  LiAR isn't a raytracer
*  Copyright (C) 2004-2023  Bram de Greve (bramz@users.sourceforge.net)
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

#include "kernel_common.h"
#include "recovery.h"
#include "sample.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(Recovery, "Abstract base class of spectral recovery models")
	PY_CLASS_STATIC_METHOD_DOC(Recovery, standard, "standard() -> Recovery");
	PY_CLASS_STATIC_METHOD_DOC(Recovery, setStandard, "setStandard(Recovery) -> None");

TRecoveryPtr Recovery::standard_(0);



// --- public --------------------------------------------------------------------------------------

Recovery::~Recovery()
{
}


Spectral Recovery::recover(const XYZ& xyz, const Sample& sample, SpectralType type) const
{
	return doRecover(xyz, sample, type);
}


Recovery::TValue Recovery::recover(const XYZ& xyz, TWavelength wavelength) const
{
	return doRecover(xyz, wavelength);
}


const TRecoveryPtr& Recovery::standard()
{
	return standard_;
}


void Recovery::setStandard(const TRecoveryPtr& standard)
{
	standard_ = standard;
}



// --- protected -----------------------------------------------------------------------------------

Recovery::Recovery()
{
}



// --- private -------------------------------------------------------------------------------------



// --- free --------------------------------------------------------------------

/** Convenience function that checks if standard recovery is initialized
 */
const Recovery& standardRecovery()
{
	const TRecoveryPtr& standard = Recovery::standard();
	if (!standard)
	{
		LASS_THROW("You must first load a standard recovery model using liar.Recovery.setStandard(recovery)");
	}
	return *standard;
}

}
}

// EOF
