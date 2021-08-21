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

#include "kernel_common.h"
#include "texture.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(Texture, "Abstract base class of textures")
PY_CLASS_STATIC_METHOD(Texture, black)
PY_CLASS_STATIC_METHOD(Texture, white)
PY_CLASS_METHOD_NAME(Texture, reduce, "__reduce__")
PY_CLASS_METHOD_NAME(Texture, getState, "__getstate__")
PY_CLASS_METHOD_NAME(Texture, setState, "__setstate__")

typedef impl::TextureBlack TTextureBlack;
PY_DECLARE_CLASS_NAME(TTextureBlack, "Black");
PY_CLASS_INNER_CLASS_NAME(Texture, TTextureBlack, "Black");
TTexturePtr Texture::black_(new TTextureBlack());

typedef impl::TextureWhite TTextureWhite;
PY_DECLARE_CLASS_NAME(TTextureWhite, "White");
PY_CLASS_INNER_CLASS_NAME(Texture, TTextureWhite, "White");
TTexturePtr Texture::white_(new TTextureWhite);

// --- public --------------------------------------------------------------------------------------

Texture::~Texture()
{
}



bool Texture::isChromatic() const
{
	return doIsChromatic();
}


const TTexturePtr& Texture::black()
{
	return black_;
}



const TTexturePtr& Texture::white()
{
	return white_;
}



const TPyObjectPtr Texture::reduce() const
{
	return python::makeTuple(
		python::fromNakedToSharedPtrCast<PyObject>(reinterpret_cast<PyObject*>(this->_lassPyGetClassDef()->type())), 
		python::makeTuple(), this->getState());
}



const TPyObjectPtr Texture::getState() const
{
	return doGetState();
}



void Texture::setState(const TPyObjectPtr& state)
{
	doSetState(state);
}



// --- protected -----------------------------------------------------------------------------------

Texture::Texture()
{
}



// --- private -------------------------------------------------------------------------------------

Texture::TValue Texture::doScalarLookUp(const Sample& sample, const IntersectionContext& context) const
{
	return lookUp(sample, context, SpectralType::Illuminant).luminance(sample);
}


// --- free ----------------------------------------------------------------------------------------



namespace impl
{

TextureBlack::TextureBlack()
{
}

const Spectral TextureBlack::doLookUp(const Sample&, const IntersectionContext&, SpectralType) const
{
	return Spectral(0);
}

Texture::TValue TextureBlack::doScalarLookUp(const Sample&, const IntersectionContext&) const
{
	return 0;
}

bool TextureBlack::doIsChromatic() const
{
	return false;
}

const TPyObjectPtr TextureBlack::doGetState() const
{
	return python::makeTuple();
}

void TextureBlack::doSetState(const TPyObjectPtr&)
{
}


TextureWhite::TextureWhite()
{
}

const Spectral TextureWhite::doLookUp(const Sample&, const IntersectionContext&, SpectralType) const
{
	return Spectral(1);
}

Texture::TValue TextureWhite::doScalarLookUp(const Sample&, const IntersectionContext&) const
{
	return 1;
}

bool TextureWhite::doIsChromatic() const
{
	return false;
}

const TPyObjectPtr TextureWhite::doGetState() const
{
	return python::makeTuple();
}

void TextureWhite::doSetState(const TPyObjectPtr&)
{
}

}

}

}

// EOF
