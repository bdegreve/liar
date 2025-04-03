/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2021  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::textures::Polynomial
 *  @brief polynomial evaluation of texture values
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_POLYNOMIAL_H
#define LIAR_GUARDIAN_OF_INCLUSION_POLYNOMIAL_H

#include "textures_common.h"
#include "unary_operator.h"
#include <lass/num/polynomial.h>

namespace liar
{
namespace textures
{

class LIAR_TEXTURES_DLL Polynomial: public UnaryOperator
{
	PY_HEADER(UnaryOperator)
public:

	typedef std::vector<TValue> TCoefficients;

	Polynomial(const TTexturePtr& texture, const TCoefficients& coefficients);

	const TCoefficients& coefficients() const;
	void setCoefficients(const TCoefficients&);

private:

	typedef lass::num::Polynomial<TValue> TPolynomial;

	const Spectral doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const override;
	TValue doScalarLookUp(const Sample& sample, const IntersectionContext& context) const override;

	TPolynomial polynomial_;
};

}

}

#endif

// EOF
