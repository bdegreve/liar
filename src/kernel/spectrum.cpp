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

#include "kernel_common.h"
#include "spectrum.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(Spectrum, "Abstract base class of spectrum definitionas")
	PY_CLASS_METHOD_NAME(Spectrum, reduce, "__reduce__")
	PY_CLASS_METHOD_NAME(Spectrum, getState, "__getstate__")
	PY_CLASS_METHOD_NAME(Spectrum, setState, "__setstate__")

	typedef impl::SpectrumFlat TSpectrumFlat;
PY_DECLARE_CLASS_NAME(TSpectrumFlat, "SpectrumFlat");
PY_CLASS_INNER_CLASS_NAME(Spectrum, TSpectrumFlat, "Flat");
PY_CLASS_MEMBER_R(TSpectrumFlat, value)

	TSpectrumPtr Spectrum::white_(new TSpectrumFlat(1));
TSpectrumPtr Spectrum::black_(new TSpectrumFlat(0));

typedef impl::SpectrumXYZ TSpectrumXYZ;
PY_DECLARE_CLASS_NAME(TSpectrumXYZ, "SpectrumXYZ");
PY_CLASS_INNER_CLASS_NAME(Spectrum, TSpectrumXYZ, "XYZ");
PY_CLASS_MEMBER_R(TSpectrumXYZ, value)




	// --- public --------------------------------------------------------------------------------------

	Spectrum::~Spectrum()
{
}


TSpectrumPtr Spectrum::make(TScalar value)
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

SpectrumFlat::SpectrumFlat(TScalar value) :
	value_(value)
{
}

TScalar SpectrumFlat::value() const
{
	return value_;
}

const Spectral SpectrumFlat::doEvaluate(const Sample&) const
{
	return Spectral(value_);
}

TScalar SpectrumFlat::doAbsAverage() const
{
	return num::abs(value_);
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

const Spectral SpectrumXYZ::doEvaluate(const Sample& sample) const
{
	return Spectral::fromXYZ(value_, sample);
}

TScalar SpectrumXYZ::doAbsAverage() const
{
	return Spectral::absAverageFromXYZ(value_);
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
