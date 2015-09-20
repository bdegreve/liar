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

#include "mediums_common.h"
#include "bounded.h"

namespace liar
{
namespace mediums
{

PY_DECLARE_CLASS_DOC(Bounded, "")
PY_CLASS_CONSTRUCTOR_0(Bounded)
PY_CLASS_CONSTRUCTOR_2(Bounded, const TMediumPtr&, const TAabb3D&)
PY_CLASS_MEMBER_RW(Bounded, child, setChild)
PY_CLASS_MEMBER_RW(Bounded, bounds, setBounds)

// --- public --------------------------------------------------------------------------------------

Bounded::Bounded()
{
}



Bounded::Bounded(const TMediumPtr& child, const TAabb3D& bounds):
	child_(child), bounds_(bounds)
{
}



const TMediumPtr& Bounded::child() const
{
	return child_;
}



void Bounded::setChild(const TMediumPtr& child)
{
	child_ = child;
}



const TAabb3D& Bounded::bounds() const
{
	return bounds_;
}



void Bounded::setBounds(const TAabb3D& bounds)
{
	bounds_ = bounds;
}




// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

size_t Bounded::doNumScatterSamples() const
{
	return child_ ? child_->numScatterSamples() : 0;
}



const Spectrum Bounded::doTransmittance(const BoundedRay& ray) const
{
	BoundedRay bounded;
	if (!child_ || !bound(ray, bounded))
	{
		return Spectrum(1);
	}
	return child_->transmittance(bounded);
}



const Spectrum Bounded::doEmission(const BoundedRay& ray) const
{
	BoundedRay bounded;
	if (!child_ || !bound(ray, bounded))
	{
		return Spectrum(0);
	}
	return child_->emission(bounded);
}



const Spectrum Bounded::doScatterOut(const BoundedRay& ray) const
{
	BoundedRay bounded;
	if (!child_ || !bound(ray, bounded))
	{
		return Spectrum(0);
	}
	return child_->scatterOut(bounded);
}



const Spectrum Bounded::doSampleScatterOut(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const
{
	BoundedRay bounded;
	if (!child_ || !bound(ray, bounded))
	{
		pdf = 0;
		return Spectrum(0);
	}
	return child_->sampleScatterOut(sample, bounded, tScatter, pdf);
}



const Spectrum Bounded::doSampleScatterOutOrTransmittance(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const
{
	BoundedRay bounded;
	if (!child_ || !bound(ray, bounded))
	{
		tScatter = ray.farLimit();
		pdf = 1;
		return Spectrum(1);
	}
	return child_->sampleScatterOutOrTransmittance(sample, bounded, tScatter, pdf);
}


const Spectrum Bounded::doPhase(const TPoint3D& pos, const TVector3D& dirIn, const TVector3D& dirOut, TScalar& pdf) const
{
	if (!child_ || !bounds_.contains(pos))
	{
		pdf = 0;
		return Spectrum(0);
	}
	return child_->phase(pos, dirIn, dirOut, pdf);
}



const Spectrum Bounded::doSamplePhase(const TPoint2D& sample, const TPoint3D& position, const TVector3D& dirIn, TVector3D& dirOut, TScalar& pdf) const
{
	if (!child_ || !bounds_.contains(position))
	{
		pdf = 0;
		dirOut = dirIn;
		return Spectrum(0);
	}
	return child_->samplePhase(sample, position, dirIn, dirOut, pdf);
}



bool Bounded::bound(const BoundedRay& ray, BoundedRay& bounded) const
{
	TScalar tNear;
	if (prim::intersect(bounds_, ray.unboundedRay(), tNear, ray.nearLimit()) == prim::rNone)
	{
		return false;
	}
	TScalar tFar;
	if (prim::intersect(bounds_, ray.unboundedRay(), tFar, tNear) == prim::rNone)
	{
		tFar = tNear;
		tNear = ray.nearLimit();
	}
	if (tNear > ray.farLimit())
	{
		return false;
	}
	bounded = kernel::bound(ray, tNear, tFar);
	return true;
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
