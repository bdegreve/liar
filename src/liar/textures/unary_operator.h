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

/** @class liar::textures::UnaryOperator
 *  @brief base class for textures operating on one input texture.
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_UNARY_OPERATOR_H
#define LIAR_GUARDIAN_OF_INCLUSION_UNARY_OPERATOR_H

#include "textures_common.h"
#include "../kernel/texture.h"

namespace liar
{
namespace textures
{

class LIAR_TEXTURES_DLL UnaryOperator: public Texture
{
	PY_HEADER(Texture)
public:

	const TTextureRef& texture() const { return texture_; }
	void setTexture(const TTextureRef& texture);

protected:

	UnaryOperator(const TTextureRef& texture);

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

private:

	bool doIsChromatic() const override;

	TTextureRef texture_;
};

}

}

#endif

// EOF
