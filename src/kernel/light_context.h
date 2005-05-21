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

/** @class liar::kernel::LightContext
 *  @brief unfolded context for a light
 *  @author Bram de Greve [BdG]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_LIGHT_CONTEXT_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_LIGHT_CONTEXT_H

#pragma once
#include "kernel_common.h"
#include "scene_light.h"
#include "sampler.h"

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL LightContext
{
public:

	LightContext(const TSceneLightPtr& iLight, const TTransformation3D& iLocalToWorld);

	const TSceneLightPtr& light() const { return light_; }

	const int idLightSamples() const { return idLightSamples_; }
	const int idBsdfSamples() const { return idBsdfSamples_; }
	const int idBsdfComponentSamples() const { return idBsdfComponentSamples_; }

    void requestSamples(const TSamplerPtr& iSampler);

	const TSpectrum sampleRadiance(const TVector2D& iSample, const TPoint3D& iPoint, 
		TRay3D& oShadowRay, TScalar& oMaxT) const
	{
		const TPoint3D localPoint = transform(iPoint, localToWorld_.inverse());
		TRay3D localRay;
		const TSpectrum radiance = light_->sampleRadiance(iSample, localPoint, localRay, oMaxT);
		oShadowRay = TRay3D(iPoint, transform(localRay.direction(), localToWorld_));
		return radiance;
	}

private:

	TTransformation3D localToWorld_;
	TSceneLightPtr light_;
	int idLightSamples_;
	int idBsdfSamples_;
	int idBsdfComponentSamples_;
};

typedef std::vector<LightContext> TLightContexts;

TLightContexts gatherLightContexts(const TSceneObjectPtr& iSceneObject);

}

}

#endif

// EOF
