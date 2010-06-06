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

#include "kernel_common.h"
#include "lass_exports.h"

namespace liar
{
namespace kernel
{
namespace impl
{

TTransformation2D scaler2D(TScalar scale) 
{ 
	return TTransformation2D::scaler(scale); 
}
TTransformation2D scalerPerAxis2D(const TVector2D& scale) 
{ 
	return TTransformation2D::scaler(scale); 
}
TTransformation2D concatenate2D(const TTransformation2D& first, const TTransformation2D& second) 
{ 
	return prim::concatenate(first, second); 
}
TTransformation2D transformation2DFromSequence(const std::vector<TScalar>& sequence)
{
	return TTransformation2D(sequence.begin(), sequence.end());
}

PY_DECLARE_CLASS_NAME(ShadowTransformation2D, "Transformation2D")
PY_CLASS_FREE_CONSTRUCTOR_1(ShadowTransformation2D, transformation2DFromSequence, const std::vector<TScalar>&)
PY_CLASS_CONSTRUCTOR_3(ShadowTransformation2D, const TPoint2D&, const TVector2D&, const TVector2D&)
PY_CLASS_METHOD(ShadowTransformation2D, inverse)
PY_CLASS_METHOD(ShadowTransformation2D, isIdentity)
PY_CLASS_METHOD(ShadowTransformation2D, isTranslation)
PY_CLASS_FREE_METHOD_NAME(ShadowTransformation2D, impl::concatenate2D, "concatenate")
PY_CLASS_STATIC_METHOD(ShadowTransformation2D, identity)
PY_CLASS_STATIC_METHOD(ShadowTransformation2D, translation)
PY_CLASS_STATIC_METHOD_EX(ShadowTransformation2D, impl::scaler2D, "scaler", 0, ShadowTransformation2D_scaler)
PY_CLASS_STATIC_METHOD_EX(ShadowTransformation2D, impl::scalerPerAxis2D, "scaler", 0, ShadowTransformation2D_scalerPerAxis)
PY_CLASS_STATIC_METHOD(ShadowTransformation2D, rotation)

}
}
}

// EOF
