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

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SPECTRUM_INL
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SPECTRUM_INL

#include "kernel_common.h"
#include "spectrum.h"

namespace liar
{
namespace kernel
{

// --- public --------------------------------------------------------------------------------------

inline Spectrum::Spectrum():
	xyz_()
{
}



inline Spectrum::Spectrum(const TVector3D& iXYZ):
	xyz_(iXYZ)
{
}



inline Spectrum::Spectrum(TParam iScalar):
	xyz_(iScalar, iScalar, iScalar)
{
}



inline Spectrum& Spectrum::operator=(const Spectrum& iOther)
{
	xyz_ = iOther.xyz_;
	return *this;
}



inline Spectrum& Spectrum::operator=(TParam iScalar)
{
	xyz_ = TVector3D(iScalar, iScalar, iScalar);
	return *this;
}



inline Spectrum::TParam Spectrum::operator[](size_t iBand) const 
{ 
	LASS_ASSERT(iBand < numberOfBands_); 
	return xyz_[iBand]; 
}



inline Spectrum::TReference Spectrum::operator[](size_t iBand)
{ 
	LASS_ASSERT(iBand < numberOfBands_); 
	return xyz_[iBand]; 
}



inline const Spectrum::TValue Spectrum::average() const
{
	return total() / numberOfBands_;
}



inline const Spectrum::TValue Spectrum::total() const
{
	return xyz_.x + xyz_.y + xyz_.z;
}



inline const Spectrum& Spectrum::operator+() const
{
	return *this;
}



inline const Spectrum Spectrum::operator-() const
{
	return Spectrum(-xyz_);
}



inline const Spectrum Spectrum::reciprocal() const
{
	return Spectrum(xyz_.reciprocal());
}



inline Spectrum& Spectrum::operator+=(TParam iScalar)
{
	xyz_ += iScalar;
	return *this;
}



inline Spectrum& Spectrum::operator-=(TParam iScalar)
{
	xyz_ -= iScalar;
	return *this;
}



inline Spectrum& Spectrum::operator*=(TParam iScalar)
{
	xyz_ *= iScalar;
	return *this;
}



inline Spectrum& Spectrum::operator/=(TParam iScalar)
{
	xyz_ /= iScalar;
	return *this;
}



inline void Spectrum::inppow(TParam iScalar)
{
	num::inppow(xyz_.x, iScalar);
	num::inppow(xyz_.y, iScalar);
	num::inppow(xyz_.z, iScalar);
}



inline void Spectrum::inpclamp(TParam iMin, TParam iMax)
{
	num::inpclamp(xyz_.x, iMin, iMax);
	num::inpclamp(xyz_.y, iMin, iMax);
	num::inpclamp(xyz_.z, iMin, iMax);
}



inline Spectrum& Spectrum::operator+=(const Spectrum& iOther)
{
	xyz_ += iOther.xyz_;
	return *this;
}



inline Spectrum& Spectrum::operator-=(const Spectrum& iOther)
{
	xyz_ -= iOther.xyz_;
	return *this;
}



inline Spectrum& Spectrum::operator*=(const Spectrum& iOther)
{
	xyz_ *= iOther.xyz_;
	return *this;
}



inline Spectrum& Spectrum::operator/=(const Spectrum& iOther)
{
	xyz_ /= iOther.xyz_;
	return *this;
}



inline void Spectrum::inppow(const Spectrum& iOther)
{
	num::inppow(xyz_.x, iOther.xyz_.x);
	num::inppow(xyz_.y, iOther.xyz_.y);
	num::inppow(xyz_.z, iOther.xyz_.z);
}



inline void Spectrum::inpclamp(const Spectrum& iMin, const Spectrum& iMax)
{
	num::inpclamp(xyz_.x, iMin.xyz_.x, iMax.xyz_.x);
	num::inpclamp(xyz_.y, iMin.xyz_.y, iMax.xyz_.y);
	num::inpclamp(xyz_.z, iMin.xyz_.z, iMax.xyz_.z);
}



inline void Spectrum::inpsqr()
{
	num::inpsqr(xyz_.x);
	num::inpsqr(xyz_.y);
	num::inpsqr(xyz_.z);
}



inline void Spectrum::inpsqrt()
{
	num::inpsqrt(xyz_.x);
	num::inpsqrt(xyz_.y);
	num::inpsqrt(xyz_.z);
}



inline void Spectrum::swap(Spectrum& iOther)
{
	std::swap(xyz_, iOther.xyz_);
}



inline const bool Spectrum::operator!() const
{
	return xyz_.isZero();
}



inline Spectrum::operator num::SafeBool() const
{
	return xyz_.isZero() ? num::safeFalse : num::safeTrue;
}



inline const TVector3D Spectrum::xyz() const
{
	return xyz_;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------



// --- free ----------------------------------------------------------------------------------------

/** @relates liar::Spectrum
 */
inline const Spectrum operator+(const Spectrum& iA, const Spectrum& iB)
{
	Spectrum result(iA);
	result += iB;
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum operator-(const Spectrum& iA, const Spectrum& iB)
{
	Spectrum result(iA);
	result -= iB;
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum operator*(const Spectrum& iA, const Spectrum& iB)
{
	Spectrum result(iA);
	result *= iB;
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum operator/(const Spectrum& iA, const Spectrum& iB)
{
	Spectrum result(iA);
	result /= iB;
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum pow(const Spectrum& iA, const Spectrum& iB)
{
	Spectrum result(iA);
	result.inppow(iB);
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum clamp(const Spectrum& iA, const Spectrum& iMin, const Spectrum& iMax)
{
	Spectrum result(iA);
	result.inpclamp(iMin, iMax);
	return result;
}




/** @relates liar::Spectrum
 */
inline const Spectrum operator+(const Spectrum& iA, Spectrum::TParam iB)
{
	Spectrum result(iA);
	result += iB;
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum operator-(const Spectrum& iA, Spectrum::TParam iB)
{
	Spectrum result(iA);
	result -= iB;
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum operator*(const Spectrum& iA, Spectrum::TParam iB)
{
	Spectrum result(iA);
	result *= iB;
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum operator/(const Spectrum& iA, Spectrum::TParam iB)
{
	Spectrum result(iA);
	result /= iB;
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum pow(const Spectrum& iA, Spectrum::TParam iB)
{
	Spectrum result(iA);
	result.inppow(iB);
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum clamp(const Spectrum& iA, Spectrum::TParam iMin, Spectrum::TParam iMax)
{
	Spectrum result(iA);
	result.inpclamp(iMin, iMax);
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum operator+(Spectrum::TParam iA, const Spectrum& iB)
{
	Spectrum result(iB);
	result += iA;
	return result;
}




/** @relates liar::Spectrum
 */
inline const Spectrum operator-(Spectrum::TParam iA, const Spectrum& iB)
{
	Spectrum result(-iB);
	result += iA;
	return result;
}




/** @relates liar::Spectrum
 */
inline const Spectrum operator*(Spectrum::TParam iA, const Spectrum& iB)
{
	Spectrum result(iB);
	result *= iA;
	return result;
}




/** @relates liar::Spectrum
 */
inline const Spectrum operator/(Spectrum::TParam iA, const Spectrum& iB)
{
	Spectrum result(iB.reciprocal());
	result *= iA;
	return result;
}




/** @relates liar::Spectrum
 */
inline const Spectrum pow(Spectrum::TParam iA, const Spectrum& iB)
{
	Spectrum result(iA);
	result.inppow(iB);
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum clamp(Spectrum::TParam iA, const Spectrum& iMin, const Spectrum& iMax)
{
	Spectrum result(iA);
	result.inpclamp(iMin, iMax);
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum sqr(const Spectrum& iA)
{
	Spectrum result(iA);
	result.inpsqr();
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum sqrt(const Spectrum& iA)
{
	Spectrum result(iA);
	result.inpsqrt();
	return result;
}



/** @relates liar::Spectrum
 *  Blend between to spectra by a factor.
 *  @return @a iA * ( 1 - @a iFactor ) + @a iB * @a iFactor
 */
inline const Spectrum blend(const Spectrum& iA, const Spectrum& iB, Spectrum::TParam iFactor)
{
	Spectrum result(iA);
	result *= (TNumTraits::one - iFactor);
	result += iFactor * iB;
	return result;
}

}

}

#endif

// EOF