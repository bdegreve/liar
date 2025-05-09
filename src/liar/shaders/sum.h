/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2025  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::shaders::Sum
 *  @brief simple addition of multiple shaders
 *  @author Bram de Greve [Bramz]
 *
 *	@warning NOT THREAD SAFE, but thread safety is not a requirement for bsdfs.
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SHADERS_SUM_H
#define LIAR_GUARDIAN_OF_INCLUSION_SHADERS_SUM_H

#include "shaders_common.h"
#include "../kernel/shader.h"
#include <lass/stde/static_vector.h>

namespace liar
{
namespace shaders
{

class LIAR_SHADERS_DLL Sum: public Shader
{
	PY_HEADER(Shader)
public:

	enum { capacity = 8 };

	typedef stde::static_vector<TShaderRef, capacity> TChildren;

	Sum(const TChildren& children);

private:

	class SumBsdf: public Bsdf
	{
	public:
		typedef stde::static_vector<TBsdfPtr, capacity> TComponents;
		SumBsdf(const Sample& sample, const IntersectionContext& context, BsdfCaps caps);
	private:
		BsdfOut doEvaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, BsdfCaps allowedCaps) const override;
		SampleBsdfOut doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, BsdfCaps allowedCaps) const override;
		TComponents components_;
		mutable stde::static_vector<const Bsdf*, capacity> activeComponents_;
		//BsdfCaps caps_;
		mutable BsdfCaps activeCaps_;
		friend class Sum;
	};

	const Spectral doEmission(const Sample& sample, const IntersectionContext& context, const TVector3D& omegaOut) const override;
	TBsdfPtr doBsdf(const Sample& sample, const IntersectionContext& context) const override;

	void doRequestSamples(const TSamplerPtr& sampler) override;
	size_t doNumReflectionSamples() const override;
	size_t doNumTransmissionSamples() const override;

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

	TChildren children_;
};

}

}

#endif

// EOF
