/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2006  Bram de Greve
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

#include "scenery_common.h"
#include "light_sky.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(LightSky)
PY_CLASS_CONSTRUCTOR_0(LightSky)
PY_CLASS_CONSTRUCTOR_1(LightSky, const TTexturePtr&)
PY_CLASS_MEMBER_RW(LightSky, "radiance", radiance, setRadiance)
PY_CLASS_MEMBER_RW(LightSky, "numberOfEmissionSamples", numberOfEmissionSamples, 
	setNumberOfEmissionSamples)
PY_CLASS_MEMBER_RW(LightSky, "samplingResolution", samplingResolution, setSamplingResolution)


// --- public --------------------------------------------------------------------------------------

LightSky::LightSky():
	radiance_(Texture::white())
{
	setSamplingResolution(512);
}



LightSky::LightSky(const TTexturePtr& radiance):
	radiance_(radiance)
{
	setSamplingResolution(512);
}



const TTexturePtr& LightSky::radiance() const
{
	return radiance_;
}



const unsigned LightSky::samplingResolution() const
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




void LightSky::setSamplingResolution(unsigned resolution)
{
#pragma LASS_TODO("clamp")
	resolution_ = static_cast<int>(resolution);
	LASS_ASSERT(resolution_ > 0);
	invResolution_ = TNumTraits::one / resolution_;
}




// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void LightSky::doPreProcess(const TSceneObjectPtr& scene, const TimePeriod& period)
{
	TMap pdf;
	buildPdf(pdf, averageRadiance_);
	buildCdf(pdf, marginalCdfU_, conditionalCdfV_);

	std::ofstream pdfFile("pdf.txt");
	std::ofstream condCdfVFile("condCdfV.txt");
	std::ofstream margCdfUFile("margCdfU.txt");

	for (int j = 0; j < resolution_; ++j)
	{
		pdfFile << pdf[j];
		condCdfVFile << conditionalCdfV_[j];
		for (int i = 1; i < resolution_; ++i)
		{
			pdfFile << "\t" << pdf[i * resolution_ + j];
			condCdfVFile << "\t" << conditionalCdfV_[i * resolution_ + j];
		}
		pdfFile << std::endl;
	}

	margCdfUFile << marginalCdfU_[0];
	for (int i = 1; i < resolution_; ++i)
	{
		margCdfUFile << "\t" << marginalCdfU_[i];
	}
}



void LightSky::doIntersect(const Sample& sample, const BoundedRay& iRAy, 
	Intersection& result) const
{
	result = Intersection(this, TNumTraits::max, seLeaving);
}



const bool LightSky::doIsIntersecting(const Sample& sample, 
	const BoundedRay& ray) const
{
	return false; //return ray.inRange(TNumTraits::max);
}



void LightSky::doLocalContext(const Sample& sample, const BoundedRay& ray,
								const Intersection& intersection, 
								IntersectionContext& result) const
{
	LASS_ASSERT(intersection.t() == TNumTraits::infinity);
    result.setT(TNumTraits::infinity);
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
    const TScalar theta = num::acos(normal.z);
	result.setUv(phi / (2 * TNumTraits::pi), theta / TNumTraits::pi);

	//               [sin theta * -sin phi]               [cos theta * cos phi]
	// dN_du = 2pi * [sin theta * cos phi ]  dN_dv = pi * [cos theta * sin phi]
	//               [0                   ]               [-sin theta         ]
	//
	const TScalar sinTheta = num::sin(theta);
	const TScalar cosTheta_sinTheta = normal.z / sinTheta;
	const TVector3D dNormal_dU = 2 * TNumTraits::pi * TVector3D(-normal.y, normal.x, 0);
	const TVector3D dNormal_dV = TNumTraits::pi * TVector3D(
		cosTheta_sinTheta * normal.x, cosTheta_sinTheta * normal.y, -sinTheta);
	result.setDNormal_dU(dNormal_dU);
	result.setDNormal_dV(dNormal_dV);
	
	result.setDPoint_dU(-radius_ * dNormal_dU);
	result.setDPoint_dV(-radius_ * dNormal_dU);
}



const bool LightSky::doContains(const Sample& sample, const TPoint3D& point) const
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



const Spectrum LightSky::doSampleEmission(
		const Sample& sample,
		const TPoint2D& lightSample, 
		const TPoint3D& target,
		const TVector3D& targetNormal,
		BoundedRay& shadowRay,
		TScalar& pdf) const
{
	TScalar i, j;
	sampleMap(lightSample, i, j, pdf);

	const TVector3D dir = direction(i, j);
	shadowRay = BoundedRay(target, dir, tolerance, TNumTraits::infinity,
		prim::IsAlreadyNormalized());

	return lookUpRadiance(sample, i, j);
}



const Spectrum LightSky::doSampleEmission(const TPoint2D& sampleA, const TPoint2D& sampleB,
		const TPoint3D& sceneCenter, TScalar sceneRadius, TRay3D& emissionRay, TScalar& pdf) const
{
	TScalar i, j, originPdf;
	sampleMap(sampleA, i, j, originPdf);
	const TVector3D originDir = direction(i, j);
	const TPoint3D origin = sceneCenter + sceneRadius * originDir;
	
	TScalar targetPdf;
	const TPoint3D target = sceneCenter + 
		sceneRadius * num::uniformSphere(sampleB, targetPdf).position();
	const TVector3D targetDir = target - origin;

	emissionRay = TRay3D(origin, target - origin);

	pdf = originPdf * targetPdf * -dot(emissionRay.direction(), originDir);
	LASS_ASSERT(pdf >= 0);

	Sample dummy;
	return lookUpRadiance(dummy, i, j);
}



const Spectrum LightSky::doTotalPower(TScalar sceneRadius) const
{
	return (TNumTraits::pi * num::sqr(sceneRadius)) * averageRadiance_;
}



const unsigned LightSky::doNumberOfEmissionSamples() const
{
	return numberOfSamples_;
}



const TPyObjectPtr LightSky::doGetLightState() const
{
	return python::makeTuple(radiance_, numberOfSamples_);
}



void LightSky::doSetLightState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, radiance_, numberOfSamples_);
}



void LightSky::buildPdf(TMap& pdf, Spectrum& averageRadiance) const
{
	Sample dummy;

	TMap tempPdf(resolution_ * resolution_);
	Spectrum totalRadiance;
    for (int i = 0; i < resolution_; ++i)
	{
		for (int j = 0; j < resolution_; ++j)
		{
			const Spectrum radiance = lookUpRadiance(
				dummy, static_cast<TScalar>(i), static_cast<TScalar>(j));
			totalRadiance += radiance;
			tempPdf[i * resolution_ + j] = radiance.average();
		}
	}
	pdf.swap(tempPdf);
	averageRadiance = totalRadiance / num::sqr(static_cast<TScalar>(resolution_));
}



void LightSky::buildCdf(const TMap& pdf, TMap& oMarginalCdfU, TMap& oConditionalCdfV) const
{
	TMap marginalPdfU(resolution_);
	TMap marginalCdfU(resolution_);
	TMap conditionalCdfV(resolution_ * resolution_);
	
	TScalar sumU = 0;
    for (int i = 0; i < resolution_; ++i)
	{
		const TScalar* pdfLine = &pdf[i * resolution_];
		TScalar* condCdfV = &conditionalCdfV[i * resolution_];
		std::partial_sum(pdfLine, pdfLine + resolution_, condCdfV);
		
		marginalPdfU[i]	= condCdfV[resolution_ - 1];

		std::transform(condCdfV, condCdfV + resolution_, condCdfV, 
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
	const int ii = static_cast<int>(
		std::lower_bound(marginalCdfU_.begin(), marginalCdfU_.end(), sample.x) - marginalCdfU_.begin());
	LASS_ASSERT(ii >= 0 && ii < resolution_);
	const TScalar u0 = ii > 0 ? marginalCdfU_[ii - 1] : TNumTraits::zero;
	const TScalar margPdfU = marginalCdfU_[ii] - u0;
	i = static_cast<TScalar>(ii) + (sample.x - u0) / margPdfU;

	const TScalar* condCdfV = &conditionalCdfV_[ii * resolution_];
	const int jj = static_cast<int>(
		std::lower_bound(condCdfV, condCdfV + resolution_, sample.y) - condCdfV);
	LASS_ASSERT(jj >= 0 && jj < resolution_);
	const TScalar v0 = jj > 0 ? condCdfV[jj - 1] : TNumTraits::zero;
	const TScalar condPdfV = condCdfV[jj] - v0;
	j = static_cast<TScalar>(jj) + (sample.y - v0) / condPdfV;

	pdf = margPdfU * condPdfV * (resolution_ * resolution_) / (4 * TNumTraits::pi);
}



const TVector3D LightSky::direction(TScalar i, TScalar j) const
{
	const TScalar z = 2 * invResolution_ * j - 1;
	const TScalar r = num::sqrt(std::max(TNumTraits::zero, 1 - z * z));
	const TScalar theta = 2 * TNumTraits::pi * invResolution_ * i;
	const TScalar x = r * num::cos(theta);
	const TScalar y = r * num::sin(theta);
	return TVector3D(x, y, z);
}



const Spectrum LightSky::lookUpRadiance(const Sample& sample, TScalar i, TScalar j) const
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