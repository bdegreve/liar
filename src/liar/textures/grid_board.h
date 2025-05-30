/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2025  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::textures::GridBoard
 *  @brief Similar to checkboard, but fills three quadrants with same source.
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_GRID_BOARD_H
#define LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_GRID_BOARD_H

#include "textures_common.h"
#include "binary_operator.h"

namespace liar
{
namespace textures
{

class LIAR_TEXTURES_DLL GridBoard: public BinaryOperator
{
	PY_HEADER(BinaryOperator)
public:

	GridBoard(const TTextureRef& a, const TTextureRef& b);

	const TVector2D thickness() const;
	void setThickness(const TVector2D& split);

protected:
	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

private:
	const Spectral doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const override;
	TValue doScalarLookUp(const Sample& sample, const IntersectionContext& context) const override;

	TVector2D halfThickness_;
};

}

}

#endif

// EOF
