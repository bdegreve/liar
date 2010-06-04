/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
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
#include "mirror.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(Mirror, "perfect mirror shader")
PY_CLASS_CONSTRUCTOR_0(Mirror)
PY_CLASS_CONSTRUCTOR_1(Mirror, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(Mirror, reflectance, setReflectance, "texture for reflectance component")

// --- public --------------------------------------------------------------------------------------

Mirror::Mirror():
	Shader(Bsdf::capsReflection | Bsdf::capsSpecular),
	reflectance_(Texture::white())
{
}



Mirror::Mirror(const TTexturePtr& reflectance):
	Shader(Bsdf::capsReflection | Bsdf::capsSpecular),
	reflectance_(reflectance)
{
}



const TTexturePtr& Mirror::reflectance() const
{
	return reflectance_;
}



void Mirror::setReflectance(const TTexturePtr& reflectance)
{
	reflectance_ = reflectance;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

size_t Mirror::doNumReflectionSamples() const
{
	return 1;
}



void Mirror::doBsdf(const Sample&, const IntersectionContext&, const TVector3D&, const BsdfIn*, const BsdfIn*, BsdfOut*) const
{
}



void Mirror::doSampleBsdf(
		const Sample& sample, const IntersectionContext& context, const TVector3D& omegaIn,
		const SampleBsdfIn* first, const SampleBsdfIn* last, SampleBsdfOut* result) const
{
	const XYZ r = reflectance_->lookUp(sample, context);
	const TVector3D omegaOut(-omegaIn.x, -omegaIn.y, omegaIn.z);
	while (first != last)
	{
		if (compatibleCaps(first->allowedCaps))
		{
			result->omegaOut = omegaOut;
			result->value = r;
			result->pdf = omegaOut.z;
			result->usedCaps = caps();
		}
		++first;
		++result;
	}
}



const TPyObjectPtr Mirror::doGetState() const
{
	return python::makeTuple(reflectance_);
}



void Mirror::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, reflectance_);
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
