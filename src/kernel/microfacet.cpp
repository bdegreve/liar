/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2023-2024  Bram de Greve (bramz@users.sourceforge.net)
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

#include "kernel_common.h"
#include "microfacet.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(MicrofacetDistribution, "Abstract base class of microfacet distributions")
PY_CLASS_METHOD_NAME(MicrofacetDistribution, reduce, "__reduce__")
PY_CLASS_METHOD_NAME(MicrofacetDistribution, getState, "__getstate__")
PY_CLASS_METHOD_NAME(MicrofacetDistribution, setState, "__setstate__")


// --- public --------------------------------------------------------------------------------------

MicrofacetDistribution::TValue
MicrofacetDistribution::D(const TVector3D& omegaHalf, TValue alphaX, TValue alphaY, TValue& pdfH) const
{
	return doD(omegaHalf, alphaX, alphaY, pdfH);
}



TVector3D MicrofacetDistribution::sampleH(const TPoint2D& sample, TValue alphaU, TValue alphaV) const
{
	return doSampleH(sample, alphaU, alphaV);
}



MicrofacetDistribution::TValue
MicrofacetDistribution::G1(const TVector3D& omega, const TVector3D& omegaHalf, TValue alphaU, TValue alphaV) const
{
	// E. Heitz, Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs, 2014, p70.

	if (dot(omega, omegaHalf) * omega.z <= 0) // same effect as X+(v.m / v.n): 1 if both have same sign, 0 otherwise
	{
		return 0;
	}
	return num::inv(1 + doLambda(omega, alphaU, alphaV));
}



MicrofacetDistribution::TValue
MicrofacetDistribution::lambda(const TVector3D& omega, TValue alphaU, TValue alphaV) const
{
	return doLambda(omega, alphaU, alphaV);
}



const TPyObjectPtr MicrofacetDistribution::reduce() const
{
	return python::makeTuple(
		python::fromNakedToSharedPtrCast<PyObject>(reinterpret_cast<PyObject*>(this->_lassPyGetClassDef()->type())),
		python::makeTuple(), this->getState());
}



const TPyObjectPtr MicrofacetDistribution::getState() const
{
	return doGetState();
}



void MicrofacetDistribution::setState(const TPyObjectPtr& state)
{
	doSetState(state);
}



// --- protected -----------------------------------------------------------------------------------

MicrofacetDistribution::TValue
MicrofacetDistribution::samplePhi(TValue sample, TValue alphaU, TValue alphaV) const
{
	using TNumTraits = num::NumTraits<TValue>;
	if (alphaU == alphaV)
	{
		return 2 * TNumTraits::pi * sample;
	}
	const TValue phi = std::atan(alphaV / alphaU * num::tan(2 * TNumTraits::pi * sample));
	if (sample <= 0.25f)
	{
		return phi;
	}
	else if (sample < 0.75f)
	{
		return phi + TNumTraits::pi;
	}
	else
	{
		return phi + 2 * TNumTraits::pi;
	}
}



// --- private -------------------------------------------------------------------------------------

const TPyObjectPtr MicrofacetDistribution::doGetState() const
{
	return python::makeTuple();
}



void MicrofacetDistribution::doSetState(const TPyObjectPtr& /*state*/)
{
}


// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
