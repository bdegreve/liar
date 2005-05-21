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

/** @class liar::kernel::Spectrum
 *  @brief respresentation of a colour spectrum.
 *  @author Bram de Greve [BdG]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SPECTRUM_FORMAT_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SPECTRUM_FORMAT_H

#include "kernel_common.h"

namespace liar
{
namespace kernel
{

class SpectrumFormat: public python::PyObjectPlus
{
	PY_HEADER(python::PyObjectPlus)
public:

	typedef util::CallTraits<TScalar>::TValue TValue;
	typedef util::CallTraits<TScalar>::TParam TParam;
	typedef util::CallTraits<TScalar>::TConstReference TConstReference;
	typedef util::CallTraits<TScalar>::TReference TReference;

	typedef std::vector<TScalar> TFrequencies;

	SpectrumFormat();
	SpectrumFormat(unsigned numberOfBands);
	SpectrumFormat(unsigned numberOfBands, TParam iBeginFrequency, TParam iEndFrequency);
    SpectrumFormat(const TFrequencies& iBandBoundaries);

	const unsigned numberOfBands() const;

	const TScalar beginFrequency(unsigned iBand) const;
	const TScalar endFrequency(unsigned iBand) const;
	const TScalar centerFrequency(unsigned iBand) const;

private:

	const TFrequencies generateBoundaries(unsigned iNumberOfBands = 3, 
		TParam iBeginFrequency = 380e-9f, TParam iEndFrequency = 780e-9f);
	const TFrequencies centresFromBoundaries(const TFrequencies& iFrequencies) const;
	const bool isValidFrequencySequence(const TFrequencies& iSequence) const;

	TFrequencies boundaries_;
	TFrequencies centres_;
};

typedef python::PyObjectPtr<SpectrumFormat>::Type TSpectrumFormatPtr;

}

}

#endif

// EOF
