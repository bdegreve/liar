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

#include "textures_common.h"
#include "sky_blend.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(SkyBlend, "mixes two textures in 2D checkerboard pattern")
PY_CLASS_CONSTRUCTOR_2(SkyBlend, TTexturePtr, TTexturePtr);
PY_CLASS_MEMBER_RW(SkyBlend, zenith, setZenith)

// --- public --------------------------------------------------------------------------------------

SkyBlend::SkyBlend(const TTexturePtr& a, const TTexturePtr& b) :
    BinaryOperator(a, b),
    zenith_(0, 0, 1)
{
}


const TVector3D& SkyBlend::zenith() const
{
    return zenith_;
}


void SkyBlend::setZenith(const TVector3D& zenith)
{
    zenith_ = zenith.normal();
}



// --- protected -----------------------------------------------------------------------------------

const TPyObjectPtr SkyBlend::doGetState() const
{
    return python::makeTuple(BinaryOperator::doGetState(), zenith_);
}



void SkyBlend::doSetState(const TPyObjectPtr& state)
{
    TPyObjectPtr parentState;
    python::decodeTuple(state, parentState, zenith_);
    BinaryOperator::doSetState(parentState);
}



// --- private -------------------------------------------------------------------------------------

const Spectral SkyBlend::doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const
{
    const TValue wA = weightA(context);
    if (wA >= 1)
    {
        return textureA()->lookUp(sample, context, type);
    }
    else if (wA <= 0)
    {
        return textureB()->lookUp(sample, context, type);
    }
    else
    {
        return wA * textureA()->lookUp(sample, context, type) + (1 - wA) * textureB()->lookUp(sample, context, type);
    }
}


Texture::TValue SkyBlend::doScalarLookUp(const Sample& sample, const IntersectionContext& context) const
{
    const TValue wA = weightA(context);
    if (wA >= 1)
    {
        return textureA()->scalarLookUp(sample, context);
    }
    else if (wA <= 0)
    {
        return textureB()->scalarLookUp(sample, context);
    }
    else
    {
        return wA * textureA()->scalarLookUp(sample, context) + (1 - wA) * textureB()->scalarLookUp(sample, context);
    }
}


Texture::TValue SkyBlend::weightA(const IntersectionContext& context) const
{
    const TVector3D dir = context.point().position();
    const TVector3D::TValue t = dot(dir, zenith_) / dir.norm();
    return static_cast<TValue>((t + 1) / 2);
}


}

}

// EOF

