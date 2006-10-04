/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2006  Bram de Greve
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

/** @class liar::Spectrum
 *  @brief respresentation of a colour spectrum.
 *  @author Bram de Greve [Bramz]
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
	explicit Spectrum(TParam scalar);
	explicit Spectrum(const TVector3D& xyz);

	Spectrum& operator=(const Spectrum& other);
	Spectrum& operator=(TParam scalar);

	TParam operator[](size_t band) const;
	TReference operator[](size_t band);
	const size_t numberOfBands() const { return numberOfBands_; }
	
	const TValue average() const;
	const TValue total() const;

	const Spectrum& operator+() const;
	const Spectrum operator-() const;
	const Spectrum reciprocal() const;

	Spectrum& operator+=(const Spectrum& other);
	Spectrum& operator-=(const Spectrum& other);
	Spectrum& operator*=(const Spectrum& other);
	Spectrum& operator/=(const Spectrum& other);
	void inppow(const Spectrum& other);
	void inpclamp(const Spectrum& min, const Spectrum& max); 
	
	Spectrum& operator+=(TParam scalar);
	Spectrum& operator-=(TParam scalar);
	Spectrum& operator*=(TParam scalar);
	Spectrum& operator/=(TParam scalar);
	void inppow(TParam scalar);
	void inpclamp(TParam min, TParam max); 
	
	void inpsqr();
	void inpsqrt();

	void swap(Spectrum& other);

	const bool operator!() const;
	operator num::SafeBool() const;

	const bool isScalar() const;
	const TVector3D xyz() const;

private:

	enum { numberOfBands_ = 3 };
	TVector3D xyz_;
};

PY_SHADOW_CLASS(LIAR_KERNEL_DLL, PySpectrum, Spectrum);

inline const Spectrum operator+(const Spectrum& a, const Spectrum& b);
inline const Spectrum operator-(const Spectrum& a, const Spectrum& b);
inline const Spectrum operator*(const Spectrum& a, const Spectrum& b);
inline const Spectrum operator/(const Spectrum& a, const Spectrum& b);
inline const Spectrum pow(const Spectrum& a, const Spectrum& b);
inline const Spectrum clamp(const Spectrum& a, const Spectrum& min, const Spectrum& max);

inline const Spectrum operator+(const Spectrum& a, Spectrum::TParam b);
inline const Spectrum operator-(const Spectrum& a, Spectrum::TParam b);
inline const Spectrum operator*(const Spectrum& a, Spectrum::TParam b);
inline const Spectrum operator/(const Spectrum& a, Spectrum::TParam b);
inline const Spectrum pow(const Spectrum& a, Spectrum::TParam b);
inline const Spectrum clamp(const Spectrum& a, Spectrum::TParam min, Spectrum::TParam max);

inline const Spectrum operator+(Spectrum::TParam a, const Spectrum& b);
inline const Spectrum operator-(Spectrum::TParam a, const Spectrum& b);
inline const Spectrum operator*(Spectrum::TParam a, const Spectrum& b);
inline const Spectrum operator/(Spectrum::TParam a, const Spectrum& b);
inline const Spectrum pow(Spectrum::TParam a, const Spectrum& b);
inline const Spectrum clamp(Spectrum::TParam a, const Spectrum& min, const Spectrum& max);

inline const Spectrum sqr(const Spectrum& a);
inline const Spectrum sqrt(const Spectrum& a);

inline const Spectrum blend(const Spectrum& a, const Spectrum& b, Spectrum::TParam iFactor);

Spectrum xyz(const TVector3D& xyz);
Spectrum xyz(TScalar x, TScalar y, TScalar z);

//Spectrum xyz(const TVector3D& xyz, const TSpectrumFormatPtr& iFormat);
//Spectrum xyz(TScalar x, TScalar y, TScalar z, const TSpectrumFormatPtr& iFormat);

template <typename E, typename T>
std::basic_ostream<E, T>& operator<<(std::basic_ostream<E, T>& stream, const Spectrum& spectrum)
{
	std::basic_ostringstream<E, T> buffer;
	buffer.copyfmt(stream);
	buffer.width(0);
	buffer << "{";
	for (size_t i = 0; i < spectrum.numberOfBands(); ++i)
	{
		if (i != 0)
		{
			buffer << ",";
		}
		buffer << spectrum[i];
	}
	buffer << "}";
	stream << buffer.str();
	return stream;
}

}

}

PY_SHADOW_CASTERS(liar::kernel::PySpectrum);

#include "spectrum.inl"

#endif

// EOF
