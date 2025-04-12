/** @file
*  @author Bram de Greve (bramz@users.sourceforge.net)
*
*  LiAR isn't a raytracer
*  Copyright (C) 2004-2025  Bram de Greve (bramz@users.sourceforge.net)
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
#include "recovery.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(Spectrum, "Abstract base class of spectrum definitionas")
	PY_CLASS_MEMBER_R(Spectrum, tristimulus)
	PY_CLASS_MEMBER_R(Spectrum, luminance)
	PY_CLASS_MEMBER_R(Spectrum, isFlat)
	PY_CLASS_METHOD_NAME(Spectrum, operator(), python::methods::_call_)
	PY_CLASS_METHOD_NAME(Spectrum, reduce, "__reduce__")
	PY_CLASS_METHOD_NAME(Spectrum, getState, "__getstate__")
	PY_CLASS_METHOD_NAME(Spectrum, setState, "__setstate__")

typedef impl::SpectrumFlat TSpectrumFlat;
PY_DECLARE_CLASS_NAME(TSpectrumFlat, "Flat");
PY_CLASS_INNER_CLASS_NAME(Spectrum, TSpectrumFlat, "Flat");
PY_CLASS_MEMBER_R(TSpectrumFlat, value)

TSpectrumRef Spectrum::white_(new TSpectrumFlat(1));
TSpectrumRef Spectrum::black_(new TSpectrumFlat(0));

typedef impl::SpectrumXYZ TSpectrumXYZ;
PY_DECLARE_CLASS_NAME(TSpectrumXYZ, "XYZ");
PY_CLASS_INNER_CLASS_NAME(Spectrum, TSpectrumXYZ, "XYZ");
PY_CLASS_MEMBER_R(TSpectrumXYZ, value)




	// --- public --------------------------------------------------------------------------------------

Spectrum::~Spectrum()
{
}



Spectrum::TValue Spectrum::operator()(TWavelength wavelength) const
{
	return doCall(wavelength);
}



Spectral Spectrum::evaluate([[maybe_unused]] const Sample& sample, SpectralType type) const
{
#if LIAR_SPECTRAL_SAMPLE_INDEPENDENT
	return Spectral(evaluated_, type);
#else
	Spectral s = doEvaluate(sample, type);
	LIAR_ASSERT(s.minimum() >= 0, "Spectrum::evaluate: negative value: " << s);
	return s;
#endif
}



XYZ Spectrum::tristimulus() const
{
	return tristimulus_;
}



Spectrum::TValue Spectrum::luminance() const
{
	return tristimulus_.y;
}


bool Spectrum::isFlat() const
{
	return doIsFlat();
}


TSpectrumRef Spectrum::make(TParam value)
{
	return TSpectrumRef(new impl::SpectrumFlat(value));
}


TSpectrumRef Spectrum::make(const XYZ& value)
{
	return TSpectrumRef(new impl::SpectrumXYZ(value));
}


const TSpectrumRef& Spectrum::white()
{
	return white_;
}


const TSpectrumRef& Spectrum::black()
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
	update();
}


// --- protected -----------------------------------------------------------------------------------

Spectrum::Spectrum()
{
}



void Spectrum::update()
{
	const auto ws = standardObserver().wavelengths();
	std::vector<TValue> values;
	values.reserve(ws.size());
	for (TWavelength w : ws)
	{
		values.push_back(doCall(w));
	}
#if LIAR_SPECTRAL_SAMPLE_INDEPENDENT
	evaluated_ = Spectral::fromSampled(ws, values, SpectralType::Illuminant);
#endif
	tristimulus_ = standardObserver().tristimulus(values);
}



// --- private -------------------------------------------------------------------------------------



// --- impl ----------------------------------------------------------------------------------------

namespace impl
{

SpectrumFlat::SpectrumFlat(TParam value) :
	value_(value)
{
#if LIAR_SPECTRAL_SAMPLE_INDEPENDENT
	evaluated_ = Spectral(value);
#endif
	tristimulus_ = value_;
}

SpectrumFlat::TValue SpectrumFlat::value() const
{
	return value_;
}

SpectrumFlat::TValue SpectrumFlat::doCall(TWavelength /* wavelength */) const
{
	return value_;
}

const Spectral SpectrumFlat::doEvaluate(const Sample&, SpectralType type) const
{
	return Spectral(value_, type);
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
#if LIAR_SPECTRAL_SAMPLE_INDEPENDENT
	update(); // still the best way to recover the spectrum
#endif
	tristimulus_ = value_;
}

const XYZ& SpectrumXYZ::value() const
{
	return value_;
}

SpectrumXYZ::TValue SpectrumXYZ::doCall(TWavelength wavelength) const
{
	return standardRecovery().recover(value_, wavelength);
}

const Spectral SpectrumXYZ::doEvaluate(const Sample& sample, SpectralType type) const
{
	return Spectral::fromXYZ(value_, sample, type);
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
