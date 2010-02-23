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

/** @class liar::LightContext
 *  @brief unfolded context for a light
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_LIGHT_CONTEXT_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_LIGHT_CONTEXT_H

#include "kernel_common.h"
#include "scene_object.h"
#include "sampler.h"
#include "scene_light.h"
#include <lass/util/thread.h>

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL LightContext
{
public:

	typedef std::vector<TSceneObjectPtr> TObjectPath;

	LightContext(const TObjectPath& objectPathToLight, const SceneLight& light);

	void setSceneBound(const TAabb3D& bound, const TimePeriod& period);

	const TObjectPath& objectPath() const { return objectPath_; }
	const SceneLight& light() const { return *light_; }

	int idLightSamples() const { return idLightSamples_; }
	int idBsdfSamples() const { return idBsdfSamples_; }
	int idBsdfComponentSamples() const { return idBsdfComponentSamples_; }

	void requestSamples(const TSamplerPtr& sampler);

	const XYZ emission(const Sample& cameraSample, const TRay3D& ray, BoundedRay& shadowRay, TScalar& pdf) const;
	const XYZ sampleEmission(
			const Sample& cameraSample, const TPoint2D& sample, const TPoint3D& target, 
			const TVector3D& targetNormal, BoundedRay& shadowRay, TScalar& pdf) const;
	const XYZ sampleEmission(
			const Sample& cameraSample, const TPoint2D& lightSampleA, const TPoint2D& lightSampleB, 
			BoundedRay& emissionRay, TScalar& pdf) const;

	const XYZ totalPower() const;
	bool isSingular() const;

private:

	void setTime(TTime time) const;

	TAabb3D localBound_;
	mutable TTransformation3D localToWorld_;	/**< concatenated local to world transformation */
	mutable TTime timeOfTransformation_;		/**< time localToWorld_ was calculated for */
	TObjectPath objectPath_;					/**< path in object tree to light (light included) */
	const SceneLight* light_;					/**< pointer to actual light object */
	int idLightSamples_;			
	int idBsdfSamples_;
	int idBsdfComponentSamples_;
	bool hasMotion_;							/**< does light move in time? */

	static util::CriticalSection lock_;
};

typedef std::vector<LightContext> TLightContexts;

TLightContexts gatherLightContexts(const TSceneObjectPtr& scene);

}

}

#endif

// EOF
