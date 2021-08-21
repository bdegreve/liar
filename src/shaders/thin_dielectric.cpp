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

#include "shaders_common.h"
#include "thin_dielectric.h"

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(ThinDielectric, "thin dielectric material")
PY_CLASS_CONSTRUCTOR_0(ThinDielectric)
PY_CLASS_CONSTRUCTOR_1(ThinDielectric, const TTexturePtr&)
PY_CLASS_CONSTRUCTOR_2(ThinDielectric, const TTexturePtr&, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(ThinDielectric, innerRefractionIndex, setInnerRefractionIndex, 
	"index of refraction for material on inside")
PY_CLASS_MEMBER_RW_DOC(ThinDielectric, outerRefractionIndex, setOuterRefractionIndex, 
	"index of refraction for material on outside")
PY_CLASS_MEMBER_RW_DOC(ThinDielectric, transparency, setTransparency, 
	"colour of transparency")

// --- public --------------------------------------------------------------------------------------

ThinDielectric::ThinDielectric():
	Shader(BsdfCaps::reflection | BsdfCaps::transmission | BsdfCaps::specular),
	innerRefractionIndex_(Texture::white()),
	outerRefractionIndex_(Texture::white()),
	transparency_(Texture::white())
{
}



ThinDielectric::ThinDielectric(const TTexturePtr& innerRefractionIndex):
	Shader(BsdfCaps::reflection | BsdfCaps::transmission | BsdfCaps::specular),
	innerRefractionIndex_(innerRefractionIndex),
	outerRefractionIndex_(Texture::white()),
	transparency_(Texture::white())
{
}



ThinDielectric::ThinDielectric(const TTexturePtr& innerRefractionIndex, const TTexturePtr& outerRefractionIndex):
	Shader(BsdfCaps::reflection | BsdfCaps::transmission | BsdfCaps::specular),
	innerRefractionIndex_(innerRefractionIndex),
	outerRefractionIndex_(outerRefractionIndex),
	transparency_(Texture::white())
{
}



const TTexturePtr& ThinDielectric::innerRefractionIndex() const
{
	return innerRefractionIndex_;
}



void ThinDielectric::setInnerRefractionIndex(const TTexturePtr& innerRefractionIndex)
{
	innerRefractionIndex_ = innerRefractionIndex;
}



const TTexturePtr& ThinDielectric::outerRefractionIndex() const
{
	return outerRefractionIndex_;
}



void ThinDielectric::setOuterRefractionIndex(const TTexturePtr& outerRefractionIndex)
{
	outerRefractionIndex_ = outerRefractionIndex;
}



const TTexturePtr& ThinDielectric::transparency() const
{
	return transparency_;
}



void ThinDielectric::setTransparency(const TTexturePtr& transparency)
{
	transparency_ = transparency;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

size_t ThinDielectric::doNumReflectionSamples() const
{
	return 1;
}



size_t ThinDielectric::doNumTransmissionSamples() const
{
	return 1;
}



TBsdfPtr ThinDielectric::doBsdf(const Sample& sample, const IntersectionContext& context) const
{
	const Spectral ior1 = max(outerRefractionIndex_->lookUp(sample, context, Illuminant), 1e-9f);
	const Spectral ior2 = max(innerRefractionIndex_->lookUp(sample, context, Illuminant), 1e-9f);
	const Spectral transparency = max(transparency_->lookUp(sample, context, Reflectant), 0);
	const Spectral ior = ior1 / ior2;
	return TBsdfPtr(new Bsdf(sample, context, caps(), ior, transparency));
}



const TPyObjectPtr ThinDielectric::doGetState() const
{
	return python::makeTuple(outerRefractionIndex_, innerRefractionIndex_, transparency_);
}



void ThinDielectric::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, outerRefractionIndex_, innerRefractionIndex_, transparency_);
}



// --- bsdf ----------------------------------------------------------------------------------------

ThinDielectric::Bsdf::Bsdf(const Sample& sample, const IntersectionContext& context, BsdfCaps caps, const Spectral& ior, const Spectral& transparency) :
	kernel::Bsdf(sample, context, caps),
	transparency_(transparency),
	ior_(ior)
{
}



BsdfOut ThinDielectric::Bsdf::doEvaluate(const TVector3D&, const TVector3D&, BsdfCaps) const
{
	return BsdfOut();
}



SampleBsdfOut ThinDielectric::Bsdf::doSample(const TVector3D& omegaIn, const TPoint2D&, TScalar componentSample, BsdfCaps allowedCaps) const
{
	// http://users.skynet.be/bdegreve/writings/reflection_transmission.pdf

	typedef Spectral::TValue TValue;

	constexpr BsdfCaps capsRefl = BsdfCaps::reflection | BsdfCaps::specular;
	constexpr BsdfCaps capsTrans = BsdfCaps::transmission | BsdfCaps::specular;

	const TValue cosI = static_cast<TValue>(omegaIn.z);
	LASS_ASSERT(cosI > 0);
	Spectral sinT2 = sqr(ior_) * (1 - num::sqr(cosI));

	sinT2.inpclamp(0, 1);
	const Spectral cosT = sqrt(1 - sinT2);
	//LASS_ASSERT(cosT > 0);
	const Spectral rOrth = (ior_ * cosI - cosT) / (ior_ * cosI + cosT);
	const Spectral rPar = (cosI - ior_ * cosT) / (cosI + ior_ * cosT);
	const Spectral r = (sqr(rOrth) + sqr(rPar)) / 2;
	//LASS_ASSERT(r < 1);
	const Spectral t = pow(transparency_, 1 / cosT);

	Spectral R = r * (1 + sqr((1 - r) * t) / (1 - sqr(r * t)));
	R.inpclamp(0, 1);
	Spectral T = num::sqr(1 - r) * t / (1 - sqr(r * t));
	T.inpclamp(0, 1);

	const bool doReflection = kernel::hasCaps(allowedCaps, capsRefl);
	const bool doTransmission = kernel::hasCaps(allowedCaps, capsTrans);
	const TValue sr = doReflection ? (doTransmission ? average(R) : 1) : 0;
	const TValue st = doTransmission ? (doReflection ? average(T) : 1) : 0;
	
	LASS_ASSERT(sr >= 0 && st >= 0);
	if (sr <= 0 && st <= 0)
	{
		return SampleBsdfOut();
	}

	const TValue pr = sr / (sr + st);
	if (componentSample < pr)
	{
		const TVector3D omegaOut = TVector3D(-omegaIn.x, -omegaIn.y, omegaIn.z);
		return SampleBsdfOut(omegaOut, R, pr, capsRefl);
	}
	else
	{
		return SampleBsdfOut(-omegaIn, T, 1 - pr, capsTrans);
	}
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
