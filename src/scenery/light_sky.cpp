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

#include "scenery_common.h"
#include "light_sky.h"
#include <lass/prim/sphere_3d.h>
#include <lass/num/distribution_transformations.h>
#include <lass/prim/impl/plane_3d_impl_detail.h>

#include <lass/io/image.h>
#include "../kernel/rgb_space.h"

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
PY_CLASS_MEMBER_RW(LightSky, numberOfEmissionSamples, setNumberOfEmissionSamples)
PY_CLASS_MEMBER_RW(LightSky, samplingResolution, setSamplingResolution)


// --- public --------------------------------------------------------------------------------------

LightSky::LightSky():
	radiance_(Texture::white())
{
	setSamplingResolution(TResolution2D(512, 512));
}



LightSky::LightSky(const TTexturePtr& radiance):
	radiance_(radiance)
{
	setSamplingResolution(TResolution2D(512, 512));
}



const TTexturePtr& LightSky::radiance() const
{
	return radiance_;
}



const TResolution2D& LightSky::samplingResolution() const
{
	return resolution_;
}



void LightSky::setRadiance(const TTexturePtr& radiance)
{
	radiance_ = radiance;
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

void LightSky::doPreProcess(const TSceneObjectPtr&, const TimePeriod&)
{
	TMap pdf;
	buildPdf(pdf, radianceMap_, averageRadiance_);
	buildCdf(pdf, marginalCdfU_, conditionalCdfV_);
}



void LightSky::doIntersect(const Sample&, const BoundedRay&, Intersection& result) const
{
	result = Intersection(this, TNumTraits::max, seLeaving);
}



const bool LightSky::doIsIntersecting(const Sample&, const BoundedRay&) const
{
	return false; //return ray.inRange(TNumTraits::max);
}



void LightSky::doLocalContext(
		const Sample& sample, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const
{
	LASS_ASSERT(intersection.t() == TNumTraits::max);
	result.setT(TNumTraits::max);
	result.setPoint(TPoint3D(ray.direction()));

	//         [sin theta * cos phi]
	// R = r * [sin theta * sin phi]
	//         [cos theta          ]
	//
	const TVector3D normal = -ray.direction();
	result.setNormal(normal);
	result.setGeometricNormal(normal);

	// phi = 2pi * u
	// theta = pi * v
	//
	LASS_ASSERT(normal.z >= -TNumTraits::one && normal.z <= TNumTraits::one);
	const TScalar phi = num::atan2(normal.x, normal.y);
	const TScalar cosTheta = normal.z;
	const TScalar theta = num::acos(cosTheta);
	result.setUv(phi / (2 * TNumTraits::pi), theta / TNumTraits::pi);

	//               [sin theta * -sin phi]               [cos theta * cos phi]
	// dN_du = 2pi * [sin theta * cos phi ]  dN_dv = pi * [cos theta * sin phi]
	//               [0                   ]               [-sin theta         ]
	//
	const TScalar sinTheta = num::sin(theta);
	const TScalar cosPhi = num::cos(phi);
	const TScalar sinPhi = num::sin(phi);
	const TVector3D dNormal_dU = 2 * TNumTraits::pi * TVector3D(-sinTheta * sinPhi, sinTheta * cosPhi, 0);
	const TVector3D dNormal_dV = TNumTraits::pi * TVector3D(cosTheta * cosPhi, cosTheta * sinPhi, -sinTheta);
	result.setDNormal_dU(dNormal_dU);
	result.setDNormal_dV(dNormal_dV);
	
	result.setDPoint_dU(-dNormal_dU);
	result.setDPoint_dV(-dNormal_dV);
}



const bool LightSky::doContains(const Sample&, const TPoint3D&) const
{
	return true;
}




const TAabb3D LightSky::doBoundingBox() const
{
	return TAabb3D();
}



const TScalar LightSky::doArea() const
{
	return TNumTraits::infinity;
}



const XYZ LightSky::doEmission(
		const Sample& sample, const TRay3D& ray, BoundedRay& shadowRay, TScalar& pdf) const
{
	const TVector3D dir = ray.direction();
	const TScalar i = num::atan2(dir.y, dir.x) * resolution_.x / (2 * TNumTraits::pi);
	const TScalar j = (dir.z + 1) * resolution_.y / 2;

	const size_t ii = static_cast<size_t>(num::floor(i));
	LASS_ASSERT(ii < resolution_.x);
	const TScalar u0 = ii > 0 ? marginalCdfU_[ii - 1] : TNumTraits::zero;
	const TScalar margPdfU = marginalCdfU_[ii] - u0;

	const TScalar* condCdfV = &conditionalCdfV_[ii * resolution_.y];
	const size_t jj = static_cast<size_t>(num::floor(j));
	LASS_ASSERT(jj < resolution_.y);
	const TScalar v0 = jj > 0 ? condCdfV[jj - 1] : TNumTraits::zero;
	const TScalar condPdfV = condCdfV[jj] - v0;

	shadowRay = BoundedRay(ray, tolerance, TNumTraits::infinity);
	pdf = margPdfU * condPdfV * (resolution_.x * resolution_.y) / (4 * TNumTraits::pi);
	return radianceMap_[ii * resolution_.y + jj];
}



const XYZ LightSky::doSampleEmission(
		const Sample&, const TPoint2D& lightSample, const TPoint3D& target, const TVector3D&, 
		BoundedRay& shadowRay, TScalar& pdf) const
{
	TScalar i, j;
	sampleMap(lightSample, i, j, pdf);

	const TVector3D dir = direction(i, j);
	shadowRay = BoundedRay(target, dir, tolerance, TNumTraits::infinity,
		prim::IsAlreadyNormalized());

	return radianceMap_[static_cast<size_t>(num::floor(i)) * resolution_.y + static_cast<size_t>(num::floor(j))];
}



const XYZ LightSky::doSampleEmission(
		const Sample& cameraSample, const TPoint2D& lightSampleA, const TPoint2D& lightSampleB,
		const TAabb3D& sceneBound, BoundedRay& emissionRay, TScalar& pdf) const
{
	const prim::Sphere3D<TScalar> worldSphere = boundingSphere(sceneBound);

	TScalar i, j, pdfA;
	sampleMap(lightSampleA, i, j, pdfA);
	const TVector3D dir = -direction(i, j);
	
	TVector3D tangentU, tangentV;
	lass::prim::impl::Plane3DImplDetail::generateDirections(dir, tangentU, tangentV);

	TScalar pdfB;
	const TPoint2D uv = num::uniformDisk(lightSampleB, pdfB);
	const TPoint3D begin = worldSphere.center() + 
		worldSphere.radius() * (tangentU * uv.x + tangentV * uv.y - dir);

	emissionRay = BoundedRay(begin, dir, tolerance);
	pdf = pdfA * pdfB / num::sqr(worldSphere.radius());

	return radianceMap_[static_cast<size_t>(num::floor(i)) * resolution_.y + static_cast<size_t>(num::floor(j))];
}



const XYZ LightSky::doTotalPower(const TAabb3D& sceneBound) const
{
	const prim::Sphere3D<TScalar> worldSphere = boundingSphere(sceneBound);
	return (8 * num::sqr(TNumTraits::pi * worldSphere.radius())) * averageRadiance_;
}



const size_t LightSky::doNumberOfEmissionSamples() const
{
	return numberOfSamples_;
}



const bool LightSky::doIsSingular() const
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



void LightSky::buildPdf(TMap& pdf, TXYZMap& radianceMap, XYZ& averageRadiance) const
{
	Sample dummy;

	TMap tempPdf(resolution_.x * resolution_.y);
	TXYZMap radMap(resolution_.x * resolution_.y);
	XYZ totalRadiance;
	for (size_t i = 0; i < resolution_.x; ++i)
	{
		for (size_t j = 0; j < resolution_.y; ++j)
		{
			const XYZ radiance = lookUpRadiance(
				dummy, static_cast<TScalar>(i) + .5f, static_cast<TScalar>(j) + .5f);
			totalRadiance += radiance;
			radMap[i * resolution_.y + j] = radiance;
			tempPdf[i * resolution_.y + j] = average(radiance);
		}
	}
	averageRadiance = totalRadiance / static_cast<TScalar>(resolution_.x * resolution_.y);
	radianceMap.swap(radMap);
	pdf.swap(tempPdf);
}



void LightSky::buildCdf(const TMap& pdf, TMap& oMarginalCdfU, TMap& oConditionalCdfV) const
{
	TMap marginalPdfU(resolution_.x);
	TMap marginalCdfU(resolution_.x);
	TMap conditionalCdfV(resolution_.x * resolution_.y);
	
	TScalar sumU = 0;
	for (size_t i = 0; i < resolution_.x; ++i)
	{
		const TScalar* pdfLine = &pdf[i * resolution_.y];
		TScalar* condCdfV = &conditionalCdfV[i * resolution_.y];
		std::partial_sum(pdfLine, pdfLine + resolution_.y, condCdfV);
		
		marginalPdfU[i]	= condCdfV[resolution_.y - 1];

		std::transform(condCdfV, condCdfV + resolution_.y, condCdfV, 
			std::bind2nd(std::divides<TScalar>(), marginalPdfU[i]));
	}

	std::partial_sum(marginalPdfU.begin(), marginalPdfU.end(), marginalCdfU.begin());
	const TScalar total	= marginalCdfU.back();
	std::transform(marginalCdfU.begin(), marginalCdfU.end(), marginalCdfU.begin(), 
		std::bind2nd(std::divides<TScalar>(), total));

	oMarginalCdfU.swap(marginalCdfU);
	oConditionalCdfV.swap(conditionalCdfV);
}



void LightSky::sampleMap(const TPoint2D& sample, TScalar& i, TScalar& j, TScalar& pdf) const
{
	const size_t ii = static_cast<size_t>(
		std::lower_bound(marginalCdfU_.begin(), marginalCdfU_.end(), sample.x) - marginalCdfU_.begin());
	LASS_ASSERT(ii < resolution_.x);
	const TScalar u0 = ii > 0 ? marginalCdfU_[ii - 1] : TNumTraits::zero;
	const TScalar margPdfU = marginalCdfU_[ii] - u0;
	i = static_cast<TScalar>(ii) + (sample.x - u0) / margPdfU;

	const TScalar* condCdfV = &conditionalCdfV_[ii * resolution_.y];
	const size_t jj = static_cast<size_t>(
		std::lower_bound(condCdfV, condCdfV + resolution_.y, sample.y) - condCdfV);
	LASS_ASSERT(jj < resolution_.y);
	const TScalar v0 = jj > 0 ? condCdfV[jj - 1] : TNumTraits::zero;
	const TScalar condPdfV = condCdfV[jj] - v0;
	j = static_cast<TScalar>(jj) + (sample.y - v0) / condPdfV;

	pdf = margPdfU * condPdfV * (resolution_.x * resolution_.y) / (4 * TNumTraits::pi);
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



const XYZ LightSky::lookUpRadiance(const Sample& sample, TScalar i, TScalar j) const
{
	const TPoint3D origin;
	const BoundedRay centralRay(origin, direction(i, j));
	const TRay3D differentialI(origin, direction(i + 1, j));
	const TRay3D differentialJ(origin, direction(i, j + 1));
	const DifferentialRay ray(centralRay, differentialI, differentialJ);

	Intersection intersection;
	this->intersect(sample, ray, intersection);

	IntersectionContext context;
	this->localContext(sample, ray, intersection, context);

	return radiance_->lookUp(sample, context);
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
