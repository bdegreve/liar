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

/** @class liar::textures::Perlin
 *  @brief Perlin noise in 3D.
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  Uses IntersectionContext::point as input.
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_PERLIN_H
#define LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_PERLIN_H

#include "textures_common.h"
#include "../kernel/texture.h"
#include <lass/num/random.h>

namespace liar
{
namespace textures
{

class LIAR_TEXTURES_DLL Perlin: public Texture
{
	PY_HEADER(Texture)
public:

	typedef num::Tuint32 TSeed;

	Perlin();
	Perlin(TSeed seed);

protected:

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

	TValue noise(const TPoint3D& point) const;

private:


	enum
	{
		dimension_ = TVector3D::dimension,
		numGradients_ = 16,
		hashSize_ = 256,
		hashPadding_ = 1,
	};

	LASS_META_ASSERT(hashSize_ % numGradients_ == 0, hash_size_must_be_a_multiple_of_num_gradient_vectors);

	const Spectral doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const override;
	TValue doScalarLookUp(const Sample& sample, const IntersectionContext& context) const override;
	bool doIsChromatic() const override;


	void init(TSeed seed = 0);
	TValue gradient(size_t x, size_t y, size_t z, TValue dx, TValue dy, TValue dz) const;
	static TValue kernel(TValue dx);

	size_t hashTables_[dimension_][hashSize_ + hashPadding_];
	TSeed seed_;
};

}

}

#endif

// EOF
