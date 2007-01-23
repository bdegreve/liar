/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
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
 *  http://liar.sourceforge.net
 */

#include "shaders_common.h"
#include "bump_mapping.h"

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS(BumpMapping)
PY_CLASS_CONSTRUCTOR_2(BumpMapping, const TShaderPtr&, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(BumpMapping, "shader", shader, setShader, "mapped shader")
PY_CLASS_MEMBER_RW_DOC(BumpMapping, "displacement", displacement, setDisplacement, "displacement function")

// --- public --------------------------------------------------------------------------------------

BumpMapping::BumpMapping(const TShaderPtr& shader, const TTexturePtr& displacement):
	Shader(shader->caps()),
	shader_(shader),
	displacement_(displacement)
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



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void BumpMapping::doRequestSamples(const TSamplerPtr& sampler)
{
	shader_->requestSamples(sampler);
}



const unsigned BumpMapping::doNumReflectionSamples() const
{
	return shader_->numReflectionSamples();
}



const unsigned BumpMapping::doNumTransmissionSamples() const
{
	return shader_->numTransmissionSamples();
}



void BumpMapping::doShadeContext(const Sample& sample, IntersectionContext& context) const
{
	const TScalar d = displacement_->lookUp(sample, context).average();
	const TVector2D dD_dUv = dDisplacement_dUv(sample, context);

	const TVector3D dP_dU = context.dPoint_dU() + dD_dUv.x * context.normal() + d * context.dNormal_dU();
	const TVector3D dP_dV = context.dPoint_dV() + dD_dUv.y * context.normal() + d * context.dNormal_dV();
	const TVector3D n = cross(dP_dU, dP_dV).normal();

	context.setDPoint_dU(dP_dU);
	context.setDPoint_dV(dP_dV);
	context.setNormal(n);
	const TTransformation3D localToWorld = context.localToWorld();
	context.setShader(context.shader()); // reset localToWorld and shaderToWorld;
	context.transformBy(localToWorld);
}



const Spectrum BumpMapping::doEmission(const Sample& sample, const IntersectionContext& context,
		const TVector3D& omegaOut) const
{
	return shader_->emission(sample, context, omegaOut);
}



void BumpMapping::doBsdf(
		const Sample& sample, const IntersectionContext& context, const TVector3D& omegaIn,
		const BsdfIn* first, const BsdfIn* last, BsdfOut* result) const
{
	shader_->bsdf(sample, context, omegaIn, first, last, result);
}


void BumpMapping::doSampleBsdf(
		const Sample& sample, const IntersectionContext& context, const TVector3D& omegaIn,
		const SampleBsdfIn* first, const SampleBsdfIn* last, SampleBsdfOut* result) const
{
	shader_->sampleBsdf(sample, context, omegaIn, first, last, result);
}



const TPyObjectPtr BumpMapping::doGetState() const
{
	return python::makeTuple(shader_, displacement_);
}



void BumpMapping::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, shader_, displacement_);
}



const TVector2D BumpMapping::dDisplacement_dUv(
		const Sample& sample, IntersectionContext& context) const
{
	const TPoint3D p = context.point();
	const TVector3D n = context.normal();
	const TPoint2D uv = context.uv();

	const TScalar epsilon = 1e-2f;
	const TVector2D dUv = pointwiseMax(TVector2D(epsilon, epsilon), pointwiseMax(
		context.dUv_dI().transform(num::abs), context.dUv_dJ().transform(num::abs)));
	const TVector2D dUv_2 = dUv / 2;

	TVector2D dD_dUv;

	context.setPoint(p + dUv_2.x * context.dPoint_dU());
	context.setNormal(n + dUv_2.x * context.dNormal_dU());
	context.setUv(uv.x + dUv_2.x, uv.y);
	dD_dUv.x = displacement_->lookUp(sample, context).average();

	context.setPoint(p - dUv_2.x * context.dPoint_dU());
	context.setNormal(n - dUv_2.x * context.dNormal_dU());
	context.setUv(uv.x - dUv_2.x, uv.y);
	dD_dUv.x -= displacement_->lookUp(sample, context).average();

	context.setPoint(p + dUv_2.y * context.dPoint_dV());
	context.setNormal(n + dUv_2.y * context.dNormal_dV());
	context.setUv(uv.x, uv.y + dUv_2.y);
	dD_dUv.y = displacement_->lookUp(sample, context).average();;

	context.setPoint(p - dUv_2.y * context.dPoint_dV());
	context.setNormal(n - dUv_2.y * context.dNormal_dV());
	context.setUv(uv.x, uv.y - dUv_2.y);
	dD_dUv.y -= displacement_->lookUp(sample, context).average();

	context.setPoint(p);
	context.setNormal(n);
	context.setUv(uv);

	return dD_dUv * dUv.reciprocal();
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
