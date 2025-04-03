/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2024-2025  Bram de Greve (bramz@users.sourceforge.net)
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

#pragma once

#include "kernel_common.h"

#if LIAR_HAVE_AVX
#	include <array>
#	include <bit>
#	include <immintrin.h>
#endif

namespace liar
{
namespace kernel
{

template <typename T>
class alignas(sizeof(T) * 4) Matrix4x4
{
public:

	using TValue = T;
	using TNumTraits = num::NumTraits<T>;
	using TMatrix = Matrix4x4<T>;
	using TVector = prim::Vector3D<T>;
	using TPoint = prim::Point3D<T>;

	Matrix4x4(): m_{} {};

	Matrix4x4(const TValue* m):
		m_{ m[ 0], m[ 1], m[ 2], m[ 3],
		    m[ 4], m[ 5], m[ 6], m[ 7],
		    m[ 8], m[ 9], m[10], m[11],
		    m[12], m[13], m[14], m[15] }
	{
	}

	Matrix4x4(const TPoint& o, const TVector& x, const TVector& y, const TVector& z):
		m_{ x.x, y.x, z.x, o.x,
		    x.y, y.y, z.y, o.y,
		    x.z, y.z, z.z, o.z,
		    TNumTraits::zero, TNumTraits::zero, TNumTraits::zero, TNumTraits::one }
	{
	}

	Matrix4x4(const TVector& offset):
		m_{ TNumTraits::one, TNumTraits::zero, TNumTraits::zero, offset.x,
		    TNumTraits::zero, TNumTraits::one, TNumTraits::zero, offset.y,
		    TNumTraits::zero, TNumTraits::zero, TNumTraits::one, offset.z,
		    TNumTraits::zero, TNumTraits::zero, TNumTraits::zero, TNumTraits::one }
	{
	}

	TMatrix operator*(const TMatrix& other) const
	{
		TMatrix result;
		for (size_t i = 0; i < 4; ++i)
		{
			for (size_t k = 0; k < 4; ++k)
			{
				for (size_t j = 0; j < 4; ++j)
				{
					result.m_[4 * i + j] += m_[4 * i + k] * other.m_[4 * k + j];
				}
			}
		}
		return result;
	}

	TVector operator*(const TVector& v) const
	{
		return TVector(
			m_[ 0] * v.x + m_[ 1] * v.y + m_[ 2] * v.z,
			m_[ 4] * v.x + m_[ 5] * v.y + m_[ 6] * v.z,
			m_[ 8] * v.x + m_[ 9] * v.y + m_[10] * v.z
		);
	}

	friend TVector operator*(const TVector& n, const TMatrix& m)
	{
		return TVector(
			n.x * m.m_[0] + n.y * m.m_[4] + n.z * m.m_[8],
			n.x * m.m_[1] + n.y * m.m_[5] + n.z * m.m_[9],
			n.x * m.m_[2] + n.y * m.m_[6] + n.z * m.m_[10]
		);
	}

	TPoint operator*(const TPoint& p) const
	{
		const TValue invW = num::inv(m_[12] * p.x + m_[13] * p.y + m_[14] * p.z + m_[15]);
		return TPoint(
			invW * (m_[ 0] * p.x + m_[ 1] * p.y + m_[ 2] * p.z + m_[ 3]),
			invW * (m_[ 4] * p.x + m_[ 5] * p.y + m_[ 6] * p.z + m_[ 7]),
			invW * (m_[ 8] * p.x + m_[ 9] * p.y + m_[10] * p.z + m_[11])
		);
	}

	TMatrix inverted() const
	{
		TMatrix result;

		const TValue v1015 = m_[10] * m_[15];
		const TValue v1411 = m_[14] * m_[11];
		const TValue v0615 = m_[ 6] * m_[15];
		const TValue v1407 = m_[14] * m_[ 7];
		const TValue v0611 = m_[ 6] * m_[11];
		const TValue v1007 = m_[10] * m_[ 7];
		const TValue v0215 = m_[ 2] * m_[15];
		const TValue v1403 = m_[14] * m_[ 3];
		const TValue v0211 = m_[ 2] * m_[11];
		const TValue v1003 = m_[10] * m_[ 3];
		const TValue v0207 = m_[ 2] * m_[ 7];
		const TValue v0603 = m_[ 6] * m_[ 3];

		result.m_[0] = v1015 * m_[ 5] + v1407 * m_[ 9] + v0611 * m_[13]
		             - v1411 * m_[ 5] - v0615 * m_[ 9] - v1007 * m_[13];
		result.m_[1] = v1411 * m_[ 1] + v0215 * m_[ 9] + v1003 * m_[13]
		             - v1015 * m_[ 1] - v1403 * m_[ 9] - v0211 * m_[13];
		result.m_[2] = v0615 * m_[ 1] + v1403 * m_[ 5] + v0207 * m_[13]
		             - v1407 * m_[ 1] - v0215 * m_[ 5] - v0603 * m_[13];
		result.m_[3] = v1007 * m_[ 1] + v0211 * m_[ 5] + v0603 * m_[ 9]
		             - v0611 * m_[ 1] - v1003 * m_[ 5] - v0207 * m_[ 9];
		result.m_[4] = v1411 * m_[ 4] + v0615 * m_[ 8] + v1007 * m_[12]
		             - v1015 * m_[ 4] - v1407 * m_[ 8] - v0611 * m_[12];
		result.m_[5] = v1015 * m_[ 0] + v1403 * m_[ 8] + v0211 * m_[12]
		             - v1411 * m_[ 0] - v0215 * m_[ 8] - v1003 * m_[12];
		result.m_[6] = v1407 * m_[ 0] + v0215 * m_[ 4] + v0603 * m_[12]
		             - v0615 * m_[ 0] - v1403 * m_[ 4] - v0207 * m_[12];
		result.m_[7] = v0611 * m_[ 0] + v1003 * m_[ 4] + v0207 * m_[ 8]
		             - v1007 * m_[ 0] - v0211 * m_[ 4] - v0603 * m_[ 8];

		const TValue v0813 = m_[ 8] * m_[13];
		const TValue v1209 = m_[12] * m_[ 9];
		const TValue v0413 = m_[ 4] * m_[13];
		const TValue v1205 = m_[12] * m_[ 5];
		const TValue v0409 = m_[ 4] * m_[ 9];
		const TValue v0805 = m_[ 8] * m_[ 5];
		const TValue v0013 = m_[ 0] * m_[13];
		const TValue v1201 = m_[12] * m_[ 1];
		const TValue v0009 = m_[ 0] * m_[ 9];
		const TValue v0801 = m_[ 8] * m_[ 1];
		const TValue v0005 = m_[ 0] * m_[ 5];
		const TValue v0401 = m_[ 4] * m_[ 1];

		result.m_[ 8] = v0813 * m_[ 7] + v1205 * m_[11] + v0409 * m_[15]
		              - v1209 * m_[ 7] - v0413 * m_[11] - v0805 * m_[15];
		result.m_[ 9] = v1209 * m_[ 3] + v0013 * m_[11] + v0801 * m_[15]
		              - v0813 * m_[ 3] - v1201 * m_[11] - v0009 * m_[15];
		result.m_[10] = v0413 * m_[ 3] + v1201 * m_[ 7] + v0005 * m_[15]
		              - v1205 * m_[ 3] - v0013 * m_[ 7] - v0401 * m_[15];
		result.m_[11] = v0805 * m_[ 3] + v0009 * m_[ 7] + v0401 * m_[11]
		              - v0409 * m_[ 3] - v0801 * m_[ 7] - v0005 * m_[11];
		result.m_[12] = v0413 * m_[10] + v0805 * m_[14] + v1209 * m_[ 6]
		              - v0409 * m_[14] - v0813 * m_[ 6] - v1205 * m_[10];
		result.m_[13] = v0009 * m_[14] + v0813 * m_[ 2] + v1201 * m_[10]
		              - v0013 * m_[10] - v0801 * m_[14] - v1209 * m_[ 2];
		result.m_[14] = v0013 * m_[ 6] + v0401 * m_[14] + v1205 * m_[ 2]
		              - v0005 * m_[14] - v0413 * m_[ 2] - v1201 * m_[ 6];
		result.m_[15] = v0005 * m_[10] + v0409 * m_[ 2] + v0801 * m_[ 6]
		              - v0009 * m_[ 6] - v0401 * m_[10] - v0805 * m_[ 2];

		const TValue det = m_[0] * result.m_[0] + m_[4] * result.m_[1] + m_[8] * result.m_[2] + m_[12] * result.m_[3];
		if (det == TNumTraits::zero)
		{
			LASS_THROW_EX(util::SingularityError, "transformation not invertible");
		}
		const TValue invDet = num::inv(det);
		for (size_t i = 0; i < 16; ++i)
		{
			result.m_[i] *= invDet;
		}
		return result;
	}

	static TMatrix identity()
	{
		TMatrix result;
		result.m_[ 0] = TNumTraits::one;
		result.m_[ 5] = TNumTraits::one;
		result.m_[10] = TNumTraits::one;
		result.m_[15] = TNumTraits::one;
		return result;
	}

private:
	TValue m_[16];
};


#if LIAR_HAVE_AVX

#pragma push_macro("shuffler_ps")
#pragma push_macro("swizzler_ps")
#pragma push_macro("swizzle1_ps")

/** reverse shuffle: (v1[x], v1[y], v2[z], v2[w])
 */
#define shuffler_ps(v1, v2, x, y, z, w) _mm_shuffle_ps(v1, v2, _MM_SHUFFLE(w, z, y, x))

/** reverse swizzle: (v[x], v[y], v[z], v[w])
 */
#define swizzler_ps(v, x, y, z, w) _mm_shuffle_ps(v, v, _MM_SHUFFLE(w, z, y, x))

 /** swizzle one value: (v[x], v[x], v[x], v[x])
  */
#define swizzle1_ps(v, x) _mm_shuffle_ps(v, v, _MM_SHUFFLE(x, x, x, x))


namespace impl
{

LIAR_FORCE_INLINE __m128 inv(__m128 x)
{
	__m128 res = _mm_rcp_ps(x);
	__m128 muls = _mm_mul_ps(x, _mm_mul_ps(res, res));
	return _mm_sub_ps(_mm_add_ps(res, res), muls);
}

/** Multiply two 2x2 matrices.
 *  @param a 2x2 matrix: a00 a01 a10 a11.
 *  @param b 2x2 matrix: b00 b01 b10 b11.
 *  @return 2x2 matrix: c00 c01 c10 c11 = a * b.
 *
 *  a * b = [a00*b00+a01*b10  a01*b11+a00*b01]
 *          [a10*b00+a11*b10  a11*b11+a10*b01]
 */
LIAR_FORCE_INLINE __m128 mat2Mul(__m128 a, __m128 b)
{
	return _mm_add_ps(
		_mm_mul_ps(a,                                      swizzler_ps(b, 0b00, 0b11, 0b00, 0b11)),
		_mm_mul_ps(swizzler_ps(a, 0b01, 0b00, 0b11, 0b10), swizzler_ps(b, 0b10, 0b01, 0b10, 0b01)));
}


/** Multiple the adjugate of a 2x2 matrix with another 2x2 matrix.
 *  @param a 2x2 matrix: a00 a01 a10 a11.
 *  @param b 2x2 matrix: b00 b01 b10 b11.
 *  @return 2x2 matrix: c00 c01 c10 c11 = adj(a) * b.
 *
 *  adj(a) = [ a11  -a01]
 *           [-a10   a00]
 *
 *  adj(a) * b = [a11*b00-a01*b10  a11*b01-a01*b11]
 *               [a00*b10-a10*b00  a00*b11-a10*b01]
 */
LIAR_FORCE_INLINE __m128 mat2AdjMul(__m128 a, __m128 b)
{
	return _mm_sub_ps(
		_mm_mul_ps(swizzler_ps(a, 0b11, 0b11, 0b00, 0b00), b),
		_mm_mul_ps(swizzler_ps(a, 0b01, 0b01, 0b10, 0b10), swizzler_ps(b, 0b10, 0b11, 0b00, 0b01)));
}


/** Multiply a 2x2 matrix with the adjugate of another 2x2 matrix.
 *  @param a 2x2 matrix: a00 a01 a10 a11.
 *  @param b 2x2 matrix: b00 b01 b10 b11.
 *  @return 2x2 matrix: c00 c01 c10 c11 = a * adj(b).
 *
 *  adj(b) = [ b11  -b01]
 *           [-b10   b00]
 *
 *  a * adj(b) = [a00*b11-a01*b10  a01*b00-a00*b01]
 *               [a10*b11-a11*b10  a11*b00-a10*b01]
 */
LIAR_FORCE_INLINE __m128 mat2mulAdj(__m128 a, __m128 b)
{
	return _mm_sub_ps(
		_mm_mul_ps(a,                                      swizzler_ps(b, 0b11, 0b00, 0b11, 0b00)),
		_mm_mul_ps(swizzler_ps(a, 0b01, 0b00, 0b11, 0b10), swizzler_ps(b, 0b10, 0b01, 0b10, 0b01)));
}


#define LIAR_USE_DPPS 1

LIAR_FORCE_INLINE __m128 dot(__m128 a, __m128 b)
{
#if LIAR_USE_DPPS
	return _mm_dp_ps(a, b, 0xff);
#else
	__m128 r = _mm_mul_ps(a, b);
	r = _mm_hadd_ps(r, r);
	r = _mm_hadd_ps(r, r);
	return r;
#endif
}

}


template <>
class alignas(64) Matrix4x4<float>
{
public:

	using TValue = float;
	using TNumTraits = num::NumTraits<float>;
	using TMatrix = Matrix4x4<float>;
	using TVector = prim::Vector3D<float>;
	using TPoint = prim::Point3D<float>;

	Matrix4x4():
		row_{
			_mm_setzero_ps(),
			_mm_setzero_ps(),
			_mm_setzero_ps(),
			_mm_setzero_ps()}
	{
	}

	Matrix4x4(__m128 r0, __m128 r1, __m128 r2, __m128 r3):
		row_{ r0, r1, r2, r3 }
	{
	}

	Matrix4x4(const TValue* m):
		row_{
			_mm_loadu_ps(m),
			_mm_loadu_ps(m + 4),
			_mm_loadu_ps(m + 8),
			_mm_loadu_ps(m + 12)}
	{
	}

	Matrix4x4(const TPoint& o, const TVector& x, const TVector& y, const TVector& z):
		row_{
			_mm_setr_ps(x.x, y.x, z.x, o.x),
			_mm_setr_ps(x.y, y.y, z.y, o.y),
			_mm_setr_ps(x.z, y.z, z.z, o.z),
			_mm_setr_ps(0.f, 0.f, 0.f, 1.f)}
	{
	}

	Matrix4x4(const TVector& offset) :
		row_{
			_mm_setr_ps(1.f, 0.f, 0.f, offset.x),
			_mm_setr_ps(0.f, 1.f, 0.f, offset.y),
			_mm_setr_ps(0.f, 0.f, 1.f, offset.z),
			_mm_setr_ps(0.f, 0.f, 0.f, 1.f) }
	{
	}

	TMatrix operator*(const TMatrix& other) const
	{
		__m128 r0 = _mm_mul_ps(swizzle1_ps(row_[0], 0), other.row_[0]);
		__m128 r1 = _mm_mul_ps(swizzle1_ps(row_[1], 0), other.row_[0]);
		__m128 r2 = _mm_mul_ps(swizzle1_ps(row_[2], 0), other.row_[0]);
		__m128 r3 = _mm_mul_ps(swizzle1_ps(row_[3], 0), other.row_[0]);

		r0 = _mm_fmadd_ps(swizzle1_ps(row_[0], 1), other.row_[1], r0);
		r1 = _mm_fmadd_ps(swizzle1_ps(row_[1], 1), other.row_[1], r1);
		r2 = _mm_fmadd_ps(swizzle1_ps(row_[2], 1), other.row_[1], r2);
		r3 = _mm_fmadd_ps(swizzle1_ps(row_[3], 1), other.row_[1], r3);

		r0 = _mm_fmadd_ps(swizzle1_ps(row_[0], 2), other.row_[2], r0);
		r1 = _mm_fmadd_ps(swizzle1_ps(row_[1], 2), other.row_[2], r1);
		r2 = _mm_fmadd_ps(swizzle1_ps(row_[2], 2), other.row_[2], r2);
		r3 = _mm_fmadd_ps(swizzle1_ps(row_[3], 2), other.row_[2], r3);

		r0 = _mm_fmadd_ps(swizzle1_ps(row_[0], 3), other.row_[3], r0);
		r1 = _mm_fmadd_ps(swizzle1_ps(row_[1], 3), other.row_[3], r1);
		r2 = _mm_fmadd_ps(swizzle1_ps(row_[2], 3), other.row_[3], r2);
		r3 = _mm_fmadd_ps(swizzle1_ps(row_[3], 3), other.row_[3], r3);

		return TMatrix(r0, r1, r2, r3);
	}

	TVector operator*(const TVector& v) const
	{
		const __m128 vv = _mm_setr_ps(v.x, v.y, v.z, 0.f);
		return TVector(
			_mm_cvtss_f32(impl::dot(row_[0], vv)),
			_mm_cvtss_f32(impl::dot(row_[1], vv)),
			_mm_cvtss_f32(impl::dot(row_[2], vv)));
	}

	friend TVector operator*(const TVector& n, const TMatrix& m)
	{
		__m128 r = _mm_mul_ps(m.row_[0], _mm_set1_ps(n.x));
		r = _mm_fmadd_ps(m.row_[1], _mm_set1_ps(n.y), r);
		r = _mm_fmadd_ps(m.row_[2], _mm_set1_ps(n.z), r);
		static_assert(sizeof(r) == 4 * sizeof(float));
		auto rr = std::bit_cast<std::array<float, 4>>(r);
		return TVector(rr[0], rr[1], rr[2]);
	}

	TPoint operator*(const TPoint& p) const
	{
		const __m128 pp = _mm_setr_ps(p.x, p.y, p.z, 1.f);
		const __m128 w = impl::dot(row_[3], pp);
		const __m128 pp_w = _mm_div_ps(pp, w);
		return TPoint(
			_mm_cvtss_f32(impl::dot(row_[0], pp_w)),
			_mm_cvtss_f32(impl::dot(row_[1], pp_w)),
			_mm_cvtss_f32(impl::dot(row_[2], pp_w)));
	}

	TMatrix transposed() const
	{
		// a0 a1 a2 a3    a0 b0 a1 b1    a0 b0 c0 d0
		// b0 b1 b2 b3 -> a2 b2 a3 b3 -> a1 b1 c1 d1
		// c0 c1 c2 c3    c0 d0 c1 d1    a2 b2 c2 d2
		// d0 d1 d2 d3    c2 d2 c3 d3    a3 b3 c3 d3
		const __m128 r0 = _mm_unpacklo_ps(row_[0], row_[1]);
		const __m128 r1 = _mm_unpackhi_ps(row_[0], row_[1]);
		const __m128 r2 = _mm_unpacklo_ps(row_[2], row_[3]);
		const __m128 r3 = _mm_unpackhi_ps(row_[2], row_[3]);
		return TMatrix(
			_mm_movelh_ps(r0, r2),
			_mm_movehl_ps(r2, r0),
			_mm_movelh_ps(r1, r3),
			_mm_movehl_ps(r3, r1)
		);
	}

	TMatrix inverted() const
	{
		TMatrix result;

		// a0 a1 b0 b1
		// a2 a3 b2 b3
		// c0 c1 d0 d1
		// c2 c3 d2 d3
		const __m128 A = _mm_movelh_ps(row_[0], row_[1]);
		const __m128 B = _mm_movehl_ps(row_[1], row_[0]);
		const __m128 C = _mm_movelh_ps(row_[2], row_[3]);
		const __m128 D = _mm_movehl_ps(row_[3], row_[2]);

		const __m128 abcd0 = shuffler_ps(row_[0], row_[2], 0, 2, 0, 2);
		const __m128 abcd1 = shuffler_ps(row_[0], row_[2], 1, 3, 1, 3);
		const __m128 abcd2 = shuffler_ps(row_[1], row_[3], 0, 2, 0, 2);
		const __m128 abcd3 = shuffler_ps(row_[1], row_[3], 1, 3, 1, 3);

		const __m128 detABCD = _mm_sub_ps(_mm_mul_ps(abcd0, abcd3), _mm_mul_ps(abcd1, abcd2));
		const __m128 detA = swizzle1_ps(detABCD, 0);
		const __m128 detB = swizzle1_ps(detABCD, 1);
		const __m128 detC = swizzle1_ps(detABCD, 2);
		const __m128 detD = swizzle1_ps(detABCD, 3);

		const __m128 A_B = impl::mat2AdjMul(A, B); // adj(A) * B
		const __m128 D_C = impl::mat2AdjMul(D, C); // adj(D) * C

		// X_ = det(D) * A - B * (adj(D) * C)
		__m128 X_ = _mm_sub_ps(_mm_mul_ps(detD, A), impl::mat2Mul(B, D_C));
		// W_ = det(A) * D - C * (adj(A) * B)
		__m128 W_ = _mm_sub_ps(_mm_mul_ps(detA, D), impl::mat2Mul(C, A_B));
		// Y_ = det(B) * C - D * adj(adj(A) * B)
		__m128 Y_ = _mm_sub_ps(_mm_mul_ps(detB, C), impl::mat2mulAdj(D, A_B));
		// Z_ = det(C) * B - A * adj(adj(C) * D)
		__m128 Z_ = _mm_sub_ps(_mm_mul_ps(detC, B), impl::mat2mulAdj(A, D_C));

		__m128 tr = impl::dot(A_B, swizzler_ps(D_C, 0, 2, 1, 3));

		__m128 detM = _mm_mul_ps(detA, detD);
		detM = _mm_fmadd_ps(detB, detC, detM);
		detM = _mm_sub_ps(detM, tr);

		const __m128 adjSign = _mm_setr_ps(1.f, -1.f, -1.f, 1.f);
		const __m128 invDetM = _mm_div_ps(adjSign, detM);

		X_ = _mm_mul_ps(X_, invDetM);
		Y_ = _mm_mul_ps(Y_, invDetM);
		Z_ = _mm_mul_ps(Z_, invDetM);
		W_ = _mm_mul_ps(W_, invDetM);

		return TMatrix(
			shuffler_ps(X_, Y_, 0b11, 0b01, 0b11, 0b01),
			shuffler_ps(X_, Y_, 0b10, 0b00, 0b10, 0b00),
			shuffler_ps(Z_, W_, 0b11, 0b01, 0b11, 0b01),
			shuffler_ps(Z_, W_, 0b10, 0b00, 0b10, 0b00)
			);
	}

	static TMatrix identity()
	{
		static TMatrix result(
			_mm_setr_ps(1.f, 0.f, 0.f, 0.f),
			_mm_setr_ps(0.f, 1.f, 0.f, 0.f),
			_mm_setr_ps(0.f, 0.f, 1.f, 0.f),
			_mm_setr_ps(0.f, 0.f, 0.f, 1.f)
		);
		return result;
	}

private:
	__m128 row_[4];
};

#pragma pop_macro("shuffler_ps")
#pragma pop_macro("swizzler_ps")
#pragma pop_macro("swizzle1_ps")

#endif

/** @relates Matrix4x4
 *	Transform a TRay3D and renormalize.
 */
inline TRay3D transform(const TRay3D& ray, const Matrix4x4<TScalar>& transformation)
{
	const TPoint3D support = transformation * ray.support();
	const TVector3D direction = transformation * ray.direction();
	return TRay3D(support, direction);
}


/** @relates Matrix4x4
 *	Transform a TRay3D, renormalize and adjust @a parameter.
 *
 *	The scalar bounds and @a parameter must be adjusted because of the renormalization, so
 *	that @a parameter refers to the correct point on the transformed ray:
 *
 *	@code
 *	TScalar newParameter = parameter;
 *	TRay3D newRay = transform(ray, transformation, newParameter);
 *	LASS_ASSERT(newRay.point(newParameter) == transformation * ray.point(parameter);
 *	@endcode.
 */
inline TRay3D transform(const TRay3D& ray, const Matrix4x4<TScalar>& transformation, TScalar& parameter)
{
	const TPoint3D support = transformation * ray.support();
	const TVector3D direction = transformation * ray.direction();
	const TScalar norm = direction.norm();
	parameter *= norm;
	return TRay3D(support, direction / norm, prim::IsAlreadyNormalized());
}

}
}
