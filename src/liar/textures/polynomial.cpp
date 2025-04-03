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

#include "textures_common.h"
#include "polynomial.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(Polynomial, "polynomial evaluation of texture values")
PY_CLASS_CONSTRUCTOR_2(Polynomial, const TTexturePtr&, const Polynomial::TCoefficients&);

// --- public --------------------------------------------------------------------------------------

Polynomial::Polynomial(const TTexturePtr& texture, const TCoefficients& coefficients):
	UnaryOperator(texture),
	polynomial_(coefficients)
{
}



const Polynomial::TCoefficients& Polynomial::coefficients() const
{
	return polynomial_.coefficients();
}



void Polynomial::setCoefficients(const TCoefficients& coefficients)
{
	polynomial_ = TPolynomial(coefficients);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const Spectral Polynomial::doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const
{
	const size_t n = polynomial_.size();
	if (n == 0)
		return Spectral();

	Spectral result(polynomial_[0]);
	if (n == 1)
		return Spectral(std::move(result), type);

	const Spectral x = texture()->lookUp(sample, context, SpectralType::Illuminant);
	result.fma(x, polynomial_[1]);
	if (n == 2)
		return Spectral(std::move(result), type);

	Spectral xx = x;
	for (size_t i = 2; i < n; ++i)
	{
		xx *= x;
		result.fma(xx, polynomial_[i]);
	}
	return Spectral(std::move(result), type);
}


Texture::TValue Polynomial::doScalarLookUp(const Sample& sample, const IntersectionContext& context) const
{
	return polynomial_(texture()->scalarLookUp(sample, context));
}


// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
