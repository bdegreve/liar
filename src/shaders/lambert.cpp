/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2006  Bram de Greve
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  http://liar.sourceforge.net
 */

#include "shaders_common.h"
#include "lambert.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS(Lambert)
PY_CLASS_CONSTRUCTOR_0(Lambert)
PY_CLASS_CONSTRUCTOR_1(Lambert, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(Lambert, "diffuse", diffuse, setDiffuse, "texture for diffuse component")

// --- public --------------------------------------------------------------------------------------

Lambert::Lambert():
	Shader(capsReflection | capsDiffuse),
	diffuse_(Texture::white())
{
}



Lambert::Lambert(const TTexturePtr& iDiffuse):
	Shader(capsReflection | capsDiffuse),
	diffuse_(iDiffuse)
{
}



const TTexturePtr& Lambert::diffuse() const
{
	return diffuse_;
}



void Lambert::setDiffuse(const TTexturePtr& iDiffuse)
{
	diffuse_ = iDiffuse;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void Lambert::doBsdf(
		const Sample& sample, const IntersectionContext& context, const TVector3D& omegaOut,
		const TVector3D* firstOmegaIn, const TVector3D* lastOmegaIn,
		Spectrum* firstValue, TScalar* firstPdf) const
{
	const Spectrum diffuseOverPi = diffuse_->lookUp(sample, context) / TNumTraits::pi;
	while (firstOmegaIn != lastOmegaIn)
	{
		if (firstOmegaIn->z * omegaOut.z > 0)
		{
			*firstValue++ = diffuseOverPi;
			*firstPdf++ = num::abs(firstOmegaIn->z) / TNumTraits::pi;
		}
		else
		{
			*firstValue++ = Spectrum();
			*firstPdf++ = 0;
		}
		++firstOmegaIn;
	}
}


void Lambert::doSampleBsdf(
		const Sample& sample, const IntersectionContext& context, const TVector3D& dirIn,
		const TPoint2D* firstBsdfSample, const TPoint2D* lastBsdfSample,
		TVector3D* firstDirOut, Spectrum* firstValue, TScalar* firstPdf) const
{
	const Spectrum diffuseOverPi = diffuse_->lookUp(sample, context) / TNumTraits::pi;
	const TScalar zSign = dirIn.z < 0 ? -1 : 1;
	while (firstBsdfSample != lastBsdfSample)
	{
		TVector3D localDirOut = 
			num::cosineHemisphere(*firstBsdfSample++, *firstPdf++).position();
		localDirOut.z *= zSign;
		*firstDirOut++ = localDirOut;
		*firstValue++ = diffuseOverPi;
	}
}



const TPyObjectPtr Lambert::doGetState() const
{
	return python::makeTuple(diffuse_);
}



void Lambert::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, diffuse_);
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF