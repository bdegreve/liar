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

#include "kernel_common.h"
#include "bsdf.h"
#include "intersection_context.h"
#include "shader.h"

namespace liar
{
namespace kernel
{

// --- public --------------------------------------------------------------------------------------

Bsdf::Bsdf(const Sample& sample, const IntersectionContext& context):
	sample_(sample),
	context_(context)
{
}



Bsdf::~Bsdf()
{
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

/** fall back for old shaders
 */
BsdfOut Bsdf::doCall(const TVector3D& omegaIn, const TVector3D& omegaOut, unsigned allowedCaps) const
{
	LASS_ASSERT(context_.shader());
	BsdfIn in;
	in.omegaOut = omegaOut;
	in.allowedCaps = allowedCaps;
	BsdfOut out;
	context_.shader()->bsdf(sample_, context_, omegaIn, &in, &in + 1, &out);
	return out;
}


/** fall back for old shaders
 */
SampleBsdfOut Bsdf::doSample(const TVector3D& omegaIn, const TPoint2D& sample, unsigned allowedCaps) const
{
	LASS_ASSERT(context_.shader());
	SampleBsdfIn in;
	in.sample = sample;
	in.allowedCaps = allowedCaps;
	SampleBsdfOut out;
	context_.shader()->sampleBsdf(sample_, context_, omegaIn, &in, &in + 1, &out);
	return out;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
