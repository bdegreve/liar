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

/** @class liar::textures::FBm
 *  @brief Perlin based Fractional Brownian motion texture.
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  Uses IntersectionContext::point as input.
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_FBM_H
#define LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_FBM_H

#include "textures_common.h"
#include "perlin.h"

namespace liar
{
namespace textures
{

class LIAR_TEXTURES_DLL FBm: public Perlin
{
	PY_HEADER(Perlin)

public:

	FBm(size_t numOctaves);
	FBm(size_t numOctaves, TValue falloff);

protected:
	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

private:
	const Spectral doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const override;
	TValue doScalarLookUp(const Sample& sample, const IntersectionContext& context) const override;

	void init(size_t numOctaves, TValue falloff = .5);

	size_t numOctaves_;
	TValue falloff_;
};

}

}

#endif

// EOF
