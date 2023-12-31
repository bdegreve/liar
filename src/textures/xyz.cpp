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

#include "textures_common.h"
#include "xyz.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(Xyz, "mixes three textures by the x, y and z context channels")
PY_CLASS_CONSTRUCTOR_3(Xyz, const TTexturePtr&, const TTexturePtr&, const TTexturePtr&);
PY_CLASS_MEMBER_RW_DOC(Xyz, textureA, setTextureA, "first texture")
PY_CLASS_MEMBER_RW_DOC(Xyz, textureB, setTextureB, "second texture")
PY_CLASS_MEMBER_RW_DOC(Xyz, textureC, setTextureC, "third texture")

// --- public --------------------------------------------------------------------------------------

Xyz::Xyz(const TTexturePtr& a, const TTexturePtr& b, const TTexturePtr& c):
	a_(a),
	b_(b),
	c_(c)
{
}



void Xyz::setTextureA(const TTexturePtr& a)
{
	a_ = a;
}



void Xyz::setTextureB(const TTexturePtr& b)
{
	b_ = b;
}



void Xyz::setTextureC(const TTexturePtr& c)
{
	c_ = c;
}



const TTexturePtr& Xyz::textureA() const
{
	return a_;
}



const TTexturePtr& Xyz::textureB() const
{
	return b_;
}



const TTexturePtr& Xyz::textureC() const
{
	return c_;
}



// --- protected -----------------------------------------------------------------------------------

const TPyObjectPtr Xyz::doGetState() const
{
	return python::makeTuple(a_, b_, c_);
}



void Xyz::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, a_, b_, c_);
}



bool Xyz::doIsChromatic() const
{
	return a_->isChromatic() || b_->isChromatic() || c_->isChromatic();
}



// --- private -------------------------------------------------------------------------------------

const Spectral Xyz::doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const
{
	const TPoint3D& p = context.point();
	const TValue x = num::fractional(static_cast<TValue>(p.x));
	const TValue y = num::fractional(static_cast<TValue>(p.y));
	const TValue z = num::fractional(static_cast<TValue>(p.z));
	return Spectral(x * a_->lookUp(sample, context, SpectralType::Illuminant) + y * b_->lookUp(sample, context, SpectralType::Illuminant) + z * c_->lookUp(sample, context, SpectralType::Illuminant), type);
}



Texture::TValue Xyz::doScalarLookUp(const Sample& sample, const IntersectionContext& context) const
{
	const TPoint3D& p = context.point();
	const TValue x = num::fractional(static_cast<TValue>(p.x));
	const TValue y = num::fractional(static_cast<TValue>(p.y));
	const TValue z = num::fractional(static_cast<TValue>(p.z));
	return x * a_->scalarLookUp(sample, context) + y * b_->scalarLookUp(sample, context) + z * c_->scalarLookUp(sample, context);
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
