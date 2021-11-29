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
#include "lafortune.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(Lafortune, "Lafortune (1997) BRDF")
PY_CLASS_CONSTRUCTOR_0(Lafortune)
PY_CLASS_CONSTRUCTOR_1(Lafortune, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(Lafortune, diffuse, setDiffuse, "texture for diffuse component")
PY_CLASS_MEMBER_RW(Lafortune, lobes, setLobes)
PY_CLASS_METHOD(Lafortune, addLobe)

typedef Lafortune::Lobe Lobe;
PY_DECLARE_CLASS_DOC(Lobe, "Lobe(x, y, z, power)");
PY_CLASS_INNER_CLASS_NAME(Lafortune, Lobe, "Lobe");
PY_CLASS_CONSTRUCTOR_4(Lobe, const TTexturePtr&, const TTexturePtr&, const TTexturePtr&, const TTexturePtr&)
PY_CLASS_PUBLIC_MEMBER(Lobe, x)
PY_CLASS_PUBLIC_MEMBER(Lobe, y)
PY_CLASS_PUBLIC_MEMBER(Lobe, z)
PY_CLASS_PUBLIC_MEMBER(Lobe, power)
PY_CLASS_METHOD_NAME(Lobe, reduce, "__reduce__")
PY_CLASS_METHOD_NAME(Lobe, getState, "__getstate__")
PY_CLASS_METHOD_NAME(Lobe, setState, "__setstate__")

Lafortune::Lobe::Lobe(TTexturePtr x, TTexturePtr y, TTexturePtr z, TTexturePtr power) :
	x(x),
	y(y),
	z(z),
	power(power)
{
}

const TPyObjectPtr Lafortune::Lobe::reduce() const
{
	return python::makeTuple(
		python::fromNakedToSharedPtrCast<PyObject>(reinterpret_cast<PyObject*>(this->_lassPyGetClassDef()->type())),
		python::makeTuple(), this->getState());
}
const TPyObjectPtr Lafortune::Lobe::getState() const
{
	return python::makeTuple(x, y, z, power);
}
void Lafortune::Lobe::setState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, x, y, z, power);
}

// --- public --------------------------------------------------------------------------------------

// TODO: fix sampling method to do BsdfCaps::glossy too.

Lafortune::Lafortune():
	Shader(BsdfCaps::reflection | BsdfCaps::diffuse),
	diffuse_(Texture::white())
{
}



Lafortune::Lafortune(const TTexturePtr& diffuse):
	Shader(BsdfCaps::reflection | BsdfCaps::diffuse),
	diffuse_(diffuse)
{
}



const TTexturePtr& Lafortune::diffuse() const
{
	return diffuse_;
}



void Lafortune::setDiffuse(const TTexturePtr& diffuse)
{
	diffuse_ = diffuse;
}



const Lafortune::TLobes& Lafortune::lobes() const
{
	return lobes_;
}



void Lafortune::setLobes(const TLobes& lobes)
{
	lobes_ = lobes;
}



void Lafortune::addLobe(TTexturePtr x, TTexturePtr y, TTexturePtr z, TTexturePtr power)
{
	if (lobes_.size() >= capacity)
	{
		LASS_THROW("Lafortune supports a maximum of " << capacity << " lobes.");
	}
	TLobePtr lobe(new Lobe(x, y, z, power));
	lobes_.push_back(lobe);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

TBsdfPtr Lafortune::doBsdf(const Sample& sample, const IntersectionContext& context) const
{
	const Spectral diffuse = diffuse_->lookUp(sample, context, SpectralType::Reflectant);
	std::unique_ptr<LafortuneBsdf> bsdf(new LafortuneBsdf(sample, context, caps(), diffuse));
	for (const TLobePtr& lobe : lobes_)
	{
		bsdf->addLobe(
			lobe->x->lookUp(sample, context, SpectralType::Illuminant),
			lobe->y->lookUp(sample, context, SpectralType::Illuminant),
			lobe->z->lookUp(sample, context, SpectralType::Illuminant),
			lobe->power->lookUp(sample, context, SpectralType::Illuminant));
	}
	return bsdf;
}



const TPyObjectPtr Lafortune::doGetState() const
{
	return python::makeTuple(diffuse_, lobes_);
}



void Lafortune::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, diffuse_, lobes_);
}


// --- bsdf ----------------------------------------------------------------------------------------

Lafortune::LafortuneBsdf::LafortuneBsdf(const Sample& sample, const IntersectionContext& context, BsdfCaps caps, const Spectral& diffuse) :
	Bsdf(sample, context, caps),
	diffuseOverPi_(diffuse / num::NumTraits<TValue>::pi)
{
}


void Lafortune::LafortuneBsdf::addLobe(const Spectral& x, const Spectral& y, const Spectral& z, const Spectral& power)
{
	lobes_.push_back(Lobe(x, y, z, power));
}


// TODO: use a sampling method better than lambertian.

BsdfOut Lafortune::LafortuneBsdf::doEvaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, BsdfCaps LASS_UNUSED(allowedCaps)) const
{
	LASS_ASSERT(shaders::hasCaps(allowedCaps, caps()));
	const TScalar cosTheta = num::abs(omegaOut.z);
	if (cosTheta <= 0)
	{
		return BsdfOut();
	}
	return BsdfOut(eval(omegaIn, omegaOut), cosTheta / TNumTraits::pi);
}


SampleBsdfOut Lafortune::LafortuneBsdf::doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar, BsdfCaps LASS_UNUSED(allowedCaps)) const
{
	LASS_ASSERT(shaders::hasCaps(allowedCaps, caps()));
	SampleBsdfOut out;
	out.omegaOut = num::cosineHemisphere(sample, out.pdf).position();
	out.value = eval(omegaIn, out.omegaOut);
	out.usedCaps = caps();
	return out;
}


Spectral Lafortune::LafortuneBsdf::eval(const TVector3D& omegaIn, const TVector3D& omegaOut) const
{
	Spectral result = diffuseOverPi_;
	const TVector3D omega = omegaIn * omegaOut; // component wise.
	for (TLobes::const_iterator i = lobes_.begin(); i != lobes_.end(); ++i)
	{
		Spectral comp = i->x * static_cast<TValue>(omega.x);
		comp.fma(i->y, static_cast<TValue>(omega.y));
		comp.fma(i->z, static_cast<TValue>(omega.z));
		comp.inpmax(0);
		comp.inppow(i->power);
		result += comp;
	}
	return result;
}


// --- free ----------------------------------------------------------------------------------------




}

}

// EOF
