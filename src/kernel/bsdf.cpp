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

#include "kernel_common.h"
#include "bsdf.h"
#include "intersection_context.h"
#include "shader.h"

namespace liar
{
namespace kernel
{

// --- public --------------------------------------------------------------------------------------

Bsdf::Bsdf(const Sample& sample, const IntersectionContext& context, TBsdfCaps caps):
	omegaGeometricNormal_(prim::normalTransform(context.geometricNormal(), context.localToBsdf())),
	sample_(sample),
	context_(context),
	caps_(caps)
{
}



Bsdf::~Bsdf()
{
}



BsdfOut Bsdf::evaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, TBsdfCaps allowedCaps) const
{
	// for reflection, omegaIn and omegaOut must lay in the same hemisphere determined by geometric normal
	const bool reflective = (dot(omegaIn, omegaGeometricNormal_) > 0) == (dot(omegaOut, omegaGeometricNormal_) > 0);
	util::clearMasked<TBsdfCaps>(allowedCaps, reflective ? capsTransmission : capsReflection);

	if (!compatibleCaps(allowedCaps))
	{
		return BsdfOut();
	}
	return doEvaluate(omegaIn, omegaOut, allowedCaps);
}



SampleBsdfOut Bsdf::sample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, TBsdfCaps allowedCaps) const
{
	if (!compatibleCaps(allowedCaps))
	{
		return SampleBsdfOut();
	}
	return doSample(omegaIn, sample, componentSample, allowedCaps);
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



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
