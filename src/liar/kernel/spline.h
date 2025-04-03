/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2021  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::Texture
 *  @brief abstract base class of all textures
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SPLINE_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SPLINE_H

#include "kernel_common.h"
#include <lass/num/spline_linear.h>
#include <lass/num/spline_bezier_path.h>
#include <lass/num/spline_cubic.h>

namespace liar
{
namespace kernel
{

/** this is a bit of a wrapper object to work with splines in python.
 *  it should be replaced sometime by direct support from lass.
 */
class LIAR_KERNEL_DLL ScalarSpline: public python::PyObjectPlus
{
    PY_HEADER(python::PyObjectPlus)
public:
    TScalar operator()(TTime t) const;
    TScalar derivative(TTime t) const;
    TScalar derivative2(TTime t) const;
    TScalar integral(TTime begin, TTime end) const;
protected:
    typedef num::Spline<TTime, TScalar> TSpline;
    ScalarSpline(TSpline* spline);
private:
    std::unique_ptr<TSpline> spline_;
};


class LIAR_KERNEL_DLL LinearScalarSpline: public ScalarSpline
{
    PY_HEADER(ScalarSpline)
public:
    typedef std::pair<TTime, TScalar> TControlPoint;
    typedef std::vector<TControlPoint> TControlSeries;
    LinearScalarSpline(const TControlSeries& controlPoints);
};


class LIAR_KERNEL_DLL CubicScalarSpline: public ScalarSpline
{
    PY_HEADER(ScalarSpline)
public:
    typedef std::pair<TTime, TScalar> TControlPoint;
    typedef std::vector<TControlPoint> TControlSeries;
    CubicScalarSpline(const TControlSeries& controlPoints);
};

}
}

#endif
