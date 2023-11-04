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
#include "bsdf.h"
#include "intersection_context.h"
#include "shader.h"

namespace liar
{
namespace kernel
{

// --- public --------------------------------------------------------------------------------------

Bsdf::Bsdf(const Sample& sample, const IntersectionContext& context, BsdfCaps caps):
	omegaGeometricNormal_(prim::normalTransform(context.geometricNormal(), context.localToBsdf())),
	sample_(sample),
	context_(context),
	caps_(caps)
{
}



Bsdf::~Bsdf()
{
}



BsdfOut Bsdf::evaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, BsdfCaps allowedCaps) const
{
	LIAR_ASSERT_NORMALIZED(omegaIn);
	LIAR_ASSERT_NORMALIZED(omegaOut);

	// for reflection, omegaIn and omegaOut must lay in the same hemisphere determined by geometric normal
	const bool reflective = (dot(omegaIn, omegaGeometricNormal_) > 0) == (dot(omegaOut, omegaGeometricNormal_) > 0);
	util::clearMasked<BsdfCaps>(allowedCaps, reflective ? BsdfCaps::transmission : BsdfCaps::reflection);

	if (!compatibleCaps(allowedCaps))
	{
		return BsdfOut();
	}
	const BsdfOut out = doEvaluate(omegaIn, omegaOut, allowedCaps);

	LIAR_ASSERT(isFinite(out.value), out.value << " from " << typeid(*this).name());
	LIAR_ASSERT(isPositiveAndFinite(out.pdf), out.pdf << " from " << typeid(*this).name());
	return out;
}



SampleBsdfOut Bsdf::sample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, BsdfCaps allowedCaps) const
{
	LIAR_ASSERT_NORMALIZED(omegaIn);

	if (!compatibleCaps(allowedCaps))
	{
		return SampleBsdfOut();
	}
	const SampleBsdfOut out = doSample(omegaIn, sample, componentSample, allowedCaps);

	LIAR_ASSERT(isFinite(out.value), out.value << " from " << typeid(*this).name());
	LIAR_ASSERT(isPositiveAndFinite(out.pdf), out.pdf << " from " << typeid(*this).name());
	LIAR_ASSERT(!(out.pdf > 0 && !isNormalized(out.omegaOut)), out.omegaOut << " (out.pdf=" << out.pdf << ") from " << typeid(*this).name());
	return out;
}


bool Bsdf::isDispersive() const
{
	return doIsDispersive();
}


const TVector3D Bsdf::bsdfToWorld(const TVector3D& v) const
{
	return context_.bsdfToWorld(v);
}



const TVector3D Bsdf::worldToBsdf(const TVector3D& v) const
{
	return context_.worldToBsdf(v);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

bool Bsdf::doIsDispersive() const
{
	return false;
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
