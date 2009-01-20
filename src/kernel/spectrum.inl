/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
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



inline Spectrum::Spectrum(const TVector3D& xyz):
	xyz_(xyz)
{
}



inline Spectrum::Spectrum(TParam scalar):
	xyz_(scalar, scalar, scalar)
{
}



inline Spectrum& Spectrum::operator=(const Spectrum& other)
{
	xyz_ = other.xyz_;
	return *this;
}



inline Spectrum& Spectrum::operator=(TParam scalar)
{
	xyz_ = TVector3D(scalar, scalar, scalar);
	return *this;
}



inline Spectrum::TParam Spectrum::operator[](size_t band) const 
{ 
	LASS_ASSERT(band < numberOfBands_); 
	return xyz_[band]; 
}



inline Spectrum::TReference Spectrum::operator[](size_t band)
{ 
	LASS_ASSERT(band < numberOfBands_); 
	return xyz_[band]; 
}



inline const Spectrum::TValue Spectrum::luminance() const
{
	return xyz_.y;
}



inline const Spectrum::TValue Spectrum::average() const
{
	return total() / numberOfBands_;
}



inline const Spectrum::TValue Spectrum::total() const
{
	return xyz_.x + xyz_.y + xyz_.z;
}



inline const Spectrum::TValue Spectrum::absAverage() const
{
	return absTotal() / numberOfBands_;
}



inline const Spectrum::TValue Spectrum::absTotal() const
{
	return num::abs(xyz_.x) + num::abs(xyz_.y) + num::abs(xyz_.z);
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



inline Spectrum& Spectrum::operator+=(TParam scalar)
{
	xyz_ += scalar;
	return *this;
}



inline Spectrum& Spectrum::operator-=(TParam scalar)
{
	xyz_ -= scalar;
	return *this;
}



inline Spectrum& Spectrum::operator*=(TParam scalar)
{
	xyz_ *= scalar;
	return *this;
}



inline Spectrum& Spectrum::operator/=(TParam scalar)
{
	xyz_ /= scalar;
	return *this;
}



inline void Spectrum::inppow(TParam scalar)
{
	num::inppow(xyz_.x, scalar);
	num::inppow(xyz_.y, scalar);
	num::inppow(xyz_.z, scalar);
}



inline void Spectrum::inpclamp(TParam min, TParam max)
{
	num::inpclamp(xyz_.x, min, max);
	num::inpclamp(xyz_.y, min, max);
	num::inpclamp(xyz_.z, min, max);
}



inline Spectrum& Spectrum::operator+=(const Spectrum& other)
{
	xyz_ += other.xyz_;
	return *this;
}



inline Spectrum& Spectrum::operator-=(const Spectrum& other)
{
	xyz_ -= other.xyz_;
	return *this;
}



inline Spectrum& Spectrum::operator*=(const Spectrum& other)
{
	xyz_ *= other.xyz_;
	return *this;
}



inline Spectrum& Spectrum::operator/=(const Spectrum& other)
{
	xyz_ /= other.xyz_;
	return *this;
}



inline void Spectrum::inppow(const Spectrum& other)
{
	num::inppow(xyz_.x, other.xyz_.x);
	num::inppow(xyz_.y, other.xyz_.y);
	num::inppow(xyz_.z, other.xyz_.z);
}



inline void Spectrum::inpclamp(const Spectrum& min, const Spectrum& max)
{
	num::inpclamp(xyz_.x, min.xyz_.x, max.xyz_.x);
	num::inpclamp(xyz_.y, min.xyz_.y, max.xyz_.y);
	num::inpclamp(xyz_.z, min.xyz_.z, max.xyz_.z);
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



inline void Spectrum::inpexp()
{
	num::inpexp(xyz_.x);
	num::inpexp(xyz_.y);
	num::inpexp(xyz_.z);
}



inline void Spectrum::inplog()
{
	num::inplog(xyz_.x);
	num::inplog(xyz_.y);
	num::inplog(xyz_.z);
}



inline void Spectrum::swap(Spectrum& other)
{
	std::swap(xyz_, other.xyz_);
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
inline const Spectrum operator+(const Spectrum& a, const Spectrum& b)
{
	Spectrum result(a);
	result += b;
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum operator-(const Spectrum& a, const Spectrum& b)
{
	Spectrum result(a);
	result -= b;
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum operator*(const Spectrum& a, const Spectrum& b)
{
	Spectrum result(a);
	result *= b;
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum operator/(const Spectrum& a, const Spectrum& b)
{
	Spectrum result(a);
	result /= b;
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum pow(const Spectrum& a, const Spectrum& b)
{
	Spectrum result(a);
	result.inppow(b);
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum clamp(const Spectrum& a, const Spectrum& min, const Spectrum& max)
{
	Spectrum result(a);
	result.inpclamp(min, max);
	return result;
}




/** @relates liar::Spectrum
 */
inline const Spectrum operator+(const Spectrum& a, Spectrum::TParam b)
{
	Spectrum result(a);
	result += b;
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum operator-(const Spectrum& a, Spectrum::TParam b)
{
	Spectrum result(a);
	result -= b;
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum operator*(const Spectrum& a, Spectrum::TParam b)
{
	Spectrum result(a);
	result *= b;
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum operator/(const Spectrum& a, Spectrum::TParam b)
{
	Spectrum result(a);
	result /= b;
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum pow(const Spectrum& a, Spectrum::TParam b)
{
	Spectrum result(a);
	result.inppow(b);
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum clamp(const Spectrum& a, Spectrum::TParam min, Spectrum::TParam max)
{
	Spectrum result(a);
	result.inpclamp(min, max);
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum operator+(Spectrum::TParam a, const Spectrum& b)
{
	Spectrum result(b);
	result += a;
	return result;
}




/** @relates liar::Spectrum
 */
inline const Spectrum operator-(Spectrum::TParam a, const Spectrum& b)
{
	Spectrum result(-b);
	result += a;
	return result;
}




/** @relates liar::Spectrum
 */
inline const Spectrum operator*(Spectrum::TParam a, const Spectrum& b)
{
	Spectrum result(b);
	result *= a;
	return result;
}




/** @relates liar::Spectrum
 */
inline const Spectrum operator/(Spectrum::TParam a, const Spectrum& b)
{
	Spectrum result(b.reciprocal());
	result *= a;
	return result;
}




/** @relates liar::Spectrum
 */
inline const Spectrum pow(Spectrum::TParam a, const Spectrum& b)
{
	Spectrum result(a);
	result.inppow(b);
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum clamp(Spectrum::TParam a, const Spectrum& min, const Spectrum& max)
{
	Spectrum result(a);
	result.inpclamp(min, max);
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum sqr(const Spectrum& a)
{
	Spectrum result(a);
	result.inpsqr();
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum sqrt(const Spectrum& a)
{
	Spectrum result(a);
	result.inpsqrt();
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum exp(const Spectrum& a)
{
	Spectrum result(a);
	result.inpexp();
	return result;
}



/** @relates liar::Spectrum
 */
inline const Spectrum log(const Spectrum& a)
{
	Spectrum result(a);
	result.inplog();
	return result;
}



/** @relates liar::Spectrum
 *  linear interpolation of two spectra.
 *  @return @a a * ( 1 - @a t ) + @a b * @a t
 */
inline const Spectrum lerp(const Spectrum& a, const Spectrum& b, Spectrum::TParam t)
{
	Spectrum result(b);
	result -= a;
	result *= t;
	result += a;
	return result;
}

}

}

#endif

// EOF
