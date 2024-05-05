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

#include <gtest/gtest.h>

#include "kernel/matrix_4x4.h"

using TMatrixF = liar::kernel::Matrix4x4<float>;
using TVectorF = lass::prim::Vector3D<float>;
using TPointF = lass::prim::Point3D<float>;

namespace
{
	void isIdentity(const TMatrixF& matrix)
	{
		const auto x = matrix * TPointF(1.f, 0.f, 0.f);
		const auto y = matrix * TPointF(0.f, 1.f, 0.f);
		const auto z = matrix * TPointF(0.f, 0.f, 1.f);
		EXPECT_NEAR(x.x, 1.f, 1e-6f);
		EXPECT_NEAR(x.y, 0.f, 1e-6f);
		EXPECT_NEAR(x.z, 0.f, 1e-6f);
		EXPECT_NEAR(y.x, 0.f, 1e-6f);
		EXPECT_NEAR(y.y, 1.f, 1e-6f);
		EXPECT_NEAR(y.z, 0.f, 1e-6f);
		EXPECT_NEAR(z.x, 0.f, 1e-6f);
		EXPECT_NEAR(z.y, 0.f, 1e-6f);
		EXPECT_NEAR(z.z, 1.f, 1e-6f);
	}
}


TEST(Matrix4x4, MulVector)
{
	TVectorF vector(1.f, 2.f, 3.f);
	{
		const TMatrixF matrix = TMatrixF::identity();
		const TVectorF result = matrix * vector;
		EXPECT_FLOAT_EQ(result.x, 1.f);
		EXPECT_FLOAT_EQ(result.y, 2.f);
		EXPECT_FLOAT_EQ(result.z, 3.f);
	}
	{
		const float m[] = {
			 1.f,  2.f,  3.f,  4.f,
			 5.f,  6.f,  7.f,  8.f,
			 9.f, 10.f, 11.f, 12.f,
			13.f, 14.f, 15.f, 16.f
		};
		const TMatrixF matrix(m);
		const TVectorF result = matrix * vector;
		EXPECT_FLOAT_EQ(result.x, 14.f);
		EXPECT_FLOAT_EQ(result.y, 38.f);
		EXPECT_FLOAT_EQ(result.z, 62.f);
	}
}


TEST(Matrix4x4, MulVectorLeft)
{
	TVectorF vector(1.f, 2.f, 3.f);
	{
		const TMatrixF matrix = TMatrixF::identity();
		const TVectorF result = vector * matrix;
		EXPECT_FLOAT_EQ(result.x, 1.f);
		EXPECT_FLOAT_EQ(result.y, 2.f);
		EXPECT_FLOAT_EQ(result.z, 3.f);
	}
	{
		const float m[] = {
			 1.f,  2.f,  3.f,  4.f,
			 5.f,  6.f,  7.f,  8.f,
			 9.f, 10.f, 11.f, 12.f,
			13.f, 14.f, 15.f, 16.f
		};
		const TMatrixF matrix(m);
		const TVectorF result = vector * matrix;
		EXPECT_FLOAT_EQ(result.x, 38.f);
		EXPECT_FLOAT_EQ(result.y, 44.f);
		EXPECT_FLOAT_EQ(result.z, 50.f);
	}
}


TEST(Matrix4x4, MulPoint)
{
	TPointF point(1.f, 2.f, 3.f);
	{
		const TMatrixF matrix = TMatrixF::identity();
		const TPointF result = matrix * point;
		EXPECT_FLOAT_EQ(result.x, 1.f);
		EXPECT_FLOAT_EQ(result.y, 2.f);
		EXPECT_FLOAT_EQ(result.z, 3.f);
	}
	{
		const float m[] = {
			 1.f,  2.f,  3.f,  4.f,
			 5.f,  6.f,  7.f,  8.f,
			 9.f, 10.f, 11.f, 12.f,
			13.f, 14.f, 15.f, 16.f
		};
		const TMatrixF matrix(m);
		const TPointF result = matrix * point;
		EXPECT_FLOAT_EQ(result.x, 0.17647059f);
		EXPECT_FLOAT_EQ(result.y, 0.45098039f);
		EXPECT_FLOAT_EQ(result.z, 0.72549020f);
	}
}


TEST(Matrix4x4, MulMatrix)
{
	{
		const TMatrixF matrix = TMatrixF::identity();
		const TMatrixF result = matrix * matrix;
		isIdentity(result);
	}
}

TEST(Matrix4x4, Inverted)
{
	{
		const TMatrixF matrix = TMatrixF::identity();
		const TMatrixF inverted = matrix.inverted();
		isIdentity(inverted);
	}
	{
		const float m[] = {
			 1.f,  3.f,  2.f,  4.f,
			 5.f,  8.f,  7.f,  5.f,
			14.f, 10.f, 11.f, 16.f,
			13.f,  9.f, 15.f, 12.f
		};
		const TMatrixF matrix(m);
		const TMatrixF inverted = matrix.inverted();

		const TPointF point(1.f, 2.f, 3.f);
		const auto point2 = inverted * (matrix * point);
		EXPECT_NEAR(point2.x, point.x, 1e-5f);
		EXPECT_NEAR(point2.y, point.y, 1e-5f);
		EXPECT_NEAR(point2.z, point.z, 1e-5f);

		const TMatrixF result = matrix * inverted;
		isIdentity(result);
	}
}
