/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2012  Bram de Greve (bramz@users.sourceforge.net)
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
#include "spline.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(ScalarSpline, "Abstract base class of scalar splines")
PY_CLASS_METHOD_NAME(ScalarSpline, operator(), python::methods::_call_)
PY_CLASS_METHOD(ScalarSpline, derivative)
PY_CLASS_METHOD(ScalarSpline, derivative2)
PY_CLASS_METHOD(ScalarSpline, integral)


TScalar ScalarSpline::operator()(TTime t) const
{
    return spline_->operator()(t);
}


TScalar ScalarSpline::derivative(TTime t) const
{
    return spline_->derivative(t);
}


TScalar ScalarSpline::derivative2(TTime t) const
{
    return spline_->derivative2(t);
}


TScalar ScalarSpline::integral(TTime begin, TTime end) const
{
    return spline_->integral(begin, end);
}


ScalarSpline::ScalarSpline(TSpline* spline):
    spline_(spline)
{
    LASS_ASSERT(spline_.get());
}


PY_DECLARE_CLASS_DOC(LinearScalarSpline, "Scalar spline with linear interpolation")
PY_CLASS_CONSTRUCTOR_1(LinearScalarSpline, const LinearScalarSpline::TControlSeries&)

LinearScalarSpline::LinearScalarSpline(const TControlSeries& controlPoints):
    ScalarSpline(new num::SplineLinear< TTime, TScalar, num::DataTraitsScalar<TScalar> >( controlPoints.begin(), controlPoints.end() ))
{
}


PY_DECLARE_CLASS_DOC(CubicScalarSpline, "Scalar spline with cubic interpolation")
PY_CLASS_CONSTRUCTOR_1(CubicScalarSpline, const CubicScalarSpline::TControlSeries&)

CubicScalarSpline::CubicScalarSpline(const TControlSeries& controlPoints):
    ScalarSpline(new num::SplineCubic< TTime, TScalar, num::DataTraitsScalar<TScalar> >( controlPoints.begin(), controlPoints.end() ))
{
}


}
}