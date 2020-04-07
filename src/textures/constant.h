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

/** @class liar::textures::Constant
 *  @brief texture with constant value
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_CONSTANT_H
#define LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_CONSTANT_H

#include "textures_common.h"
#include "../kernel/texture.h"
#include "../kernel/spectrum.h"

namespace liar
{
namespace textures
{

class LIAR_TEXTURES_DLL Constant: public Texture
{
	PY_HEADER(Texture)
public:

	explicit Constant(const TSpectrumPtr& value);
	explicit Constant(const XYZ& value);
	explicit Constant(TValue scalar);

	const TSpectrumPtr& value() const;
	void setValue(const TSpectrumPtr& value);

protected:

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

private:

	const Spectral doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const override;
	TValue doScalarLookUp(const Sample& sample, const IntersectionContext& context) const override;
	bool doIsChromatic() const override;

	TSpectrumPtr value_;
};

}

}

#endif

// EOF
