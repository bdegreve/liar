/** @file
*  @author Bram de Greve (bramz@users.sourceforge.net)
*
*  LiAR isn't a raytracer
*  Copyright (C) 2004-2024  Bram de Greve (bramz@users.sourceforge.net)
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

#include "scenery_common.h"
#include "light_projection.h"
#include "../kernel/projection_perspective.h"
#include <lass/num/distribution_transformations.h>
#include <lass/prim/impl/plane_3d_impl_detail.h>

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_DOC(LightProjection, "spot light")
PY_CLASS_CONSTRUCTOR_0(LightProjection)
	PY_CLASS_MEMBER_RW(LightProjection, intensity, setIntensity)
	PY_CLASS_MEMBER_RW(LightProjection, projection, setProjection)
	PY_CLASS_MEMBER_RW(LightProjection, attenuation, setAttenuation)
	PY_CLASS_MEMBER_RW(LightProjection, samplingResolution, setSamplingResolution)

// --- public --------------------------------------------------------------------------------------

LightProjection::LightProjection():
	intensity_(Texture::white()),
	projection_(new ProjectionPerspective()),
	attenuation_(Attenuation::defaultAttenuation())
{
	setSamplingResolution(TResolution2D(256, 256));
}


const TTexturePtr& LightProjection::intensity() const
{
	return intensity_;
}


void LightProjection::setIntensity(const TTexturePtr& iIntensity)
{
	intensity_ = iIntensity;
}


const TProjectionPtr& LightProjection::projection() const
{
	return projection_;
}


void LightProjection::setProjection(const TProjectionPtr& projection)
{
	projection_ = projection;
}


const TAttenuationPtr& LightProjection::attenuation() const
{
	return attenuation_;
}


void LightProjection::setAttenuation(const TAttenuationPtr& iAttenuation)
{
	attenuation_ = iAttenuation;
}


const TResolution2D& LightProjection::samplingResolution() const
{
	return resolution_;
}


void LightProjection::setSamplingResolution(const TResolution2D& resolution)
{
	resolution_ = resolution;
	invResolution_.x = num::inv(static_cast<TScalar>(std::max<size_t>(resolution_.x, 1)));
	invResolution_.y = num::inv(static_cast<TScalar>(std::max<size_t>(resolution_.y, 1)));
}


// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void LightProjection::doPreProcess(const TimePeriod&)
{
	util::ProgressIndicator progress("Preprocessing projection map");
	TMap pdf;
	buildPdf(pdf, power_, progress);
	buildCdf(pdf, marginalCdfU_, conditionalCdfV_, progress);
}



void LightProjection::doIntersect(const Sample&, const BoundedRay&, Intersection& result) const
{
	result = Intersection::empty();
}



bool LightProjection::doIsIntersecting(const Sample&, const BoundedRay&) const
{
	return false;
}



void LightProjection::doLocalContext(const Sample&, const BoundedRay& ray, const Intersection&, IntersectionContext& result) const
{
	TRay3D r;
	TScalar t;
	result.setUv(projection_->uv(ray.point(1), r, t));
}



bool LightProjection::doContains(const Sample&, const TPoint3D&) const
{
	return false;
}



const TAabb3D LightProjection::doBoundingBox() const
{
	return TAabb3D();
}



TScalar LightProjection::doArea() const
{
	return 0;
}



TScalar LightProjection::doArea(const TVector3D&) const
{
	return 0;
}


const Spectral LightProjection::doSampleEmission(const Sample& sample, const TPoint2D&, const TPoint3D& target, BoundedRay& shadowRay, TScalar& pdf) const
{
	TRay3D ray;
	TScalar t;
	const Projection::TUv uv = projection_->uv(target, ray, t);
	if (t <= 0 || uv.x < 0 || uv.y < 0 || uv.x > 1 || uv.y > 1)
	{
		pdf = TNumTraits::zero;
		return Spectral();
	}

	const TScalar distance = ray.t(target);
	shadowRay = BoundedRay(target, -ray.direction(), tolerance, distance, prim::IsAlreadyNormalized());
	pdf = TNumTraits::one;

	Intersection intersection(this, t, seNoEvent);
	IntersectionContext context(*this, sample, ray, intersection, 0);
	return intensity_->lookUp(sample, context, SpectralType::Illuminant) /
		static_cast<Spectral::TValue>(attenuation_->attenuation(distance, num::sqr(distance)));
}



const Spectral LightProjection::doEmission(const Sample&, const TRay3D& ray, BoundedRay& shadowRay, TScalar& pdf) const
{
	shadowRay = ray;
	pdf = 0;
	return Spectral(0);
}



const Spectral LightProjection::doSampleEmission(const Sample& sample, const TPoint2D& lightSampleA, const TPoint2D&, BoundedRay& emissionRay, TScalar& pdf) const
{
	TScalar u, v, pdfA;
	sampleMap(lightSampleA, u, v, pdfA);

	TScalar pdfB;
	const TRay3D ray = projection_->ray(u, v, pdfB);
	emissionRay = BoundedRay(ray, tolerance);

	pdf = (pdfA * static_cast<TScalar>(resolution_.x * resolution_.y)) * pdfB;

	Intersection intersection(this, 1, seNoEvent);
	IntersectionContext context(*this, sample, ray, intersection, 0);
	return intensity_->lookUp(sample, context, SpectralType::Illuminant);
}



TScalar LightProjection::doTotalPower() const
{
	return power_;
}


size_t LightProjection::doNumberOfEmissionSamples() const
{
	return 1;
}



bool LightProjection::doIsSingular() const
{
	return true;
}



const TPyObjectPtr LightProjection::doGetLightState() const
{
	return python::makeTuple(intensity_, projection_, attenuation_, resolution_);
}



void LightProjection::doSetLightState(const TPyObjectPtr& state)
{
	TResolution2D res;
	python::decodeTuple(state, intensity_, projection_, attenuation_, res);
	setSamplingResolution(res);
}



void LightProjection::buildPdf(TMap& pdfMap, TScalar& power, util::ProgressIndicator& progress) const
{
	Sample dummy;

	const size_t n = resolution_.x * resolution_.y;
	TMap tempPdf(n);
	TScalar totalPower = 0;
	for (size_t i = 0; i < resolution_.x; ++i)
	{
		const TScalar u = (static_cast<TScalar>(i) + .5f) * invResolution_.x;
		progress(static_cast<double>(.5 * u));
		for (size_t j = 0; j < resolution_.y; ++j)
		{
			const TScalar v = (static_cast<TScalar>(j) +.5f) * invResolution_.y;

			TScalar pdf;
			const BoundedRay centralRay = projection_->ray(u, v, pdf);
			const TRay3D differentialI = projection_->ray(u + invResolution_.x, v);
			const TRay3D differentialJ = projection_->ray(u, v + invResolution_.y);
			const DifferentialRay ray(centralRay, differentialI, differentialJ);

			Intersection intersection(this, 1, seNoEvent);
			IntersectionContext context(*this, dummy, ray, intersection, 0);
			const TScalar intensity = intensity_->scalarLookUp(dummy, context);

			tempPdf[i * resolution_.y + j] = intensity;
			totalPower += intensity / (pdf * static_cast<TScalar>(n));
		}
	}

	pdfMap.swap(tempPdf);
	power = totalPower;
}



void LightProjection::buildCdf(const TMap& pdf, TMap& oMarginalCdfU, TMap& oConditionalCdfV, util::ProgressIndicator& progress) const
{
	TMap marginalPdfU(resolution_.x);
	TMap marginalCdfU(resolution_.x);
	TMap conditionalCdfV(resolution_.x * resolution_.y);

	for (size_t i = 0; i < resolution_.x; ++i)
	{
		progress(.5 + .5 * static_cast<double>(i) / static_cast<double>(resolution_.x));
		const TScalar* pdfLine = &pdf[i * resolution_.y];
		TMap::iterator condCdfV = conditionalCdfV.begin() + i * resolution_.y;
		std::partial_sum(pdfLine, pdfLine + resolution_.y, condCdfV);

		marginalPdfU[i] = condCdfV[resolution_.y - 1];
		std::transform(condCdfV, condCdfV + resolution_.y, condCdfV, [d=marginalPdfU[i]](TScalar x) { return x / d; });

	}

	std::partial_sum(marginalPdfU.begin(), marginalPdfU.end(), marginalCdfU.begin());
	std::transform(marginalCdfU.begin(), marginalCdfU.end(), marginalCdfU.begin(), [d=marginalCdfU.back()](TScalar x) { return x / d; });

	oMarginalCdfU.swap(marginalCdfU);
	oConditionalCdfV.swap(conditionalCdfV);
}



void LightProjection::sampleMap(const TPoint2D& sample, TScalar& u, TScalar& v, TScalar& pdf) const
{
	const size_t i = std::min(resolution_.x - 1, static_cast<size_t>(
		std::lower_bound(marginalCdfU_.begin(), marginalCdfU_.end(), sample.x) - marginalCdfU_.begin()));
	const TScalar x0 = i > 0 ? marginalCdfU_[i - 1] : TNumTraits::zero;
	const TScalar margPdfU = marginalCdfU_[i] - x0;
	u = invResolution_.x * (static_cast<TScalar>(i) + (sample.x - x0) / margPdfU);

	TMap::const_iterator condCdfV = conditionalCdfV_.begin() + i * resolution_.y;
	const size_t j = std::min(resolution_.y - 1, static_cast<size_t>(
		std::lower_bound(condCdfV, condCdfV + resolution_.y, sample.y) - condCdfV));
	const TScalar y0 = j > 0 ? condCdfV[j - 1] : TNumTraits::zero;
	const TScalar condPdfV = condCdfV[j] - y0;
	v = invResolution_.y * (static_cast<TScalar>(j) + (sample.y - y0) / condPdfV);

	pdf = margPdfU * condPdfV;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
