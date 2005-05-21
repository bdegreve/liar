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
 *
 *  http://liar.sourceforge.net
 */

#include "kernel_common.h"
#include "ray_tracer.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS(RayTracer)

// --- public --------------------------------------------------------------------------------------

RayTracer::~RayTracer()
{
}



const TSceneObjectPtr& RayTracer::scene() const
{
    return scene_;
}



void RayTracer::setScene(const TSceneObjectPtr& iScene)
{
    scene_ = iScene;
	lights_ = gatherLightContexts(iScene);
    doPreprocess();
}



void RayTracer::requestSamples(const TSamplerPtr& iSampler) 
{ 
	if (scene_ && iSampler)
	{
		iSampler->clearSubSequenceRequests();
		for (TLightContexts::iterator i = lights_.begin(); i != lights_.end(); ++i)
		{
			i->requestSamples(iSampler);
		}
		doRequestSamples(iSampler); 
	}
}



// --- protected -----------------------------------------------------------------------------------

RayTracer::RayTracer(PyTypeObject* iType):
    PyObjectPlus(iType)
{
}



// --- private -------------------------------------------------------------------------------------



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF