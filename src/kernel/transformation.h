/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
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

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_TRANSFORMATION_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_TRANSFORMATION_H

#include "kernel_common.h"
#include <lass/spat/planar_mesh.h>

namespace liar
{
namespace kernel
{

/** this is a bit of a wrapper object to work with transformations in python.
 *  it should be replaced sometime by direct support from lass.
 */
class LIAR_KERNEL_DLL PyTransformation3D: public python::PyObjectPlus
{
	PY_HEADER(python::PyObjectPlus)
public:
	typedef std::vector<TScalar> TMatrix;
	PyTransformation3D(const TTransformation3D& transformation);
	PyTransformation3D(const TMatrix& matrix);
	PyTransformation3D(const TPoint3D& origin, const TVector3D& x, const TVector3D& y, const TVector3D& z);
	const TTransformation3D& transformation() const; 
	const TMatrix matrix() const;
	const PyTransformation3D inverse() const;
	const PyTransformation3D concatenate(const PyTransformation3D& other) const;
	bool isIdentity() const;
	bool isTranslation() const;
	static const PyTransformation3D identity();
	static const PyTransformation3D translation(const TVector3D& offset);
	static const PyTransformation3D scaler(TScalar scale);
	static const PyTransformation3D scalerPerAxis(const TVector3D& scale);
	static const PyTransformation3D rotation(prim::XYZ axis, TScalar radians);
	static const PyTransformation3D rotationAroundVector(const TVector3D& axis, TScalar radians);
	static const PyTransformation3D lookAt(const TPoint3D& eye, const TPoint3D& target, const TVector3D& sky);
private:
	TTransformation3D transformation_;
};

typedef python::PyObjectPtr<PyTransformation3D>::Type TPyTransformation3DPtr;

}

}

#endif

// EOF
