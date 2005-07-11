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

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SPECTRUM_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SPECTRUM_H

#include "kernel_common.h"

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

	Spectrum();
	explicit Spectrum(TParam iScalar);
	explicit Spectrum(const TVector3D& iXYZ);

	Spectrum& operator=(const Spectrum& iOther);
	Spectrum& operator=(TParam iScalar);

	TParam operator[](size_t iBand) const;
	TReference operator[](size_t iBand);
	const size_t numberOfBands() const { return numberOfBands_; }
	
	const TValue averagePower() const;
	const TValue totalPower() const;

	const Spectrum& operator+() const;
	const Spectrum operator-() const;
	const Spectrum reciprocal() const;
	
	Spectrum& operator+=(TParam iScalar);
	Spectrum& operator-=(TParam iScalar);
	Spectrum& operator*=(TParam iScalar);
	Spectrum& operator/=(TParam iScalar);

	Spectrum& operator+=(const Spectrum& iOther);
	Spectrum& operator-=(const Spectrum& iOther);
	Spectrum& operator*=(const Spectrum& iOther);
	Spectrum& operator/=(const Spectrum& iOther);

	Spectrum& pow(TParam iScalar);
	Spectrum& pow(const Spectrum& iOther);

	void swap(Spectrum& iOther);

	const bool isScalar() const;

	const TVector3D xyz() const;

private:

	enum { numberOfBands_ = 3 };
	TVector3D xyz_;
};

PY_SHADOW_CLASS(LIAR_KERNEL_DLL, PySpectrum, Spectrum);

inline const Spectrum operator+(Spectrum::TParam iA, const Spectrum& iB);
inline const Spectrum operator-(Spectrum::TParam iA, const Spectrum& iB);
inline const Spectrum operator*(Spectrum::TParam iA, const Spectrum& iB);
inline const Spectrum operator/(Spectrum::TParam iA, const Spectrum& iB);
inline const Spectrum pow(Spectrum::TParam iA, const Spectrum& iB);

inline const Spectrum operator+(const Spectrum& iA, Spectrum::TParam iB);
inline const Spectrum operator-(const Spectrum& iA, Spectrum::TParam iB);
inline const Spectrum operator*(const Spectrum& iA, Spectrum::TParam iB);
inline const Spectrum operator/(const Spectrum& iA, Spectrum::TParam iB);
inline const Spectrum pow(const Spectrum& iA, Spectrum::TParam iB);

inline const Spectrum operator+(const Spectrum& iA, const Spectrum& iB);
inline const Spectrum operator-(const Spectrum& iA, const Spectrum& iB);
inline const Spectrum operator*(const Spectrum& iA, const Spectrum& iB);
inline const Spectrum operator/(const Spectrum& iA, const Spectrum& iB);
inline const Spectrum pow(const Spectrum& iA, const Spectrum& iB);

Spectrum xyz(const TVector3D& iXYZ);
Spectrum xyz(TScalar iX, TScalar iY, TScalar iZ);

//Spectrum xyz(const TVector3D& iXYZ, const TSpectrumFormatPtr& iFormat);
//Spectrum xyz(TScalar iX, TScalar iY, TScalar iZ, const TSpectrumFormatPtr& iFormat);

}

}

PY_SHADOW_CASTERS(liar::kernel::PySpectrum);

#include "spectrum.inl"

#endif

// EOF
