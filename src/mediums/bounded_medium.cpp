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
#include "bounded_medium.h"

namespace liar
{
namespace mediums
{

PY_DECLARE_CLASS_DOC(BoundedMedium, "")
PY_CLASS_CONSTRUCTOR_0(BoundedMedium)
PY_CLASS_CONSTRUCTOR_2(BoundedMedium, const TMediumPtr&, const TAabb3D&)
PY_CLASS_MEMBER_RW(BoundedMedium, medium, setMedium)
PY_CLASS_MEMBER_RW(BoundedMedium, bounds, setBounds)

// --- public --------------------------------------------------------------------------------------

BoundedMedium::BoundedMedium()
{
}



BoundedMedium::BoundedMedium(const TMediumPtr& medium, const TAabb3D& bounds):
	medium_(medium), bounds_(bounds)
{
}



const TMediumPtr& BoundedMedium::medium() const
{
	return medium_;
}



void BoundedMedium::setMedium(const TMediumPtr& medium)
{
	medium_ = medium;
}



const TAabb3D& BoundedMedium::bounds() const
{
	return bounds_;
}



void BoundedMedium::setBounds(const TAabb3D& bounds)
{
	bounds_ = bounds;
}




// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

size_t BoundedMedium::doNumScatterSamples() const
{
	return medium_ ? medium_->numScatterSamples() : 0;
}



const XYZ BoundedMedium::doTransmittance(const BoundedRay& ray) const
{
	BoundedRay bounded;
	if (!medium_ || !bound(ray, bounded))
	{
		return XYZ(1);
	}
	return medium_->transmittance(bounded);
}



const XYZ BoundedMedium::doEmission(const BoundedRay& ray) const
{
	BoundedRay bounded;
	if (!medium_ || !bound(ray, bounded))
	{
		return XYZ(0);
	}
	return medium_->emission(bounded);
}



const XYZ BoundedMedium::doScatterOut(const BoundedRay& ray) const
{
	BoundedRay bounded;
	if (!medium_ || !bound(ray, bounded))
	{
		return XYZ(0);
	}
	return medium_->scatterOut(bounded);
}



const XYZ BoundedMedium::doSampleScatterOut(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const
{
	BoundedRay bounded;
	if (!medium_ || !bound(ray, bounded))
	{
		pdf = 0;
		return XYZ(0);
	}
	return medium_->sampleScatterOut(sample, bounded, tScatter, pdf);
}



const XYZ BoundedMedium::doSampleScatterOutOrTransmittance(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const
{
	BoundedRay bounded;
	if (!medium_ || !bound(ray, bounded))
	{
		tScatter = ray.farLimit();
		pdf = 1;
		return XYZ(1);
	}
	return medium_->sampleScatterOutOrTransmittance(sample, bounded, tScatter, pdf);
}


const XYZ BoundedMedium::doPhase(const TPoint3D& pos, const TVector3D& dirIn, const TVector3D& dirOut, TScalar& pdf) const
{
	if (!medium_ || !bounds_.contains(pos))
	{
		pdf = 0;
		return XYZ(0);
	}
	return medium_->phase(pos, dirIn, dirOut, pdf);
}



const XYZ BoundedMedium::doSamplePhase(const TPoint2D& sample, const TPoint3D& position, const TVector3D& dirIn, TVector3D& dirOut, TScalar& pdf) const
{
	if (!medium_ || !bounds_.contains(position))
	{
		pdf = 0;
		dirOut = dirIn;
		return XYZ(0);
	}
	return medium_->samplePhase(sample, position, dirIn, dirOut, pdf);
}



bool BoundedMedium::bound(const BoundedRay& ray, BoundedRay& bounded) const
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
