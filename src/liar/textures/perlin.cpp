/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2023  Bram de Greve (bramz@users.sourceforge.net)
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

#include "textures_common.h"
#include "perlin.h"
#if LIAR_HAVE_PCG
#	include <pcg_random.hpp>
#else
#	include <random>
#endif

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(Perlin, "3D Perlin noise using intersection point as input")
PY_CLASS_CONSTRUCTOR_0(Perlin);
PY_CLASS_CONSTRUCTOR_1(Perlin, Perlin::TSeed);

// --- public --------------------------------------------------------------------------------------

Perlin::Perlin()
{
	init();
}



Perlin::Perlin(TSeed seed)
{
	init(seed);
}



// --- protected -----------------------------------------------------------------------------------

namespace
{

}

Texture::TValue Perlin::noise(const TPoint3D& point) const
{
	typedef prim::Vector3D<size_t> TIndex;
	typedef prim::Vector3D<TValue> TValue3D;

	const TScalar modulo = hashSize_;
	const TPoint3D p(num::mod(point.x, modulo), num::mod(point.y, modulo), num::mod(point.z, modulo));
	const TPoint3D p0(num::floor(p.x), num::floor(p.y), num::floor(p.z));
	const TValue3D dp(static_cast<TValue>(p.x - p0.x), static_cast<TValue>(p.y - p0.y), static_cast<TValue>(p.z - p0.z));
	const TIndex i(static_cast<size_t>(p0.x), static_cast<size_t>(p0.y), static_cast<size_t>(p0.z));

	const TValue3D s = dp.transform(kernel);

	const TValue g[1 << dimension_] =
	{
		gradient(i.x     , i.y     , i.z     , dp.x     , dp.y     , dp.z    ),
		gradient(i.x + 1 , i.y     , i.z     , dp.x - 1 , dp.y     , dp.z    ),
		gradient(i.x     , i.y + 1 , i.z     , dp.x     , dp.y - 1 , dp.z    ),
		gradient(i.x + 1 , i.y + 1 , i.z     , dp.x - 1 , dp.y - 1 , dp.z    ),
		gradient(i.x     , i.y     , i.z + 1 , dp.x     , dp.y     , dp.z - 1),
		gradient(i.x + 1 , i.y     , i.z + 1 , dp.x - 1 , dp.y     , dp.z - 1),
		gradient(i.x     , i.y + 1 , i.z + 1 , dp.x     , dp.y - 1 , dp.z - 1),
		gradient(i.x + 1 , i.y + 1 , i.z + 1 , dp.x - 1 , dp.y - 1 , dp.z - 1),
	};

	return num::lerp(
		num::lerp(num::lerp(g[0], g[1], s.x), num::lerp(g[2], g[3], s.x), s.y),
		num::lerp(num::lerp(g[4], g[5], s.x), num::lerp(g[6], g[7], s.x), s.y),
		s.z);
}



// --- private -------------------------------------------------------------------------------------

const Spectral Perlin::doLookUp(const Sample&, const IntersectionContext& context, SpectralType type) const
{
	return Spectral(noise(context.point()), type);
}



Texture::TValue Perlin::doScalarLookUp(const Sample&, const IntersectionContext& context) const
{
	return noise(context.point());
}



bool Perlin::doIsChromatic() const
{
	return false;
}



const TPyObjectPtr Perlin::doGetState() const
{
	return python::makeTuple(seed_);
}



void Perlin::doSetState(const TPyObjectPtr& state)
{
	TSeed seed;
	python::decodeTuple(state, seed);
	init(seed);
}



void Perlin::init(TSeed seed)
{
	seed_ = seed;
#if LIAR_HAVE_PCG
	pcg32_k2_fast rng(seed_);
#else
	std::minstd_rand rng(seed_);
#endif
	for (size_t k = 0; k < dimension_; ++k)
	{
		size_t* table = hashTables_[k];
		for (size_t i = 0; i < hashSize_; ++i)
		{
			table[i] = i;
		}
		std::shuffle(table, table + hashSize_, rng);
		std::copy(table, table + hashPadding_, table + hashSize_); // fill padding
	}
}



Texture::TValue Perlin::gradient(size_t x, size_t y, size_t z, TValue dx, TValue dy, TValue dz) const
{
	LASS_ASSERT(x < (hashSize_ + hashPadding_) && y < (hashSize_ + hashPadding_) && z < (hashSize_ + hashPadding_));
	const size_t hashed = hashTables_[0][x] ^ hashTables_[1][y] ^ hashTables_[2][z];

	switch (hashed % numGradients_)
	{
	case 0: return dx + dy;
	case 1: return dy - dx;
	case 2: return dx - dy;
	case 3: return -dx - dy;
	case 4: return dx + dz;
	case 5: return dz - dx;
	case 6: return dx - dz;
	case 7: return -dx - dz;
	case 8: return dy + dz;
	case 9: return dz - dy;
	case 10: return dy - dz;
	case 11: return -dy - dz;
	case 12: return dx + dy;
	case 13: return dy - dx;
	case 14: return dz - dy;
	case 15: return -dy - dz;
	default:
		LASS_ENFORCE_UNREACHABLE;
		return 0;
	};

}



Texture::TValue Perlin::kernel(TValue dx)
{
	const TValue dx2 = dx * dx;
	const TValue dx3 = dx2 * dx;
	return (6 * dx3 - 15 * dx2 + 10 * dx) * dx2;
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
