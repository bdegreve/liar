/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LIAR isn't a raytracer
 *  Copyright (C) 2004  Bram de Greve
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

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SPECTRUM_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SPECTRUM_H

#include "kernel_common.h"
#include "spectrum_format.h"
#include <lass/num/vector.h>

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL Spectrum
{
public:

	typedef util::CallTraits<TScalar>::TValue TValue;
	typedef util::CallTraits<TScalar>::TParam TParam;
	typedef util::CallTraits<TScalar>::TConstReference TConstReference;
	typedef util::CallTraits<TScalar>::TReference TReference;

	explicit Spectrum(const TSpectrumFormatPtr& iFormat = defaultFormat());

	const TSpectrumFormatPtr& format() const;
	void setFormat(const TSpectrumFormatPtr& iFormat);
	const unsigned numberOfBands() const;

	TParam power(unsigned iBand) const;
	TReference power(unsigned iBand);
	const TValue totalPower() const;
	
	TParam alpha() const;
	TReference alpha();
	
	Spectrum& Spectrum::operator+=(TParam iOther);
	Spectrum& Spectrum::operator-=(TParam iOther);
	Spectrum& Spectrum::operator*=(TParam iOther);
	Spectrum& Spectrum::operator/=(TParam iOther);

	Spectrum& Spectrum::operator+=(const Spectrum& iOther);
	Spectrum& Spectrum::operator-=(const Spectrum& iOther);
	Spectrum& Spectrum::operator*=(const Spectrum& iOther);
	Spectrum& Spectrum::operator/=(const Spectrum& iOther);

	void swap(Spectrum& iOther);

	static Spectrum empty();
	static TSpectrumFormatPtr& defaultFormat();

private:

	typedef num::Vector<TScalar> TPowers;

	static Spectrum::TPowers Spectrum::convertPowers(const TPowers& iPowersA,
		const TSpectrumFormatPtr& iFormatA, const TSpectrumFormatPtr& iFormatB);

	TPowers powers_;
	TValue alpha_;
	TSpectrumFormatPtr format_;

	static TSpectrumFormatPtr defaultFormat_;
};

}

}

#endif

// EOF
