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

/** @class liar::Spectrum
*  @brief abstract base class of all spectrum definitions
*  @author Bram de Greve [Bramz]
*/

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SPECTRUM_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SPECTRUM_H

#include "kernel_common.h"
#include "spectral.h"
#include "xyz.h"

namespace liar
{
namespace kernel
{
namespace impl
{
	class SpectrumFlat;
	class SpectrumXYZ;
}

class Sample;

class Spectrum;
typedef PyObjectRef<Spectrum> TSpectrumRef;

class LIAR_KERNEL_DLL Spectrum: public python::PyObjectPlus
{
	PY_HEADER(python::PyObjectPlus)
public:

	typedef Spectral::TValue TValue;
	typedef Spectral::TParam TParam;

	virtual ~Spectrum();

	TValue operator()(TWavelength wavelength) const;

	Spectral evaluate(const Sample& sample, SpectralType type) const;
	XYZ tristimulus() const;
	TValue luminance() const;
	bool isFlat() const;

	static TSpectrumRef make(TParam value);
	static TSpectrumRef make(const XYZ& value);
	static const TSpectrumRef& white();
	static const TSpectrumRef& black();


	const TPyObjectPtr reduce() const;
	const TPyObjectPtr getState() const;
	void setState(const TPyObjectPtr& state);

protected:
	Spectrum();

	void update();

private:
	virtual TValue doCall(TWavelength wavelength) const = 0;
	virtual const Spectral doEvaluate(const Sample& sample, SpectralType type) const = 0;
	virtual bool doIsFlat() const;

	virtual const TPyObjectPtr doGetState() const = 0;
	virtual void doSetState(const TPyObjectPtr& state) = 0;

	friend class impl::SpectrumFlat;
	friend class impl::SpectrumXYZ;

	static TSpectrumRef white_;
	static TSpectrumRef black_;

#if LIAR_SPECTRAL_SAMPLE_INDEPENDENT
	Spectral evaluated_;
#endif
	XYZ tristimulus_;
};


namespace impl
{

class LIAR_KERNEL_DLL SpectrumFlat : public Spectrum
{
	PY_HEADER(Spectrum)
public:
	explicit SpectrumFlat(Spectral::TParam value);
	Spectral::TValue value() const;
private:
	TValue doCall(TWavelength wavelength) const override;
	const Spectral doEvaluate(const Sample& sample, SpectralType type) const override;
	bool doIsFlat() const override;
	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;
	TValue value_;
};


class LIAR_KERNEL_DLL SpectrumXYZ : public Spectrum
{
	PY_HEADER(Spectrum)
public:
	explicit SpectrumXYZ(const XYZ& value);
	const XYZ& value() const;
private:
	TValue doCall(TWavelength wavelength) const override;
	const Spectral doEvaluate(const Sample& sample, SpectralType type) const override;
	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;
	XYZ value_;
};


}
}
}

#endif

// EOF
