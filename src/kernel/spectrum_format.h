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
#include <lass/num/spline_linear.h>

namespace liar
{
namespace kernel
{

class SpectrumFormat;
typedef python::PyObjectPtr<SpectrumFormat>::Type TSpectrumFormatPtr;

class LIAR_KERNEL_DLL SpectrumFormat: public python::PyObjectPlus
{
	PY_HEADER(python::PyObjectPlus)
public:

	typedef util::CallTraits<TScalar>::TValue TValue;
	typedef util::CallTraits<TScalar>::TParam TParam;
	typedef util::CallTraits<TScalar>::TConstReference TConstReference;
	typedef util::CallTraits<TScalar>::TReference TReference;

	typedef std::vector<TScalar> TWavelengths;
	typedef std::vector<TVector3D> TXyzWeights;

	SpectrumFormat();
	SpectrumFormat(unsigned numberOfBands);
	SpectrumFormat(unsigned numberOfBands, TScalar iBeginWavelength, TScalar iEndWavelength);
    SpectrumFormat(const TWavelengths& iBandBoundaries);
    SpectrumFormat(const TWavelengths& iBandBoundaries, const TXyzWeights& iXyzWeights);

	const unsigned numberOfBands() const;
	const TScalar totalBandWidth() const;

	const TScalar beginWavelength(unsigned iBand) const;
	const TScalar endWavelength(unsigned iBand) const;
	const TScalar centerWavelength(unsigned iBand) const;
	const TScalar bandWidth(unsigned iBand) const;
	const TVector3D xyzWeight(unsigned iBand) const;

	static const TSpectrumFormatPtr& defaultFormat();
	static void setDefaultFormat(const TSpectrumFormatPtr& iFormat);

private:

	typedef num::SplineLinear<TScalar, TVector3D, num::DataTraitsStaticVector<TVector3D> > TXyzSpline;

	const TWavelengths generateBoundaries(unsigned iNumberOfBands, 
		TParam iBeginWavelength, TParam iEndWavelength);
	const TWavelengths centresFromBoundaries(const TWavelengths& iWavelengths) const;
	const TXyzWeights xyzWeightsFromBoundaries(const TWavelengths& iWavelengths) const;
	const bool isValidWavelengthSequence(const TWavelengths& iSequence) const;

	static TXyzSpline loadStandardObserver();

	TWavelengths boundaries_;
	TWavelengths centres_;
	TXyzWeights xyzWeights_;

	static TXyzSpline standardObserver_;
	static TSpectrumFormatPtr defaultFormat_;
};

}

}

#endif

// EOF
