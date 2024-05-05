/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2024  Bram de Greve (bramz@users.sourceforge.net)
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


}
}
