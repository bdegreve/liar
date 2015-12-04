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

#include "shaders_common.h"
#include "conductor.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(Conductor, "Conductive Fresnel material (like metal)")
	PY_CLASS_CONSTRUCTOR_0(Conductor)
	PY_CLASS_CONSTRUCTOR_1(Conductor, const TTexturePtr&)
	PY_CLASS_CONSTRUCTOR_2(Conductor, const TTexturePtr&, const TTexturePtr&)
	PY_CLASS_MEMBER_RW_DOC(Conductor, refractionIndex, setRefractionIndex,
	"index of refraction for material")
	PY_CLASS_MEMBER_RW_DOC(Conductor, absorptionCoefficient, setAbsorptionCoefficient,
	"index of refraction for material on outside")
	PY_CLASS_MEMBER_RW_DOC(Conductor, reflectance, setReflectance,
	"multiplier for the Fresnel reflectance")

// --- public --------------------------------------------------------------------------------------

Conductor::Conductor() :
	Shader(Bsdf::capsReflection | Bsdf::capsSpecular)
{
	init();
}



Conductor::Conductor(const TTexturePtr& refractionIndex) :
	Shader(Bsdf::capsReflection | Bsdf::capsSpecular)
{
	init(refractionIndex);
}



Conductor::Conductor(const TTexturePtr& refractionIndex, const TTexturePtr& absorptionCoefficient) :
	Shader(Bsdf::capsReflection | Bsdf::capsSpecular)
{
	init(refractionIndex, absorptionCoefficient);
}



const TTexturePtr& Conductor::refractionIndex() const
{
	return refractionIndex_;
}



void Conductor::setRefractionIndex(const TTexturePtr& refractionIndex)
{
	refractionIndex_ = refractionIndex;
}



const TTexturePtr& Conductor::absorptionCoefficient() const
{
	return absorptionCoefficient_;
}



void Conductor::setAbsorptionCoefficient(const TTexturePtr& absorptionCoefficient)
{
	absorptionCoefficient_ = absorptionCoefficient;
}



const TTexturePtr& Conductor::reflectance() const
{
	return reflectance_;
}



void Conductor::setReflectance(const TTexturePtr& reflectance)
{
	reflectance_ = reflectance;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

size_t Conductor::doNumReflectionSamples() const
{
	return 1;
}



TBsdfPtr Conductor::doBsdf(const Sample& sample, const IntersectionContext& context) const
{
	// in theory, refractive indices must be at least 1, but they must be more than zero for sure.
	// IOR as average of spectral works correctly for LIAR_SPECTRAL_MODE_SINGLE. Everything else uses ... well, and average.
	const Spectral eta = max(refractionIndex_->lookUp(sample, context, Illuminant), 1e-9);
	const Spectral kappa = max(absorptionCoefficient_->lookUp(sample, context, Illuminant), 1e-9);
	const Spectral reflectance = reflectance_->lookUp(sample, context, Reflectant);

	return TBsdfPtr(new ConductorBsdf(sample, context, caps(), eta, kappa, reflectance));
}


const TPyObjectPtr Conductor::doGetState() const
{
	return python::makeTuple(refractionIndex_, absorptionCoefficient_, reflectance_);
}



void Conductor::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, refractionIndex_, absorptionCoefficient_, reflectance_);
}



void Conductor::init(const TTexturePtr& refractionIndex, const TTexturePtr& absorptionCoefficient, const TTexturePtr& reflectance)
{
	refractionIndex_ = refractionIndex;
	absorptionCoefficient_ = absorptionCoefficient;
	reflectance_ = reflectance;
}



// --- bsdf ----------------------------------------------------------------------------------------

Conductor::ConductorBsdf::ConductorBsdf(const Sample& sample, const IntersectionContext& context, TBsdfCaps caps, const Spectral& eta, const Spectral& kappa, const Spectral& reflectance) :
	Bsdf(sample, context, caps),
	reflectance_(reflectance),
	eta_(eta),
	kappa_(kappa)
{
	LASS_ASSERT(ior_ > 0);
	LASS_ASSERT(kappa_ > 0);
}



BsdfOut Conductor::ConductorBsdf::doEvaluate(const TVector3D&, const TVector3D&, TBsdfCaps) const
{
	return BsdfOut();
}



SampleBsdfOut Conductor::ConductorBsdf::doSample(const TVector3D& omegaIn, const TPoint2D&, TScalar componentSample, TBsdfCaps allowedCaps) const
{
	enum
	{
		capsRefl = Bsdf::capsReflection | Bsdf::capsSpecular,
	};

	const TScalar cosI = omegaIn.z;
	LASS_ASSERT(cosI > 0);
	const TScalar cosI2 = num::sqr(cosI);
	const Spectral a = sqr(eta_) + sqr(kappa_);
	const Spectral b = (2 * cosI) * eta_;

	const Spectral aOrth = a + cosI2;
	const Spectral aPar = a * cosI2 + 1;

	const Spectral rOrth2 = (aOrth - b) / (aOrth + b);
	const Spectral rPar2 = (aPar - b) / (aPar + b);
	const Spectral rFresnel = (rOrth2 + rPar2) / 2;

	const TVector3D omegaRefl(-omegaIn.x, -omegaIn.y, omegaIn.z);
	return SampleBsdfOut(omegaRefl, reflectance_ * rFresnel / cosI, 1, capsRefl);
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
