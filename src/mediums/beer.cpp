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

#include "mediums_common.h"
#include "beer.h"

namespace liar
{
namespace mediums
{

PY_DECLARE_CLASS_DOC(Beer, "Beer's Law")
PY_CLASS_CONSTRUCTOR_0(Beer)
PY_CLASS_CONSTRUCTOR_1(Beer, const TSpectrumPtr&)
PY_CLASS_MEMBER_RW(Beer, transparency, setTransparency)

// --- public --------------------------------------------------------------------------------------

Beer::Beer():
	transparency_(Spectrum::white())
{
}



Beer::Beer(const TSpectrumPtr& transparency) :
	transparency_(transparency)
{
}



const TSpectrumPtr& Beer::transparency() const
{
	return transparency_;
}



void Beer::setTransparency(const TSpectrumPtr& transparency)
{
	transparency_ = transparency;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const Spectral Beer::doTransmittance(const Sample& sample, const BoundedRay& ray) const
{
	const TScalar t = ray.farLimit() - ray.nearLimit();
	LASS_ASSERT(t >= 0);
	return pow(transparency_->evaluate(sample, SpectralType::Reflectant), static_cast<Spectral::TValue>(t));
}



const Spectral Beer::doEmission(const Sample&, const BoundedRay&) const
{
	return Spectral(0);
}


const Spectral Beer::doScatterOut(const Sample&, const BoundedRay&) const
{
	return Spectral(0);
}



const Spectral Beer::doSampleScatterOut(TScalar, const BoundedRay&, TScalar&, TScalar& pdf) const
{
	pdf = 0;
	return Spectral(0);
}



/** As we don't do any scattering in Beer, all incoming photons exit at the end (albeit attenuated)
 */
const Spectral Beer::doSampleScatterOutOrTransmittance(const Sample& sample, TScalar, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const
{
	tScatter = ray.farLimit();
	pdf = 1;
	return transmittance(sample, ray);
}



const Spectral Beer::doPhase(const Sample&, const TPoint3D&, const TVector3D&, const TVector3D&, TScalar& pdf) const
{
	pdf = 0;
	return Spectral(0);
}



const Spectral Beer::doSamplePhase(const Sample&, const TPoint2D&, const TPoint3D&, const TVector3D&, TVector3D&, TScalar& pdf) const
{
	pdf = 0;
	return Spectral(0);
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
