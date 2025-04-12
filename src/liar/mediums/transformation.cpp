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

#include "mediums_common.h"
#include "transformation.h"

namespace liar
{
namespace mediums
{

PY_DECLARE_CLASS_DOC(Transformation, "")
PY_CLASS_CONSTRUCTOR_2(Transformation, const TMediumRef&, const TTransformation3D&)
PY_CLASS_CONSTRUCTOR_2(Transformation, const TMediumRef&, const TPyTransformation3DRef&)
PY_CLASS_MEMBER_RW(Transformation, child, setChild)
PY_CLASS_MEMBER_RW(Transformation, localToWorld, setLocalToWorld)

// --- public --------------------------------------------------------------------------------------

Transformation::Transformation(const TMediumRef& child, const TTransformation3D& localToWorld):
	child_(child)
{
	setLocalToWorld(localToWorld);
}



Transformation::Transformation(const TMediumRef& child, const TPyTransformation3DRef& localToWorld):
	child_(child)
{
	setLocalToWorld(localToWorld->transformation());
}



const TMediumRef& Transformation::child() const
{
	return child_;
}



void Transformation::setChild(const TMediumRef& child)
{
	child_ = child;
}



const TTransformation3D& Transformation::localToWorld() const
{
	return localToWorld_;
}



void Transformation::setLocalToWorld(const TTransformation3D& localToWorld)
{
	localToWorld_ = localToWorld;
	worldToLocal_ = localToWorld_.inverse();
}




// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

size_t Transformation::doNumScatterSamples() const
{
	return child_->numScatterSamples();
}



const Spectral Transformation::doTransmittance(const Sample& sample, const BoundedRay& ray) const
{
	return child_->transmittance(sample, transform(ray, worldToLocal_));
}



const Spectral Transformation::doEmission(const Sample& sample, const BoundedRay& ray) const
{
	return child_->emission(sample, transform(ray, worldToLocal_));
}



const Spectral Transformation::doScatterOut(const Sample& sample, const BoundedRay& ray) const
{
	return child_->scatterOut(sample, transform(ray, worldToLocal_));
}



const Spectral Transformation::doSampleScatterOut(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const
{
	TScalar scale = 1;
	const BoundedRay local = transform(ray, worldToLocal_, scale);
	const Spectral result = child_->sampleScatterOut(sample, local, tScatter, pdf);
	tScatter /= scale;
	return result;
}



const Spectral Transformation::doSampleScatterOutOrTransmittance(const Sample& sample, TScalar scatterSample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const
{
	TScalar scale = 1;
	const BoundedRay local = transform(ray, worldToLocal_, scale);
	const Spectral result = child_->sampleScatterOutOrTransmittance(sample, scatterSample, local, tScatter, pdf);
	tScatter /= scale;
	return result;
}


const Spectral Transformation::doPhase(const Sample& sample, const TPoint3D& pos, const TVector3D& dirIn, const TVector3D& dirOut, TScalar& pdf) const
{
	return child_->phase(sample, transform(pos, worldToLocal_), transform(dirIn, worldToLocal_), transform(dirOut, worldToLocal_), pdf);
}



const Spectral Transformation::doSamplePhase(const Sample& sample, const TPoint2D& phaseSample, const TPoint3D& pos, const TVector3D& dirIn, TVector3D& dirOut, TScalar& pdf) const
{
	const Spectral result = child_->samplePhase(sample, phaseSample, transform(pos, worldToLocal_), transform(dirIn, worldToLocal_), dirOut, pdf);
	dirOut = transform(dirOut, localToWorld_);
	return result;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
