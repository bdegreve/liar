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

/** @class liar::textures::ContextMapping
 *  @brief Intermediate base class that transforms context before look up.
 *	@author Bram de Greve (bramz@users.sourceforge.net)
*/

#ifndef LIAR_GUARDIAN_OF_INCLUSION_CONTEXT_MAPPING_H
#define LIAR_GUARDIAN_OF_INCLUSION_CONTEXT_MAPPING_H

#include "textures_common.h"
#include "unary_operator.h"

namespace liar
{
namespace textures
{

class LIAR_TEXTURES_DLL ContextMapping: public UnaryOperator
{
	PY_HEADER(UnaryOperator)
protected:

	ContextMapping(const TTextureRef& texture);

private:

	const Spectral doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const override;
	TValue doScalarLookUp(const Sample& sample, const IntersectionContext& context) const override;

	virtual void doTransformContext(const Sample& sample, IntersectionContext& context) const = 0;
};

}

}

#endif

// EOF
