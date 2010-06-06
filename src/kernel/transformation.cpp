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
#include "transformation.h"

namespace liar
{
namespace kernel
{

// --- PyTransformation3D --------------------------------------------------------------------------

PY_DECLARE_CLASS_NAME(PyTransformation3D, "Transformation3D")
PY_CLASS_CONSTRUCTOR_1(PyTransformation3D, const TTransformation3D&)
PY_CLASS_CONSTRUCTOR_1(PyTransformation3D, const PyTransformation3D::TMatrix&)
PY_CLASS_CONSTRUCTOR_4(PyTransformation3D, const TPoint3D&, const TVector3D&, const TVector3D&, const TVector3D&)
PY_CLASS_MEMBER_R(PyTransformation3D, matrix)
PY_CLASS_METHOD(PyTransformation3D, inverse)
PY_CLASS_METHOD(PyTransformation3D, isIdentity)
PY_CLASS_METHOD(PyTransformation3D, isTranslation)
PY_CLASS_METHOD(PyTransformation3D, concatenate)
PY_CLASS_STATIC_METHOD(PyTransformation3D, identity)
PY_CLASS_STATIC_METHOD(PyTransformation3D, translation)
PY_CLASS_STATIC_METHOD(PyTransformation3D, scaler)
PY_CLASS_STATIC_METHOD_NAME(PyTransformation3D, scalerPerAxis, "scaler")
PY_CLASS_STATIC_METHOD(PyTransformation3D, rotation)
PY_CLASS_STATIC_METHOD_NAME(PyTransformation3D, rotationAroundVector, "rotation")
PY_CLASS_STATIC_METHOD(PyTransformation3D, lookAt)

PyTransformation3D::PyTransformation3D(const TTransformation3D& transformation):
	transformation_(transformation) 
{
}


PyTransformation3D::PyTransformation3D(const TMatrix& matrix):
	transformation_(matrix.begin(), matrix.end()) 
{
}


PyTransformation3D::PyTransformation3D(const TPoint3D& origin, const TVector3D& x, const TVector3D& y, const TVector3D& z):
	transformation_(origin, x, y, z) 
{
}


const TTransformation3D& PyTransformation3D::transformation() const 
{ 
	return transformation_; 
}


const PyTransformation3D::TMatrix PyTransformation3D::matrix() const 
{
	const TScalar* mat = transformation_.matrix();
	TMatrix result;
	std::copy(mat, mat + 16, std::back_inserter(result));
	return result; 
}


const PyTransformation3D PyTransformation3D::inverse() const 
{ 
	return PyTransformation3D(transformation_.inverse()); 
}


bool PyTransformation3D::isIdentity() const 
{ 
	return transformation_.isIdentity();
}


bool PyTransformation3D::isTranslation() const 
{ 
	return transformation_.isTranslation();
}


const PyTransformation3D PyTransformation3D::concatenate(const PyTransformation3D& other) const
{
	return PyTransformation3D(prim::concatenate(transformation_, other.transformation_));
}


const PyTransformation3D PyTransformation3D::identity() 
{ 
	return PyTransformation3D(TTransformation3D::identity()); 
}


const PyTransformation3D PyTransformation3D::translation(const TVector3D& offset) 
{ 
	return PyTransformation3D(TTransformation3D::translation(offset)); 
}


const PyTransformation3D PyTransformation3D::scaler(TScalar scale) 
{ 
	return PyTransformation3D(TTransformation3D::scaler(scale)); 
}


const PyTransformation3D PyTransformation3D::scalerPerAxis(const TVector3D& scale) 
{ 
	return PyTransformation3D(TTransformation3D::scaler(scale)); 
}


const PyTransformation3D PyTransformation3D::rotation(prim::XYZ axis, TScalar radians) 
{ 
	return PyTransformation3D(TTransformation3D::rotation(axis, radians)); 
}


const PyTransformation3D PyTransformation3D::rotationAroundVector(const TVector3D& axis, TScalar radians) 
{ 
	return PyTransformation3D(TTransformation3D::rotation(axis, radians)); 
}


const PyTransformation3D PyTransformation3D::lookAt(const TPoint3D& eye, const TPoint3D& target, const TVector3D& sky) 
{ 
	return PyTransformation3D(TTransformation3D::lookAt(eye, target, sky));
}

}
}

// EOF
