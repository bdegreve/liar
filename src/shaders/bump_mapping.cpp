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

#include "shaders_common.h"
#include "bump_mapping.h"
#include <lass/num/impl/matrix_solve.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(BumpMapping, "Applies bump mapping to shader")
PY_CLASS_CONSTRUCTOR_2(BumpMapping, const TShaderPtr&, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(BumpMapping, shader, setShader, "mapped shader")
PY_CLASS_MEMBER_RW_DOC(BumpMapping, displacement, setDisplacement, "displacement function")
PY_CLASS_MEMBER_RW_DOC(BumpMapping, scale, setScale, "scale")

// --- public --------------------------------------------------------------------------------------

BumpMapping::BumpMapping(const TShaderPtr& shader, const TTexturePtr& displacement):
	Shader(shader->caps()),
	shader_(shader),
	displacement_(displacement),
	scale_(1)
{
}



const TShaderPtr& BumpMapping::shader() const
{
	return shader_;
}



void BumpMapping::setShader(const TShaderPtr& shader)
{
	shader_ = shader;
	setCaps(shader_->caps());
}



const TTexturePtr& BumpMapping::displacement() const
{
	return displacement_;
}



void BumpMapping::setDisplacement(const TTexturePtr& displacement)
{
	displacement_ = displacement;
}



TScalar BumpMapping::scale() const
{
	return scale_;
}



void BumpMapping::setScale(TScalar scale)
{
	scale_ = scale;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void BumpMapping::doRequestSamples(const TSamplerPtr& sampler)
{
	shader_->requestSamples(sampler);
}



size_t BumpMapping::doNumReflectionSamples() const
{
	return shader_->numReflectionSamples();
}



size_t BumpMapping::doNumTransmissionSamples() const
{
	return shader_->numTransmissionSamples();
}



void BumpMapping::doShadeContext(const Sample& sample, IntersectionContext& context) const
{
	const TScalar d = scale_ * average(displacement_->lookUp(sample, context));
	const TVector2D dD_dUv = dDisplacement_dUv(sample, context);

	const TScalar s = num::sign(dot(cross(context.dPoint_dU(), context.dPoint_dV()), context.normal()));

	const TVector3D dP_dU = context.normal().reject(context.dPoint_dU()) + s * (dD_dUv.x * context.normal() + d * context.dNormal_dU());
	const TVector3D dP_dV = context.normal().reject(context.dPoint_dV()) + s * (dD_dUv.y * context.normal() + d * context.dNormal_dV());
	TVector3D n = cross(dP_dU, dP_dV).normal();

	if (dot(n, context.geometricNormal()) < 0)
	{
		n = -n;
	}

	context.setDPoint_dU(dP_dU);
	context.setDPoint_dV(dP_dV);
	context.setNormal(n);
	//const TTransformation3D localToWorld = context.localToWorld();
	//context.setShader(context.shader()); // reset localToWorld and shaderToWorld;
	//context.transformBy(localToWorld);
}



const XYZ BumpMapping::doEmission(const Sample& sample, const IntersectionContext& context,
		const TVector3D& omegaOut) const
{
	return shader_->emission(sample, context, omegaOut);
}



TBsdfPtr BumpMapping::doBsdf(const Sample& sample, const IntersectionContext& context) const
{
	return shader_->bsdf(sample, context);
}



const TPyObjectPtr BumpMapping::doGetState() const
{
	return python::makeTuple(shader_, displacement_, scale_);
}



void BumpMapping::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, shader_, displacement_, scale_);
}



const TVector2D BumpMapping::dDisplacement_dUv(
		const Sample& sample, IntersectionContext& context) const
{
	const TPoint3D p = context.point();
	const TVector3D n = context.normal();
	const TPoint2D uv = context.uv();

	if (false && context.hasScreenSpaceDifferentials())
	{
		TVector2D dD_dIj;

		context.setPoint(p + .5 * context.dPoint_dI());
		context.setNormal(n + .5 * context.dNormal_dI());
		context.setUv(uv + .5 * context.dUv_dI());
		dD_dIj.x = scale_ * average(displacement_->lookUp(sample, context));
		context.setPoint(p - .5 * context.dPoint_dI());
		context.setNormal(n - .5 * context.dNormal_dI());
		context.setUv(uv - .5 * context.dUv_dI());
		dD_dIj.x -= scale_ * average(displacement_->lookUp(sample, context));

		context.setPoint(p + .5 * context.dPoint_dJ());
		context.setNormal(n + .5 * context.dNormal_dJ());
		context.setUv(uv + .5 * context.dUv_dJ());
		dD_dIj.y = scale_ * average(displacement_->lookUp(sample, context));
		context.setPoint(p - .5 * context.dPoint_dJ());
		context.setNormal(n - .5 * context.dNormal_dJ());
		context.setUv(uv - .5 * context.dUv_dJ());
		dD_dIj.y -= scale_ * average(displacement_->lookUp(sample, context));

		const TScalar dUv_dIj[4] = 
		{
			context.dUv_dI().x, context.dUv_dJ().x, 
			context.dUv_dI().y, context.dUv_dJ().y
		};
		TScalar dIj_dUv[4] = 
		{
			1, 0,
			0, 1
		};
		if (num::impl::cramer2<TScalar>(dUv_dIj, dIj_dUv, dIj_dUv + 4))
		{
			const TVector2D dD_dUv(dIj_dUv[0] * dD_dIj.x + dIj_dUv[1] * dD_dIj.y, dIj_dUv[2] * dD_dIj.x + dIj_dUv[3] * dD_dIj.y);
			return dD_dUv;
		}
	}

	const TScalar epsilon = 1e-3f;
	/*
	const TVector2D dUv = pointwiseMax(
		TVector2D(epsilon, epsilon), 
		.1f * (context.dUv_dI().transform(num::abs) + context.dUv_dJ().transform(num::abs)) / 2);
	/*/
	const TVector2D dUv(epsilon, epsilon);
	/**/
	const TVector2D dUv_2 = dUv / 2;

	TVector2D dD_dUv;

	context.setPoint(p + dUv_2.x * context.dPoint_dU());
	context.setNormal(n + dUv_2.x * context.dNormal_dU());
	context.setUv(uv.x + dUv_2.x, uv.y);
	dD_dUv.x = scale_ * average(displacement_->lookUp(sample, context));
	context.setPoint(p - dUv_2.x * context.dPoint_dU());
	context.setNormal(n - dUv_2.x * context.dNormal_dU());
	context.setUv(uv.x - dUv_2.x, uv.y);
	dD_dUv.x -= scale_ * average(displacement_->lookUp(sample, context));

	context.setPoint(p + dUv_2.y * context.dPoint_dV());
	context.setNormal(n + dUv_2.y * context.dNormal_dV());
	context.setUv(uv.x, uv.y + dUv_2.y);
	dD_dUv.y = scale_ * average(displacement_->lookUp(sample, context));
	context.setPoint(p - dUv_2.y * context.dPoint_dV());
	context.setNormal(n - dUv_2.y * context.dNormal_dV());
	context.setUv(uv.x, uv.y - dUv_2.y);
	dD_dUv.y -= scale_ * average(displacement_->lookUp(sample, context));

	context.setPoint(p);
	context.setNormal(n);
	context.setUv(uv);

	return dD_dUv * dUv.reciprocal();
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
