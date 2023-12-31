/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2020  Bram de Greve (bramz@users.sourceforge.net)
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

	const TTexturePtr& texture() const { return texture_; }
	void setTexture(const TTexturePtr& texture);

protected:

	UnaryOperator(const TTexturePtr& texture);

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

private:

	bool doIsChromatic() const override;

	TTexturePtr texture_;
};

}

}

#endif

// EOF
