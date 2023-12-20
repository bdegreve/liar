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

/** @class liar::Recovery
*  @brief abstract base class of recovery models to obtain spectral data from XYZ tristimulus values.
*  @author Bram de Greve [Bramz]
*/

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_RECOVERY_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_RECOVERY_H

#include "kernel_common.h"
#include "spectral.h"
#include "xyz.h"

namespace liar
{
namespace kernel
{

class Recovery;
class Sample;

typedef python::PyObjectPtr<Recovery>::Type TRecoveryPtr;

class LIAR_KERNEL_DLL Recovery: public python::PyObjectPlus
{
	PY_HEADER(python::PyObjectPlus)
public:

	using TValue = Spectral::TValue;

	virtual ~Recovery();

	Spectral recover(const XYZ& xyz, const Sample& sample, SpectralType type) const;
	TValue recover(const XYZ& xyz, TWavelength wavelength) const;

	static const TRecoveryPtr& standard();
	static void setStandard(const TRecoveryPtr& standard);

protected:

	Recovery();

private:

	virtual Spectral doRecover(const XYZ& xyz, const Sample& sample, SpectralType type) const = 0;
	virtual TValue doRecover(const XYZ& xyz, TWavelength wavelength) const = 0;

	static TRecoveryPtr standard_;
};


LIAR_KERNEL_DLL const Recovery& standardRecovery();

}

}

#endif

// EOF
