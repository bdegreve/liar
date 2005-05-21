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

#include "kernel_common.h"
#include "spectrum.h"
#include <lass/stde/extended_iterator.h>

namespace liar
{
namespace kernel
{

TSpectrumFormatPtr Spectrum::defaultFormat_ = TSpectrumFormatPtr(new SpectrumFormat(3));

// --- public --------------------------------------------------------------------------------------

Spectrum::Spectrum(const TSpectrumFormatPtr& iFormat):
	powers_(iFormat->numberOfBands()),
	alpha_(TNumTraits::zero),
	format_(iFormat)
{
}



const TSpectrumFormatPtr& Spectrum::format() const
{
	return format_;
}



void Spectrum::setFormat(const TSpectrumFormatPtr& iFormat)
{
	powers_ = convertPowers(powers_, format_, iFormat);
	format_ = iFormat;
}



const unsigned Spectrum::numberOfBands() const
{
	return format_->numberOfBands();
}



Spectrum::TParam Spectrum::power(unsigned iBand) const
{
	LASS_ASSERT(iBand < numberOfBands());
	return powers_[iBand];
}



Spectrum::TReference Spectrum::power(unsigned iBand)
{
	LASS_ASSERT(iBand < numberOfBands());
	return powers_[iBand];
}



const Spectrum::TValue Spectrum::totalPower() const
{
	return powers_.sum();
}



Spectrum::TParam Spectrum::alpha() const
{
	return alpha_;
}



Spectrum::TReference Spectrum::alpha()
{
	return alpha_;
}



Spectrum& Spectrum::operator+=(TParam iOther)
{
	powers_ += iOther;
	alpha_ += iOther;
	return *this;
}



Spectrum& Spectrum::operator-=(TParam iOther)
{
	powers_ -= iOther;
	alpha_ -= iOther;
	return *this;
}



Spectrum& Spectrum::operator*=(TParam iOther)
{
	powers_ *= iOther;
	alpha_ *= iOther;
	return *this;
}



Spectrum& Spectrum::operator/=(TParam iOther)
{
	powers_ /= iOther;
	alpha_ /= iOther;
	return *this;
}



Spectrum& Spectrum::operator+=(const Spectrum& iOther)
{
	if (format_ == iOther.format_)
	{
		powers_ += iOther.powers_;
	}
	else
	{
		powers_ += convertPowers(iOther.powers_, iOther.format_, format_);
	}

	alpha_ += iOther.alpha_;

	return *this;
}



Spectrum& Spectrum::operator-=(const Spectrum& iOther)
{
	if (format_ == iOther.format_)
	{
		powers_ -= iOther.powers_;
	}
	else
	{
		powers_ -= convertPowers(iOther.powers_, iOther.format_, format_);
	}

	alpha_ -= iOther.alpha_;

	return *this;
}



Spectrum& Spectrum::operator*=(const Spectrum& iOther)
{
	if (format_ == iOther.format_)
	{
		powers_ *= iOther.powers_;
	}
	else
	{
		powers_ *= convertPowers(iOther.powers_, iOther.format_, format_);
	}

	alpha_ *= iOther.alpha_;

	return *this;
}



Spectrum& Spectrum::operator/=(const Spectrum& iOther)
{
	if (format_ == iOther.format_)
	{
		powers_ /= iOther.powers_;
	}
	else
	{
		powers_ /= convertPowers(iOther.powers_, iOther.format_, format_);
	}

	alpha_ /= iOther.alpha_;

	return *this;
}



TSpectrumFormatPtr& Spectrum::defaultFormat()
{
	return defaultFormat_;
}



Spectrum Spectrum::empty()
{
	return Spectrum(defaultFormat_);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

Spectrum::TPowers Spectrum::convertPowers(const TPowers& iPowersA,
										  const TSpectrumFormatPtr& iFormatA,										  										  
										  const TSpectrumFormatPtr& iFormatB)
{
	const unsigned numBandsA = iFormatA->numberOfBands();
	const unsigned numBandsB = iFormatB->numberOfBands();

	TPowers powersB(numBandsB);

	TValue begin = std::max(iFormatA->beginFrequency(0), iFormatB->beginFrequency(0));
	unsigned iA = 0;
	while (iFormatA->endFrequency(iA) <= begin && iA < numBandsA)
	{
		++iA;
	}

	unsigned iB = 0;
	while (iFormatB->endFrequency(iB) <= begin && iB < numBandsB)
	{
		++iB;
	}

	TValue endB = iFormatB->endFrequency(iB);
	while (iA < numBandsA)
	{
		const TValue endA = iFormatA->endFrequency(iA);
		const TValue powerDensityA = iPowersA[iA] / (endA - iFormatA->beginFrequency(iA));

		while (endB <= endA)
		{
			powersB[iB] += (endB - begin) * powerDensityA;
			begin = endB;
			++iB;

			if (iB == numBandsB)
			{
				return powersB;
			}
			endB = iFormatB->endFrequency(iB);
		}

		powersB[iB] += (endA - begin) * powerDensityA;
		begin = endA;
		++iA;
	}

	return powersB;
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF