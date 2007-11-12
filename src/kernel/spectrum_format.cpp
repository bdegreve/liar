/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
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
 *  http://liar.bramz.org
 */

#include "kernel_common.h"
#include "spectrum_format.h"
#include <lass/stde/extended_iterator.h>

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS(SpectrumFormat);
PY_CLASS_CONSTRUCTOR_1(SpectrumFormat, unsigned);
PY_CLASS_CONSTRUCTOR_3(SpectrumFormat, unsigned, TScalar, TScalar);
PY_CLASS_CONSTRUCTOR_1(SpectrumFormat, const SpectrumFormat::TWavelengths&);
PY_CLASS_CONSTRUCTOR_2(SpectrumFormat, const SpectrumFormat::TWavelengths&,
					   const SpectrumFormat::TXyzWeights&);
PY_CLASS_MEMBER_R(SpectrumFormat, numberOfBands);
PY_CLASS_METHOD(SpectrumFormat, beginWavelength);
PY_CLASS_METHOD(SpectrumFormat, endWavelength);
PY_CLASS_METHOD(SpectrumFormat, centerWavelength);
PY_CLASS_METHOD(SpectrumFormat, xyzWeight);

PY_CLASS_STATIC_METHOD(SpectrumFormat, defaultFormat);
PY_CLASS_STATIC_METHOD(SpectrumFormat, setDefaultFormat);

SpectrumFormat::TXyzSpline SpectrumFormat::standardObserver_ = 
	SpectrumFormat::loadStandardObserver();
TSpectrumFormatPtr SpectrumFormat::defaultFormat_ = TSpectrumFormatPtr(new SpectrumFormat(3));

// --- public --------------------------------------------------------------------------------------

/** construct an N bands format in visible range, using CIE-1964 observer for XYZ weights.
 *
 *  Creates a format with @a iNumberOfBands equally distributed bands in the range 380 nm to 780 nm.
 *  
 *  The CIE-1964 standard observer is used to calculate XYZ tristimulus weights
 */
SpectrumFormat::SpectrumFormat(unsigned iNumberOfBands)
{
	boundaries_ = generateBoundaries(iNumberOfBands, 380e-9f, 780e-9f);
	centres_ = centresFromBoundaries(boundaries_);
	xyzWeights_ = xyzWeightsFromBoundaries(boundaries_);
}



/** construct an N bands format in user specified range, using CIE-1964 observer for XYZ weights.
 *
 *  Creates a format with @a iNumberOfBands equally distributed bands in the range 
 *  @a iBeginWavelength to @a iEndWavelength.  Mind that the wavelength is expressed in meters!
 *  so 380e-9 is 380 nm.  
 *
 *  The CIE-1964 standard observer is used to calculate XYZ tristimulus weights.
 */
SpectrumFormat::SpectrumFormat(unsigned iNumberOfBands, TScalar iBeginWavelength, 
							   TScalar iEndWavelength)
{
	boundaries_ = generateBoundaries(iNumberOfBands, iBeginWavelength, iEndWavelength);
	centres_ = centresFromBoundaries(boundaries_);
	xyzWeights_ = xyzWeightsFromBoundaries(boundaries_);
}



/** construct a format with user specified band boundaries, using CIE-1964 observer for XYZ weights
 *
 *  Creates a format with a vector @a iBandBoundaries as band boundaries.  The number of boundaries
 *  is one more than the resulting number of bands (since you have to stop the last band 
 *  somewhere ;)  Mind that wavelengths are expressed in meters!
 *
 *  The CIE-1964 standard observer is used to calculate XYZ tristimulus weights.
 */
SpectrumFormat::SpectrumFormat(const TWavelengths& iBandBoundaries):
	boundaries_(iBandBoundaries)
{
	centres_ = centresFromBoundaries(boundaries_);
	xyzWeights_ = xyzWeightsFromBoundaries(boundaries_);
}



/** construct a format with user specified band boundaries and user specified XYZ weights.
 *
 *  Creates a format with a vector @a iBandBoundaries as band boundaries.  The number of boundaries
 *  is one more than the resulting number of bands (since you have to stop the last band 
 *  somewhere ;)  Mind that wavelengths are expressed in meters!
 *
 *  The XYZ tristimulus weights are taken from the vector @a iXyzWeights.  The number of weights
 *  must be equal to the number of bands, and thus one less than the number of boundaries.
 */
SpectrumFormat::SpectrumFormat(const TWavelengths& iBandBoundaries,
							   const TXyzWeights& iXyzWeights):
	boundaries_(iBandBoundaries),
	xyzWeights_(iXyzWeights)
{
	centres_ = centresFromBoundaries(boundaries_);
	if (boundaries_.size() != xyzWeights_.size() + 1)
	{
		LASS_THROW("number of band boundaries must be one more than number of XYZ weights");
	}
}



const unsigned SpectrumFormat::numberOfBands() const
{
	return static_cast<unsigned>(centres_.size());
}



const TScalar SpectrumFormat::totalBandWidth() const
{
	return boundaries_.back() - boundaries_.front();
}



const TScalar SpectrumFormat::beginWavelength(unsigned band) const
{
	LASS_ASSERT(band < numberOfBands());
	return boundaries_[band];
}



const TScalar SpectrumFormat::endWavelength(unsigned band) const
{
	LASS_ASSERT(band < numberOfBands());
	return boundaries_[band + 1];
}



const TScalar SpectrumFormat::centerWavelength(unsigned band) const
{
	LASS_ASSERT(band < numberOfBands());
	return centres_[band];
}



const TScalar SpectrumFormat::bandWidth(unsigned band) const
{
	LASS_ASSERT(band < numberOfBands());
	return boundaries_[band + 1] - boundaries_[band];
}



const TVector3D SpectrumFormat::xyzWeight(unsigned band) const
{
	LASS_ASSERT(band < numberOfBands());
	return xyzWeights_[band];
}



const TSpectrumFormatPtr& SpectrumFormat::defaultFormat()
{
	return defaultFormat_;
}



void SpectrumFormat::setDefaultFormat(const TSpectrumFormatPtr& iDefaultFormat)
{
	defaultFormat_ = iDefaultFormat;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const SpectrumFormat::TWavelengths
SpectrumFormat::generateBoundaries(unsigned iNumberOfBands, TParam iBeginWavelength, 
								   TParam iEndWavelength)
{
	if (iNumberOfBands < 1)
	{
		LASS_THROW("A spectrum format must have at leaste one band.  "
			"Please, specify iNumberOfBands >= 1).");
	}
	if (iBeginWavelength <= TNumTraits::zero || iEndWavelength <= iBeginWavelength)
	{
		LASS_THROW("Invalid begin and end wavelength.  Please, specify them so that "
			"0 < 'begin wavelength' < 'end wavelength'.");
	}

	TWavelengths result;
	const TValue delta = (iEndWavelength - iBeginWavelength) / iNumberOfBands;
	for (unsigned i = 0; i < iNumberOfBands; ++i)
	{
		result.push_back(iBeginWavelength + i * delta);
	}
	result.push_back(iEndWavelength);

	return result;
}



const SpectrumFormat::TWavelengths 
SpectrumFormat::centresFromBoundaries(const TWavelengths& iBoundaries) const
{
	if (!isValidWavelengthSequence(iBoundaries))
	{
		LASS_THROW("The specified boundaries wavelenghts are invalid.  Please, provide a sequence "
			"of wavelengths l[i] so that (a) it has at least two elements, (b) all elements are "
			"greater than zero (l[i] > 0, for all i) and (b) the sequence is increasing (l[i] < "
			"l[i + 1], for all i).");
	}

	TWavelengths result;
	const TWavelengths::const_iterator end = stde::prev(iBoundaries.end());
	for (TWavelengths::const_iterator i = iBoundaries.begin(); i != end; ++i)
	{
		result.push_back(((*i) + (*stde::next(i))) / 2);
	}
	return result;
}



const SpectrumFormat::TXyzWeights
SpectrumFormat::xyzWeightsFromBoundaries(const TWavelengths& iBoundaries) const
{
	if (!isValidWavelengthSequence(iBoundaries))
	{
		LASS_THROW("The specified boundaries wavelenghts are invalid.  Please, provide a sequence "
			"of wavelengths l[i] so that (a) it has at least two elements, (b) all elements are "
			"greater than zero (l[i] > 0, for all i) and (b) the sequence is increasing (l[i] < "
			"l[i + 1], for all i).");
	}

	TXyzWeights result;
	const TWavelengths::const_iterator end = stde::prev(iBoundaries.end());
	for (TWavelengths::const_iterator i = iBoundaries.begin(); i != end; ++i)
	{
		result.push_back(standardObserver_.integral(*i, *stde::next(i)));
	}
	return result;
}



const bool SpectrumFormat::isValidWavelengthSequence(const TWavelengths& iSequence) const
{
	if (iSequence.size() < 2)
	{
		return false;
	}

	const TWavelengths::const_iterator end = stde::prev(iSequence.end());
	for (TWavelengths::const_iterator i = iSequence.begin(); i != end; ++i)
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



/** load the XYZ sensitivity curves of the CIE-1964 standard observer.
 * 
 *  http://www.isc.tamu.edu/~astro/color/cie_xyz1964.html
 *  The curves are rescaled to have area 1 for all channels.
 */
SpectrumFormat::TXyzSpline SpectrumFormat::loadStandardObserver()
{
	const TScalar w[] =
	{
		3.70e-007f,
		3.75e-007f,
		3.80e-007f,
		3.85e-007f,
		3.90e-007f,
		3.95e-007f,
		4.00e-007f,
		4.05e-007f,
		4.10e-007f,
		4.15e-007f,
		4.20e-007f,
		4.25e-007f,
		4.30e-007f,
		4.35e-007f,
		4.40e-007f,
		4.45e-007f,
		4.50e-007f,
		4.55e-007f,
		4.60e-007f,
		4.65e-007f,
		4.70e-007f,
		4.75e-007f,
		4.80e-007f,
		4.85e-007f,
		4.90e-007f,
		4.95e-007f,
		5.00e-007f,
		5.05e-007f,
		5.10e-007f,
		5.15e-007f,
		5.20e-007f,
		5.25e-007f,
		5.30e-007f,
		5.35e-007f,
		5.40e-007f,
		5.45e-007f,
		5.50e-007f,
		5.55e-007f,
		5.60e-007f,
		5.65e-007f,
		5.70e-007f,
		5.75e-007f,
		5.80e-007f,
		5.85e-007f,
		5.90e-007f,
		5.95e-007f,
		6.00e-007f,
		6.05e-007f,
		6.10e-007f,
		6.15e-007f,
		6.20e-007f,
		6.25e-007f,
		6.30e-007f,
		6.35e-007f,
		6.40e-007f,
		6.45e-007f,
		6.50e-007f,
		6.55e-007f,
		6.60e-007f,
		6.65e-007f,
		6.70e-007f,
		6.75e-007f,
		6.80e-007f,
		6.85e-007f,
		6.90e-007f,
		6.95e-007f,
		7.00e-007f,
		7.05e-007f,
		7.10e-007f,
		7.15e-007f,
		7.20e-007f,
		7.25e-007f,
		7.30e-007f,
		7.35e-007f,
		7.40e-007f,
		7.45e-007f,
		7.50e-007f,
		7.55e-007f,
		7.60e-007f,
		7.65e-007f,
		7.70e-007f,
		7.75e-007f,
		7.80e-007f,
	};
	const TVector3D xyz[] =
	{
		TVector3D(0.0000e000f, 0.0000e000f, 0.0000e000f),
		TVector3D(0.0000e000f, 0.0000e000f, 0.0000e000f),
		TVector3D(1.7146e003f, 0.0000e000f, 5.9997e003f),
		TVector3D(6.0010e003f, 8.5718e002f, 2.4856e004f),
		TVector3D(2.0575e004f, 2.5715e003f, 8.9996e004f),
		TVector3D(6.1725e004f, 6.8574e003f, 2.7684e005f),
		TVector3D(1.6374e005f, 1.7144e004f, 7.3711e005f),
		TVector3D(3.7206e005f, 3.8573e004f, 1.6894e006f),
		TVector3D(7.2612e005f, 7.5432e004f, 3.3376e006f),
		TVector3D(1.2053e006f, 1.2429e005f, 5.6295e006f),
		TVector3D(1.7532e006f, 1.8344e005f, 8.3353e006f),
		TVector3D(2.2692e006f, 2.5287e005f, 1.0992e007f),
		TVector3D(2.6979e006f, 3.3173e005f, 1.3315e007f),
		TVector3D(3.0665e006f, 4.2516e005f, 1.5415e007f),
		TVector3D(3.2894e006f, 5.3231e005f, 1.6862e007f),
		TVector3D(3.3151e006f, 6.4031e005f, 1.7376e007f),
		TVector3D(3.1780e006f, 7.6717e005f, 1.7098e007f),
		TVector3D(2.9405e006f, 9.1118e005f, 1.6291e007f),
		TVector3D(2.5916e006f, 1.0989e006f, 1.4960e007f),
		TVector3D(2.1784e006f, 1.3098e006f, 1.3327e007f),
		TVector3D(1.6769e006f, 1.5875e006f, 1.1293e007f),
		TVector3D(1.1342e006f, 1.8849e006f, 8.8299e006f),
		TVector3D(6.9012e005f, 2.1738e006f, 6.6177e006f),
		TVector3D(3.5235e005f, 2.5518e006f, 4.8863e006f),
		TVector3D(1.3888e005f, 2.9067e006f, 3.5596e006f),
		TVector3D(4.3722e004f, 3.3893e006f, 2.5919e006f),
		TVector3D(3.2577e004f, 3.9499e006f, 1.8728e006f),
		TVector3D(1.3202e005f, 4.5550e006f, 1.3645e006f),
		TVector3D(3.2148e005f, 5.2005e006f, 9.5996e005f),
		TVector3D(6.1210e005f, 5.8777e006f, 7.0454e005f),
		TVector3D(1.0090e006f, 6.5300e006f, 5.2026e005f),
		TVector3D(1.4831e006f, 7.0571e006f, 3.6941e005f),
		TVector3D(2.0275e006f, 7.5020e006f, 2.6142e005f),
		TVector3D(2.6079e006f, 7.9186e006f, 1.7656e005f),
		TVector3D(3.2303e006f, 8.2460e006f, 1.1742e005f),
		TVector3D(3.8715e006f, 8.4192e006f, 6.7711e004f),
		TVector3D(4.5419e006f, 8.5015e006f, 3.4284e004f),
		TVector3D(5.2817e006f, 8.5641e006f, 9.4281e003f),
		TVector3D(6.0456e006f, 8.5486e006f, 0.0000e000f),
		TVector3D(6.8051e006f, 8.4209e006f, 0.0000e000f),
		TVector3D(7.5330e006f, 8.1912e006f, 0.0000e000f),
		TVector3D(8.1545e006f, 7.8449e006f, 0.0000e000f),
		TVector3D(8.6946e006f, 7.4480e006f, 0.0000e000f),
		TVector3D(9.2098e006f, 7.0769e006f, 0.0000e000f),
		TVector3D(9.5888e006f, 6.6637e006f, 0.0000e000f),
		TVector3D(9.7242e006f, 6.1751e006f, 0.0000e000f),
		TVector3D(9.6359e006f, 5.6428e006f, 0.0000e000f),
		TVector3D(9.3367e006f, 5.0908e006f, 0.0000e000f),
		TVector3D(8.8343e006f, 4.5259e006f, 0.0000e000f),
		TVector3D(8.1502e006f, 3.9584e006f, 0.0000e000f),
		TVector3D(7.3410e006f, 3.4124e006f, 0.0000e000f),
		TVector3D(6.4717e006f, 2.9110e006f, 0.0000e000f),
		TVector3D(5.5509e006f, 2.4301e006f, 0.0000e000f),
		TVector3D(4.5873e006f, 1.9569e006f, 0.0000e000f),
		TVector3D(3.7001e006f, 1.5412e006f, 0.0000e000f),
		TVector3D(2.9465e006f, 1.2018e006f, 0.0000e000f),
		TVector3D(2.3001e006f, 9.2232e005f, 0.0000e000f),
		TVector3D(1.7514e006f, 6.9603e005f, 0.0000e000f),
		TVector3D(1.3082e006f, 5.1688e005f, 0.0000e000f),
		TVector3D(9.6188e005f, 3.7802e005f, 0.0000e000f),
		TVector3D(6.9697e005f, 2.7258e005f, 0.0000e000f),
		TVector3D(4.9637e005f, 1.9372e005f, 0.0000e000f),
		TVector3D(3.5063e005f, 1.3629e005f, 0.0000e000f),
		TVector3D(2.4518e005f, 9.5147e004f, 0.0000e000f),
		TVector3D(1.7060e005f, 6.6003e004f, 0.0000e000f),
		TVector3D(1.1831e005f, 4.6288e004f, 0.0000e000f),
		TVector3D(8.2300e004f, 3.1716e004f, 0.0000e000f),
		TVector3D(5.6581e004f, 2.2287e004f, 0.0000e000f),
		TVector3D(3.9435e004f, 1.5429e004f, 0.0000e000f),
		TVector3D(2.6576e004f, 1.0286e004f, 0.0000e000f),
		TVector3D(1.8860e004f, 6.8574e003f, 0.0000e000f),
		TVector3D(1.2859e004f, 5.1431e003f, 0.0000e000f),
		TVector3D(8.5729e003f, 3.4287e003f, 0.0000e000f),
		TVector3D(6.0010e003f, 2.5715e003f, 0.0000e000f),
		TVector3D(4.2864e003f, 1.7144e003f, 0.0000e000f),
		TVector3D(3.4291e003f, 8.5718e002f, 0.0000e000f),
		TVector3D(2.5719e003f, 8.5718e002f, 0.0000e000f),
		TVector3D(1.7146e003f, 8.5718e002f, 0.0000e000f),
		TVector3D(8.5729e002f, 0.0000e000f, 0.0000e000f),
		TVector3D(8.5729e002f, 0.0000e000f, 0.0000e000f),
		TVector3D(8.5729e002f, 0.0000e000f, 0.0000e000f),
		TVector3D(0.0000e000f, 0.0000e000f, 0.0000e000f),
		TVector3D(0.0000e000f, 0.0000e000f, 0.0000e000f),
	};

	return TXyzSpline(w, w + sizeof w / sizeof(TScalar), xyz);
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
