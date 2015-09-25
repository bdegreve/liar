/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::textures::ProjectionMapping
 *  @brief computes UV coordinates using a Projection
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_PROJECTION_MAPPING_H
#define LIAR_GUARDIAN_OF_INCLUSION_PROJECTION_MAPPING_H

#include "textures_common.h"
#include "unary_operator.h"
#include "../kernel/projection.h"

namespace liar
{
namespace textures
{

class LIAR_TEXTURES_DLL ProjectionMapping: public UnaryOperator
{
	PY_HEADER(UnaryOperator)
public:

	ProjectionMapping(const TTexturePtr& texture, const TProjectionPtr& center);

	const TProjectionPtr& projection() const;
	void setProjection(const TProjectionPtr& projection);

protected:

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

private:

	const Spectrum doLookUp(const Sample& sample, const IntersectionContext& context) const;

	TProjectionPtr projection_;
};

}

}

#endif

// EOF
