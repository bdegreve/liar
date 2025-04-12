/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2023-2025  Bram de Greve (bramz@users.sourceforge.net)
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
#include "alpha_channel.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(AlphaChannel, "image file alpha channel")
PY_CLASS_CONSTRUCTOR_1(AlphaChannel, const TImageRef&);
PY_CLASS_MEMBER_RW(AlphaChannel, image, setImage);

// --- public --------------------------------------------------------------------------------------

AlphaChannel::AlphaChannel(const TImageRef& image):
	image_(image)
{
}



const TImageRef& AlphaChannel::image() const
{
	return image_;
}



void AlphaChannel::setImage(const TImageRef& image)
{
	image_ = image;
}


// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const Spectral AlphaChannel::doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const
{
	return Spectral(doScalarLookUp(sample, context), type);
}


Texture::TValue AlphaChannel::doScalarLookUp(const Sample&, const IntersectionContext& context) const
{
	return image_->lookUp(context).a;
}


bool AlphaChannel::doIsChromatic() const
{
	return false;
}



const TPyObjectPtr AlphaChannel::doGetState() const
{
	return python::makeTuple(image_);
}



void AlphaChannel::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, image_);
}


// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
