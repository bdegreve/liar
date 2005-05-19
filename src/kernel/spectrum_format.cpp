/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LIAR isn't a raytracer
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

#include "kernel_common.h"
#include "spectrum_format.h"
#include <lass/stde/extended_iterator.h>

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS(SpectrumFormat);
PY_CLASS_CONSTRUCTOR_0(SpectrumFormat);
PY_CLASS_CONSTRUCTOR_1(SpectrumFormat, unsigned);
PY_CLASS_CONSTRUCTOR_3(SpectrumFormat, unsigned, TScalar, TScalar);
PY_CLASS_CONSTRUCTOR_1(SpectrumFormat, const SpectrumFormat::TFrequencies&);
PY_CLASS_MEMBER_R(SpectrumFormat, "numberOfBands", numberOfBands);
PY_CLASS_METHOD(SpectrumFormat, beginFrequency);
PY_CLASS_METHOD(SpectrumFormat, endFrequency);
PY_CLASS_METHOD(SpectrumFormat, centerFrequency);

// --- public --------------------------------------------------------------------------------------

SpectrumFormat::SpectrumFormat():
	PyObjectPlus(&Type)
{
	boundaries_ = generateBoundaries();
	centres_ = centresFromBoundaries(boundaries_);
}



SpectrumFormat::SpectrumFormat(unsigned iNumberOfBands):
	PyObjectPlus(&Type)
{
	boundaries_ = generateBoundaries(iNumberOfBands);
	centres_ = centresFromBoundaries(boundaries_);
}



SpectrumFormat::SpectrumFormat(unsigned iNumberOfBands, TParam iBeginFrequency, 
							   TParam iEndFrequency):
	PyObjectPlus(&Type)
{
	boundaries_ = generateBoundaries(iNumberOfBands, iBeginFrequency, iEndFrequency);
	centres_ = centresFromBoundaries(boundaries_);
}



SpectrumFormat::SpectrumFormat(const TFrequencies& iBandBoundaries):
	PyObjectPlus(&Type),
	boundaries_(iBandBoundaries)
{
	centres_ = centresFromBoundaries(boundaries_);
}



const unsigned SpectrumFormat::numberOfBands() const
{
	return static_cast<unsigned>(centres_.size());
}



const TScalar SpectrumFormat::beginFrequency(unsigned iBand) const
{
	LASS_ASSERT(iBand < numberOfBands());
	return boundaries_[iBand];
}



const TScalar SpectrumFormat::endFrequency(unsigned iBand) const
{
	LASS_ASSERT(iBand < numberOfBands());
	return boundaries_[iBand + 1];
}



const TScalar SpectrumFormat::centerFrequency(unsigned iBand) const
{
	LASS_ASSERT(iBand < numberOfBands());
	return centres_[iBand];
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const SpectrumFormat::TFrequencies
SpectrumFormat::generateBoundaries(unsigned iNumberOfBands, TParam iBeginFrequency, 
								   TParam iEndFrequency)
{
	if (iNumberOfBands < 1)
	{
		LASS_THROW("A spectrum format must have at leaste one band.  "
			"Please, specify iNumberOfBands >= 1).");
	}
	if (iBeginFrequency <= TNumTraits::zero || iEndFrequency <= iBeginFrequency)
	{
		LASS_THROW("Invalid begin and end frequency.  Please, specify them so that "
			"'begin frequency' > 0 and 'end frequency' > 'begin frequency'.");
	}

	const TValue logBegin = num::log(iBeginFrequency);
	const TValue logEnd = num::log(iEndFrequency);
	const TValue logDelta = (logEnd - logBegin) / iNumberOfBands;

	TFrequencies result;
	for (unsigned i = 0; i < iNumberOfBands; ++i)
	{
		result.push_back(num::exp(logBegin + i * logDelta));
	}
	result.push_back(iEndFrequency);

	return result;
}



const SpectrumFormat::TFrequencies 
SpectrumFormat::centresFromBoundaries(const TFrequencies& iBoundaries) const
{
	if (!isValidFrequencySequence(boundaries_))
	{
		LASS_THROW("The specified boundaries frequencies are invalid.  Please, provide a sequence "
			"of frequencies f[i] so that (a) all elements are greater than zero (f[i] > 0, "
			"for all i) and (b) the sequence is increasing (f[i] < f[i + 1], for all i).");
	}

	TFrequencies result;

	const TFrequencies::const_iterator end = stde::prior(iBoundaries.end());
	for (TFrequencies::const_iterator i = iBoundaries.begin(); i != end; ++i)
	{
		result.push_back(num::sqrt((*i) * (*stde::next(i))));
	}

	return result;
}



const bool SpectrumFormat::isValidFrequencySequence(const TFrequencies& iSequence) const
{
	const TFrequencies::const_iterator end = stde::prior(iSequence.end());
	for (TFrequencies::const_iterator i = iSequence.begin(); i != end; ++i)
	{
		if (*i <= TNumTraits::zero)
		{
			return false;
		}
		if (*i >= *stde::next(i))
		{
			return false;
		}
	}
	return true;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF