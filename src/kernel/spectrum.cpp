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

#include "kernel_common.h"
#include "spectrum.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(Spectrum, "Abstract base class of spectrum definitionas")
	PY_CLASS_MEMBER_R(Spectrum, luminance)
	PY_CLASS_MEMBER_R(Spectrum, isFlat)
	PY_CLASS_METHOD_NAME(Spectrum, reduce, "__reduce__")
	PY_CLASS_METHOD_NAME(Spectrum, getState, "__getstate__")
	PY_CLASS_METHOD_NAME(Spectrum, setState, "__setstate__")

typedef impl::SpectrumFlat TSpectrumFlat;
PY_DECLARE_CLASS_NAME(TSpectrumFlat, "Flat");
PY_CLASS_INNER_CLASS_NAME(Spectrum, TSpectrumFlat, "Flat");
PY_CLASS_MEMBER_R(TSpectrumFlat, value)

TSpectrumPtr Spectrum::white_(new TSpectrumFlat(1));
TSpectrumPtr Spectrum::black_(new TSpectrumFlat(0));

typedef impl::SpectrumXYZ TSpectrumXYZ;
PY_DECLARE_CLASS_NAME(TSpectrumXYZ, "XYZ");
PY_CLASS_INNER_CLASS_NAME(Spectrum, TSpectrumXYZ, "XYZ");
PY_CLASS_MEMBER_R(TSpectrumXYZ, value)




	// --- public --------------------------------------------------------------------------------------

Spectrum::~Spectrum()
{
}


Spectral Spectrum::evaluate(const Sample& sample, SpectralType type) const
{
	return doEvaluate(sample, type);
}


Spectrum::TValue Spectrum::luminance() const
{
	return doLuminance();
}


bool Spectrum::isFlat() const
{
	return doIsFlat();
}


TSpectrumPtr Spectrum::make(TParam value)
{
	return TSpectrumPtr(new impl::SpectrumFlat(value));
}


TSpectrumPtr Spectrum::make(const XYZ& value)
{
	return TSpectrumPtr(new impl::SpectrumXYZ(value));
}


const TSpectrumPtr& Spectrum::white()
{
	return white_;
}


const TSpectrumPtr& Spectrum::black()
{
	return black_;
}


const TPyObjectPtr Spectrum::reduce() const
{
	return python::makeTuple(
		python::fromNakedToSharedPtrCast<PyObject>(reinterpret_cast<PyObject*>(this->_lassPyGetClassDef()->type())),
		python::makeTuple(), this->getState());
}


bool Spectrum::doIsFlat() const
{
	return false;
}


const TPyObjectPtr Spectrum::getState() const
{
	return doGetState();
}



void Spectrum::setState(const TPyObjectPtr& state)
{
	doSetState(state);
}


// --- protected -----------------------------------------------------------------------------------

Spectrum::Spectrum()
{
}



// --- private -------------------------------------------------------------------------------------



// --- impl ----------------------------------------------------------------------------------------

namespace impl
{

SpectrumFlat::SpectrumFlat(TParam value) :
	value_(value)
{
}

SpectrumFlat::TValue SpectrumFlat::value() const
{
	return value_;
}

const Spectral SpectrumFlat::doEvaluate(const Sample&, SpectralType type) const
{
	return Spectral(value_, type);
}

SpectrumFlat::TValue SpectrumFlat::doLuminance() const
{
	return value_;
}

bool SpectrumFlat::doIsFlat() const
{
	return true;
}

const TPyObjectPtr SpectrumFlat::doGetState() const
{
	return python::makeTuple(value_);
}

void SpectrumFlat::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, value_);
}


SpectrumXYZ::SpectrumXYZ(const XYZ& value) :
	value_(value)
{
}

const XYZ& SpectrumXYZ::value() const
{
	return value_;
}

const Spectral SpectrumXYZ::doEvaluate(const Sample& sample, SpectralType type) const
{
	return Spectral::fromXYZ(value_, sample, type);
}

SpectrumXYZ::TValue SpectrumXYZ::doLuminance() const
{
	return value_.y;
}

const TPyObjectPtr SpectrumXYZ::doGetState() const
{
	return python::makeTuple(value_);
}

void SpectrumXYZ::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, value_);
}

}
}
}

// EOF
