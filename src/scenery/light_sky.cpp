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

#include "scenery_common.h"
#include "light_sky.h"

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
    SceneLight(&Type),
	radiance_(Texture::white())
{
	setSamplingResolution(512);
}



LightSky::LightSky(const TTexturePtr& iRadiance):
    SceneLight(&Type),
	radiance_(iRadiance)
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



void LightSky::setRadiance(const TTexturePtr& iRadiance)
{
	radiance_ = iRadiance;
}



void LightSky::setNumberOfEmissionSamples(unsigned iNumber)
{
	numberOfSamples_ = iNumber;
}




void LightSky::setSamplingResolution(unsigned iResolution)
{
#pragma LASS_TODO("clamp")
	resolution_ = static_cast<int>(iResolution);
	LASS_ASSERT(resolution_ > 0);
	invResolution_ = TNumTraits::one / resolution_;
}




// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void LightSky::doPreProcess(const TimePeriod& iPeriod)
{
	TMap pdf;
	buildPdf(pdf);
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



void LightSky::doIntersect(const Sample& iSample, const BoundedRay& iRAy, 
	Intersection& oResult) const
{
	oResult = Intersection(this, TNumTraits::max, seLeaving);
}



const bool LightSky::doIsIntersecting(const Sample& iSample, 
	const BoundedRay& iRay) const
{
	return false; //return iRay.inRange(TNumTraits::max);
}



void LightSky::doLocalContext(const Sample& iSample, const BoundedRay& iRay,
								const Intersection& iIntersection, 
								IntersectionContext& oResult) const
{
	LASS_ASSERT(iIntersection.t() == TNumTraits::infinity);
    oResult.setT(TNumTraits::infinity);
    oResult.setPoint(TPoint3D(iRay.direction()));

	//         [sin theta * cos phi]
	// R = r * [sin theta * sin phi]
	//         [cos theta          ]
	//
	const TVector3D normal = -iRay.direction();
	oResult.setNormal(normal);
	oResult.setGeometricNormal(normal);

	// phi = 2pi * u
	// theta = pi * v
	//
	LASS_ASSERT(normal.z >= -TNumTraits::one && normal.z <= TNumTraits::one);
    const TScalar phi = num::atan2(normal.x, normal.y);
    const TScalar theta = num::acos(normal.z);
	oResult.setUv(phi / (2 * TNumTraits::pi), theta / TNumTraits::pi);

	//               [sin theta * -sin phi]               [cos theta * cos phi]
	// dN_du = 2pi * [sin theta * cos phi ]  dN_dv = pi * [cos theta * sin phi]
	//               [0                   ]               [-sin theta         ]
	//
	const TScalar sinTheta = num::sin(theta);
	const TScalar cosTheta_sinTheta = normal.z / sinTheta;
	const TVector3D dNormal_dU = 2 * TNumTraits::pi * TVector3D(-normal.y, normal.x, 0);
	const TVector3D dNormal_dV = TNumTraits::pi * TVector3D(
		cosTheta_sinTheta * normal.x, cosTheta_sinTheta * normal.y, -sinTheta);
	oResult.setDNormal_dU(dNormal_dU);
	oResult.setDNormal_dV(dNormal_dV);
	
	oResult.setDPoint_dU(-radius_ * dNormal_dU);
	oResult.setDPoint_dV(-radius_ * dNormal_dU);
}



const bool LightSky::doContains(const Sample& iSample, const TPoint3D& iPoint) const
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
		const Sample& iSample,
		const TVector2D& iLightSample, 
		const TPoint3D& iTarget,
		const TVector3D& iTargetNormal,
		BoundedRay& oShadowRay,
		TScalar& oPdf) const
{
	const int i = std::lower_bound(marginalCdfU_.begin(), marginalCdfU_.end(), iLightSample.x) 
		- marginalCdfU_.begin();
	LASS_ASSERT(i >= 0 && i < resolution_);
	const TScalar* condCdfV = &conditionalCdfV_[i * resolution_];
	const int j = std::lower_bound(condCdfV, condCdfV + resolution_, iLightSample.y) - condCdfV;
	LASS_ASSERT(j >= 0 && j < resolution_);

	const TVector3D dir = direction(i, j);
	oShadowRay = BoundedRay(iTarget, dir, tolerance, TNumTraits::infinity,
		prim::IsAlreadyNormalized());

	const TScalar margPdfU = marginalCdfU_[i] - (i > 0 ? marginalCdfU_[i - 1] : TNumTraits::zero);
	const TScalar condPdfV = condCdfV[j] - (j > 0 ? condCdfV[j - 1] : TNumTraits::zero);
	oPdf = margPdfU * condPdfV * (resolution_ * resolution_) / (4 * TNumTraits::pi);

	return lookUpRadiance(iSample, i, j);
}



const unsigned LightSky::doNumberOfEmissionSamples() const
{
	return numberOfSamples_;
}



const TPyObjectPtr LightSky::doGetLightState() const
{
	return python::makeTuple(radiance_, numberOfSamples_);
}



void LightSky::doSetLightState(const TPyObjectPtr& iState)
{
	python::decodeTuple(iState, radiance_, numberOfSamples_);
}



void LightSky::buildPdf(TMap& oPdf) const
{
	Sample sample;
	
	TMap pdf(resolution_ * resolution_);
    for (int i = 0; i < resolution_; ++i)
	{
		for (int j = 0; j < resolution_; ++j)
		{
			pdf[i * resolution_ + j] = lookUpRadiance(sample, i, j).average();
		}
	}
	oPdf.swap(pdf);
}



void LightSky::buildCdf(const TMap& iPdf, TMap& oMarginalCdfU, TMap& oConditionalCdfV) const
{
	TMap marginalPdfU(resolution_);
	TMap marginalCdfU(resolution_);
	TMap conditionalCdfV(resolution_ * resolution_);
	
	TScalar sumU = 0;
    for (int i = 0; i < resolution_; ++i)
	{
		const TScalar* pdf = &iPdf[i * resolution_];
		TScalar* condCdfV = &conditionalCdfV[i * resolution_];
		std::partial_sum(pdf, pdf + resolution_, condCdfV);
		
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



const TVector3D LightSky::direction(unsigned i, unsigned j) const
{
	const TScalar z = 2 * invResolution_ * j - 1;
	const TScalar r = num::sqrt(std::max(TNumTraits::zero, 1 - z * z));
	const TScalar theta = 2 * TNumTraits::pi * invResolution_ * i;
	const TScalar x = r * num::cos(theta);
	const TScalar y = r * num::sin(theta);
	return TVector3D(x, y, z);
}



const Spectrum LightSky::lookUpRadiance(const Sample& iSample, unsigned i, unsigned j) const
{
	const TPoint3D origin;
	const BoundedRay centralRay(origin, direction(i, j));
	const TRay3D differentialI(origin, direction(i + 1, j));
	const TRay3D differentialJ(origin, direction(i, j + 1));
	const DifferentialRay ray(centralRay, differentialI, differentialJ);

	Intersection intersection;
	this->intersect(iSample, ray, intersection);

	IntersectionContext context;
	this->localContext(iSample, ray, intersection, context);

	return radiance_->lookUp(iSample, context);
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
