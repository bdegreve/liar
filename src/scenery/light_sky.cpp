/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2021  Bram de Greve (bramz@users.sourceforge.net)
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
#include "light_sky.h"
#include "sphere.h"
#include "../kernel/rgb_space.h"

#include <lass/num/distribution_transformations.h>

#if LASS_COMPILER_TYPE == LASS_COMPILER_TYPE_MSVC
#	pragma warning(disable: 4996) // std::copy may be unsafe
#endif

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_DOC(LightSky, "infinite sky light")
PY_CLASS_CONSTRUCTOR_0(LightSky)
PY_CLASS_CONSTRUCTOR_1(LightSky, const TTexturePtr&)
PY_CLASS_MEMBER_RW(LightSky, radiance, setRadiance)
PY_CLASS_MEMBER_RW(LightSky, portal, setPortal)
PY_CLASS_MEMBER_RW(LightSky, numberOfEmissionSamples, setNumberOfEmissionSamples)
PY_CLASS_MEMBER_RW(LightSky, samplingResolution, setSamplingResolution)


// --- public --------------------------------------------------------------------------------------

LightSky::LightSky()
{
	init(Texture::white());
}



LightSky::LightSky(const TTexturePtr& radiance)
{
	init(radiance);
}



const TTexturePtr& LightSky::radiance() const
{
	return radiance_;
}



const TSceneObjectPtr& LightSky::portal() const
{
	return portal_;
}



const TResolution2D& LightSky::samplingResolution() const
{
	return resolution_;
}



void LightSky::setRadiance(const TTexturePtr& radiance)
{
	radiance_ = radiance;
}



void LightSky::setPortal(const TSceneObjectPtr& portal)
{
	if (portal && !portal->hasSurfaceSampling())
	{
		throw python::PythonException(PyExc_TypeError, "portal must support surface sampling", LASS_PRETTY_FUNCTION);
	}
	portal_ = portal;
}



void LightSky::setNumberOfEmissionSamples(unsigned number)
{
	numberOfSamples_ = number;
}




void LightSky::setSamplingResolution(const TResolution2D& resolution)
{
	resolution_ = resolution;
	invResolution_.x = num::inv(static_cast<TScalar>(std::max<size_t>(resolution_.x, 1)));
	invResolution_.y = num::inv(static_cast<TScalar>(std::max<size_t>(resolution_.y, 1)));
}




// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void LightSky::doPreProcess(const TSceneObjectPtr& scene, const TimePeriod&)
{
	/*if (!portal_)
	{
		portal_.reset(new Sphere(boundingSphere(scene->boundingBox())));
	}*/
	
	sceneBounds_ = boundingSphere(scene->boundingBox());

	// intersections with sky are always this distance removed from any point.
	const TScalar radius = sceneBounds_.radius();
	fixedDistance_ = radius > 0 ? 1e3 * radius : 1e3;

	util::ProgressIndicator progress("Preprocessing environment map");
	TMap pdf;
	buildPdf(pdf, power_, progress);
	buildCdf(pdf, marginalCdfU_, conditionalCdfV_, progress);
}



void LightSky::doIntersect(const Sample&, const BoundedRay& ray, Intersection& result) const
{
	if (ray.inRange(fixedDistance_))
	{
		result = Intersection(this, fixedDistance_, seLeaving);
	}
	else
	{
		result = Intersection::empty();
	}
}



bool LightSky::doIsIntersecting(const Sample&, const BoundedRay& ray) const
{
	return ray.inRange(fixedDistance_);
}



void LightSky::doLocalContext(const Sample&, const BoundedRay& ray, const Intersection& LASS_UNUSED(intersection), IntersectionContext& result) const
{
	//LASS_ASSERT(intersection.t() == diameter_);
	result.setT(fixedDistance_);
	result.setPoint(sceneBounds_.center() + fixedDistance_ * ray.direction());

	//                  [sin theta * cos phi]
	// R =  r * D = r * [sin theta * sin phi]
	//                  [cos theta          ]
	//
	const TVector3D& dir = ray.direction();

	// N = -D
	//
	const TVector3D normal = -dir;
	result.setNormal(normal);
	result.setGeometricNormal(normal);

	// phi = 2pi * u
	// theta = -pi * v
	//
	const TScalar phi = sphericalPhi(dir);
	const TScalar theta = sphericalTheta(dir);
	result.setUv(phi / (2 * TNumTraits::pi), -theta / TNumTraits::pi);

	//               [sin theta * -sin phi]                  [ cos theta * cos phi]
	// dD/du = 2pi * [sin theta *  cos phi]    dD/dv = -pi * [ cos theta * sin phi]
	//               [0                   ]                  [-sin theta          ]
	//
	const TScalar sinTheta = num::sin(theta);
	const TScalar cosTheta_sinTheta = dir.z / sinTheta;
	const TVector3D dDir_dU = 2 * TNumTraits::pi * TVector3D(-dir.y, dir.x, 0);
	const TVector3D dDir_dV = -TNumTraits::pi * TVector3D(cosTheta_sinTheta * dir.x, cosTheta_sinTheta * dir.y, -sinTheta);

	// dN/du = -dD/du    dN/dv = -dD/dv
	//
	result.setDNormal_dU(-dDir_dU);
	result.setDNormal_dV(-dDir_dV);

	// dR/du = r * dD/du    dR/dv = r * dD/dv
	//
	result.setDPoint_dU(fixedDistance_ * dDir_dU);
	result.setDPoint_dV(fixedDistance_ * dDir_dV);
}



bool LightSky::doContains(const Sample&, const TPoint3D&) const
{
	return true;
}




const TAabb3D LightSky::doBoundingBox() const
{
	return TAabb3D();
}



TScalar LightSky::doArea() const
{
	return TNumTraits::infinity;
}



TScalar LightSky::doArea(const TVector3D&) const
{
	return TNumTraits::infinity;
}



const Spectral LightSky::doEmission(const Sample& sample, const TRay3D& ray, BoundedRay& shadowRay, TScalar& pdf) const
{
	shadowRay = BoundedRay(ray, tolerance, fixedDistance_);
	if (portal_ && !portal_->isIntersecting(sample, shadowRay))
	{
		pdf = 0;
		return Spectral();
	}

	Intersection intersection(this, fixedDistance_, seLeaving);
	IntersectionContext context(*this, sample, shadowRay, intersection, 0);

	const TScalar nx = static_cast<TScalar>(resolution_.x);
	const TScalar ny = static_cast<TScalar>(resolution_.y);

	const TVector3D dir = ray.direction();
	LASS_ASSERT(num::almostEqual(context.uv().x, num::fractional(num::atan2(dir.y, dir.x) / (2 * TNumTraits::pi)), 1e-5));
	const TScalar i = context.uv().x * nx;
	const TScalar j = (dir.z + 1) * ny / 2; // cylindrical coordinate.

	const size_t ii = static_cast<size_t>(num::floor(i > 0 ? i : i + nx)) % resolution_.x;
	const TScalar u0 = ii > 0 ? marginalCdfU_[ii - 1] : TNumTraits::zero;
	const TScalar margPdfU = marginalCdfU_[ii] - u0;

	const TValue* condCdfV = &conditionalCdfV_[ii * resolution_.y];
	const size_t jj = num::clamp<size_t>(static_cast<size_t>(num::floor(j)), 0, resolution_.y - 1);
	LASS_ASSERT(jj < resolution_.y);
	const TValue v0 = jj > 0 ? condCdfV[jj - 1] : 0;
	const TValue condPdfV = condCdfV[jj] - v0;

	pdf = margPdfU * condPdfV * nx * ny / (4 * TNumTraits::pi);

	return radiance_->lookUp(sample, context, SpectralType::Illuminant);
}



const Spectral LightSky::doSampleEmission(
		const Sample& sample, const TPoint2D& lightSample, const TPoint3D& target, 
		BoundedRay& shadowRay, TScalar& pdf) const
{
	TScalar i, j;
	sampleMap(lightSample, i, j, pdf);

	const TVector3D dir = direction(i, j);
	shadowRay = BoundedRay(target, dir, tolerance, fixedDistance_, prim::IsAlreadyNormalized());

	if (portal_ && !portal_->isIntersecting(sample, shadowRay))
	{
		pdf = 0;
		return Spectral();
	}

	Intersection intersection(this, fixedDistance_, seLeaving);
	IntersectionContext context(*this, sample, shadowRay, intersection, 0);
	return radiance_->lookUp(sample, context, SpectralType::Illuminant);
}



const Spectral LightSky::doSampleEmission(
		const Sample& sample, const TPoint2D& lightSampleA, const TPoint2D& lightSampleB,
		BoundedRay& emissionRay, TScalar& pdf) const
{
	TScalar i, j, pdfA;
	sampleMap(lightSampleA, i, j, pdfA);
	const TVector3D dir = direction(i, j);

	TScalar pdfB;
	TPoint3D begin;
	if (portal_)
	{
		TVector3D normal;
		begin = portal_->sampleSurface(lightSampleB, dir, normal, pdfB);
		const TScalar cosTheta = -prim::dot(dir, normal);
		if (cosTheta <= 0)
		{
			pdf = 0;
			return Spectral();
		}
		pdfB /= cosTheta; // is this required?
	}
	else
	{
		// assume a big round disk as portal, fit it around the center, but moved aside by dir * radius_
		TVector3D tangentU, tangentV;
		lass::prim::impl::Plane3DImplDetail::generateDirections(dir, tangentU, tangentV);
		const TPoint2D uv = num::uniformDisk(lightSampleB, pdfB);
		begin = sceneBounds_.center() + sceneBounds_.radius() * (tangentU * uv.x + tangentV * uv.y + dir);
		pdfB /= num::sqr(sceneBounds_.radius());
	}

	emissionRay = BoundedRay(begin, -dir, tolerance);
	pdf = pdfA * pdfB;

	BoundedRay shadowRay(begin - fixedDistance_ * dir, dir, tolerance, fixedDistance_, prim::IsAlreadyNormalized());
	Intersection intersection(this, fixedDistance_, seLeaving);
	IntersectionContext context(*this, sample, shadowRay, intersection, 0);
	return radiance_->lookUp(sample, context, SpectralType::Illuminant);
}



TScalar LightSky::doTotalPower() const
{
	return power_;
}



size_t LightSky::doNumberOfEmissionSamples() const
{
	return numberOfSamples_;
}



bool LightSky::doIsSingular() const
{
	return false;
}



const TPyObjectPtr LightSky::doGetLightState() const
{
	return python::makeTuple(radiance_, numberOfSamples_);
}



void LightSky::doSetLightState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, radiance_, numberOfSamples_);
}



void LightSky::init(const TTexturePtr& radiance)
{
	setRadiance(radiance);
	setNumberOfEmissionSamples(1);
	setSamplingResolution(TResolution2D(512, 512));
}



void LightSky::buildPdf(TMap& pdf, TScalar& power, util::ProgressIndicator& progress) const
{
	Sample dummy;

	const size_t n = resolution_.x * resolution_.y;
	TMap tempPdf(n);
	TScalar averageIntensity = 0;
	for (size_t i = 0; i < resolution_.x; ++i)
	{
		const TScalar fi = static_cast<TScalar>(i) + .5f;
		progress(.8 * static_cast<double>(i) / static_cast<double>(resolution_.x));
		for (size_t j = 0; j < resolution_.y; ++j)
		{
			const TScalar fj = static_cast<TScalar>(j) + .5f;
			const TValue radiance = lookUpLuminance(dummy, fi, fj);;
			const TScalar projectedArea = portal_ ? portal_->area(direction(fi, fj)) : (TNumTraits::pi * num::sqr(fixedDistance_));
			const TValue intensity = static_cast<TValue>(radiance * projectedArea);
			tempPdf[i * resolution_.y + j] = intensity;
			averageIntensity += intensity;
		}
	}
	averageIntensity /= static_cast<TScalar>(n);

	pdf.swap(tempPdf);
	power = averageIntensity * (4 * TNumTraits::pi);
}



void LightSky::buildCdf(const TMap& pdf, TMap& oMarginalCdfU, TMap& oConditionalCdfV, util::ProgressIndicator& progress) const
{
	TMap marginalPdfU(resolution_.x);
	TMap marginalCdfU(resolution_.x);
	TMap conditionalCdfV(resolution_.x * resolution_.y);
	
	for (size_t i = 0; i < resolution_.x; ++i)
	{
		progress(.8 + .2 * static_cast<double>(i) / static_cast<double>(resolution_.x));
		const size_t offset = i * resolution_.y;
		TMap::const_iterator pdfLine = pdf.begin() + offset;
		TMap::iterator condCdfV = conditionalCdfV.begin() + offset;
		std::partial_sum(pdfLine, pdfLine + resolution_.y, condCdfV);	
		marginalPdfU[i]	= condCdfV[resolution_.y - 1];
		std::transform(condCdfV, condCdfV + resolution_.y, condCdfV, [d = marginalPdfU[i]](TScalar x) { return x / d; });
	}

	std::partial_sum(marginalPdfU.begin(), marginalPdfU.end(), marginalCdfU.begin());
	std::transform(marginalCdfU.begin(), marginalCdfU.end(), marginalCdfU.begin(), [d = marginalCdfU.back()](TScalar x) { return x / d; });

	oMarginalCdfU.swap(marginalCdfU);
	oConditionalCdfV.swap(conditionalCdfV);
}



void LightSky::sampleMap(const TPoint2D& sample, TScalar& i, TScalar& j, TScalar& pdf) const
{
	const size_t ii = std::min(resolution_.x - 1, static_cast<size_t>(
		std::lower_bound(marginalCdfU_.begin(), marginalCdfU_.end(), sample.x) - marginalCdfU_.begin()));
	const TScalar u0 = ii > 0 ? marginalCdfU_[ii - 1] : TNumTraits::zero;
	const TScalar margPdfU = marginalCdfU_[ii] - u0;
	i = static_cast<TScalar>(ii) + (sample.x - u0) / margPdfU;

	TMap::const_iterator condCdfV = conditionalCdfV_.begin() + ii * resolution_.y;
	const size_t jj = std::min(resolution_.y - 1, static_cast<size_t>(
		std::lower_bound(condCdfV, condCdfV + resolution_.y, sample.y) - condCdfV));
	const TValue v0 = jj > 0 ? condCdfV[jj - 1] : 0;
	const TValue condPdfV = condCdfV[jj] - v0;
	j = static_cast<TScalar>(jj) + (sample.y - v0) / condPdfV;

	pdf = margPdfU * condPdfV * static_cast<TScalar>(resolution_.x * resolution_.y) / (4 * TNumTraits::pi);
}



const TVector3D LightSky::direction(TScalar i, TScalar j) const
{
	const TScalar z = 2 * invResolution_.y * j - 1;
	const TScalar r = num::sqrt(std::max(TNumTraits::zero, 1 - z * z));
	const TScalar theta = 2 * TNumTraits::pi * invResolution_.x * i;
	const TScalar x = r * num::cos(theta);
	const TScalar y = r * num::sin(theta);
	return TVector3D(x, y, z);
}



const Spectral LightSky::lookUpRadiance(const Sample& sample, TScalar i, TScalar j) const
{
	const BoundedRay centralRay(sceneBounds_.center(), direction(i, j));
	const TRay3D differentialI(sceneBounds_.center(), direction(i + 1, j));
	const TRay3D differentialJ(sceneBounds_.center(), direction(i, j + 1));
	const DifferentialRay ray(centralRay, differentialI, differentialJ);

	Intersection intersection(this, fixedDistance_, seLeaving);
	IntersectionContext context(*this, sample, ray, intersection, 0);

	return radiance_->lookUp(sample, context, SpectralType::Illuminant);
}



LightSky::TValue LightSky::lookUpLuminance(const Sample& sample, TScalar i, TScalar j) const
{
	const BoundedRay centralRay(sceneBounds_.center(), direction(i, j));
	const TRay3D differentialI(sceneBounds_.center(), direction(i + 1, j));
	const TRay3D differentialJ(sceneBounds_.center(), direction(i, j + 1));
	const DifferentialRay ray(centralRay, differentialI, differentialJ);

	Intersection intersection(this, fixedDistance_, seLeaving);
	IntersectionContext context(*this, sample, ray, intersection, 0);

	return radiance_->scalarLookUp(sample, context);
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
