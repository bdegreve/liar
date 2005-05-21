/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2005  Bram de Greve
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "tracers_common.h"
#include "direct_lighting.h"

namespace liar
{
namespace tracers
{

PY_DECLARE_CLASS(DirectLighting)
PY_CLASS_CONSTRUCTOR_0(DirectLighting)

// --- public --------------------------------------------------------------------------------------

DirectLighting::DirectLighting():
	RayTracer(&Type)
{
}



// --- protected -----------------------------------------------------------------------------------

// --- private -------------------------------------------------------------------------------------

void DirectLighting::doPreprocess()
{
}



void DirectLighting::doRequestSamples(const kernel::TSamplerPtr& iSampler)
{
}



TSpectrum DirectLighting::doCastRay(const kernel::DifferentialRay& iPrimaryRay, 
									const kernel::Sample& iSample) const
{
	kernel::Intersection intersection;
	scene()->intersect(iPrimaryRay.ray(), intersection);
	if (!intersection)
	{
		return TSpectrum();
	}

	kernel::IntersectionContext context;
	intersection.object()->localContext(iPrimaryRay, intersection, context);
	kernel::TShaderPtr shader = context.shader();

	TSpectrum result;
	const kernel::TLightContexts::const_iterator end = lights().end();
	for (kernel::TLightContexts::const_iterator i = lights().begin(); i != end; ++i)
	{
		result += shader->directLight(iPrimaryRay, intersection, context, iSample, scene(), *i);
	}

	return result;
}




// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
