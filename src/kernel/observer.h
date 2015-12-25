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

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_KERNEL_OBSERVER_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_KERNEL_OBSERVER_H

#include "kernel_common.h"
#include "xyz.h"

namespace liar
{
namespace kernel
{

class Observer;
typedef python::PyObjectPtr<Observer>::Type TObserverPtr;

class LIAR_KERNEL_DLL Observer: public python::PyObjectPlus
{
	PY_HEADER(python::PyObjectPlus)
public:
	typedef XYZ::TValue TValue;
	typedef std::vector<TWavelength> TWavelengths;
	typedef std::vector<XYZ> TXYZs;
	typedef std::vector<TValue> TValues;

	Observer(const TWavelengths& wavelengths, const TXYZs& sensitivities);

	const TWavelengths& wavelengths() const;
	const TXYZs& sensitivities() const;

	TWavelength minWavelength() const;
	TWavelength maxWavelength() const;

	const XYZ sensitivity(TWavelength wavelength) const;

	const XYZ tristimulus(const TValues& spectrum) const;
	const XYZ tristimulus(const TWavelengths& wavelengths, const TValues& spectrum) const;
	template <typename Func>
	const XYZ tristimulusFunc(Func func) const
	{
		XYZ acc;
		for (size_t k = 0, n = w_.size(); k < n; ++k)
		{
			acc += dXYZ_[k] * func(w_[k]);
		}
		return acc;
	}

	TValue luminance(const TValues& spectrum) const;
	TValue luminance(const TWavelengths& wavelengths, const TValues& spectrum) const;
	template <typename Func>
	TValue luminanceFunc(Func func) const
	{
		TValue y = 0;
		for (size_t k = 0, n = w_.size(); k < n; ++k)
		{
			y += dXYZ_[k].y * func(w_[k]);
		}
		return y;
	}

	TWavelength sample(TScalar sample, TScalar& pdf) const;

	static const TObserverPtr& standard();
	static void setStandard(const TObserverPtr& standard);

private:
	TWavelengths w_;
	TXYZs xyz_;
	TXYZs dxyz_dw_;
	TXYZs dXYZ_;
	std::vector<TScalar> cdf_;

#if LIAR_SPECTRAL_MODE_BANDED
	friend class Spectral;
	const XYZ tristimulus(const Spectral& spectrum) const;
	XYZ xyzBands_[LIAR_SPECTRAL_MODE_BANDED];
#endif

	static TObserverPtr standard_;
};


LIAR_KERNEL_DLL const Observer& standardObserver();

}
}

#endif

// EOF
