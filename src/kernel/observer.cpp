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
#include "observer.h"
#include <lass/stde/extended_iterator.h>
#include <lass/stde/access_iterator.h>

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(Observer, "Conversion of spectral data to tristimulus")
PY_CLASS_MEMBER_R(Observer, wavelengthRange)
PY_CLASS_METHOD_QUALIFIED_1(Observer, tristimulus, const XYZ, TWavelength)
PY_CLASS_METHOD_QUALIFIED_1(Observer, tristimulus, const XYZ, Observer::TWavelengthRange)
PY_CLASS_METHOD_QUALIFIED_1(Observer, chromaticity, const XYZ, TWavelength)
PY_CLASS_METHOD_QUALIFIED_1(Observer, chromaticity, const XYZ, const Observer::TWavelengthRange&)
//PY_CLASS_METHOD(Observer, sample)

namespace
{

/** load the XYZ sensitivity curves of the CIE-1931 2deg standard observer.
 *
 *  The CIEXYZ colour space is defined using the 1931 2deg CIE Standard Colorimetric Observer Data, so that's the one we're using here.
 * 
 *  http://www.cis.rit.edu/research/mcsl2/online/CIE/StdObsFuncs.htm
 *  The curves are rescaled to have area 1 for all channels.
 */
Observer loadStandardObserver()
{
	const TWavelength w[] =
	{
		3.50e-07f,
		3.55e-07f,
		3.60e-07f,
		3.65e-07f,
		3.70e-07f,
		3.75e-07f,
		3.80e-07f,
		3.85e-07f,
		3.90e-07f,
		3.95e-07f,
		4.00e-07f,
		4.05e-07f,
		4.10e-07f,
		4.15e-07f,
		4.20e-07f,
		4.25e-07f,
		4.30e-07f,
		4.35e-07f,
		4.40e-07f,
		4.45e-07f,
		4.50e-07f,
		4.55e-07f,
		4.60e-07f,
		4.65e-07f,
		4.70e-07f,
		4.75e-07f,
		4.80e-07f,
		4.85e-07f,
		4.90e-07f,
		4.95e-07f,
		5.00e-07f,
		5.05e-07f,
		5.10e-07f,
		5.15e-07f,
		5.20e-07f,
		5.25e-07f,
		5.30e-07f,
		5.35e-07f,
		5.40e-07f,
		5.45e-07f,
		5.50e-07f,
		5.55e-07f,
		5.60e-07f,
		5.65e-07f,
		5.70e-07f,
		5.75e-07f,
		5.80e-07f,
		5.85e-07f,
		5.90e-07f,
		5.95e-07f,
		6.00e-07f,
		6.05e-07f,
		6.10e-07f,
		6.15e-07f,
		6.20e-07f,
		6.25e-07f,
		6.30e-07f,
		6.35e-07f,
		6.40e-07f,
		6.45e-07f,
		6.50e-07f,
		6.55e-07f,
		6.60e-07f,
		6.65e-07f,
		6.70e-07f,
		6.75e-07f,
		6.80e-07f,
		6.85e-07f,
		6.90e-07f,
		6.95e-07f,
		7.00e-07f,
		7.05e-07f,
		7.10e-07f,
		7.15e-07f,
		7.20e-07f,
		7.25e-07f,
		7.30e-07f,
		7.35e-07f,
		7.40e-07f,
		7.45e-07f,
		7.50e-07f,
		7.55e-07f,
		7.60e-07f,
		7.65e-07f,
		7.70e-07f,
		7.75e-07f,
		7.80e-07f,
		7.85e-07f,
		7.90e-07f,
		7.95e-07f,
		8.00e-07f,
		8.05e-07f,
		8.10e-07f,
		8.15e-07f,
		8.20e-07f,
		8.25e-07f,
		8.30e-07f,
		8.35e-07f,
		8.40e-07f,
	};
	const XYZ xyz[] =
	{
		XYZ(0.0000e+00f, 0.0000e+00f, 0.0000e+00f),
		XYZ(0.0000e+00f, 0.0000e+00f, 0.0000e+00f),
		XYZ(1.2155e+03f, 3.6656e+01f, 5.6701e+03f),
		XYZ(2.1719e+03f, 6.5181e+01f, 1.0160e+04f),
		XYZ(3.8824e+03f, 1.1595e+02f, 1.8205e+04f),
		XYZ(6.9396e+03f, 2.0607e+02f, 3.2612e+04f),
		XYZ(1.2801e+04f, 3.6497e+02f, 6.0341e+04f),
		XYZ(2.0923e+04f, 5.9893e+02f, 9.8696e+04f),
		XYZ(3.9704e+04f, 1.1230e+03f, 1.8757e+05f),
		XYZ(7.1585e+04f, 2.0308e+03f, 3.3875e+05f),
		XYZ(1.3391e+05f, 3.7059e+03f, 6.3475e+05f),
		XYZ(2.1700e+05f, 5.9893e+03f, 1.0309e+06f),
		XYZ(4.0715e+05f, 1.1324e+04f, 1.9403e+06f),
		XYZ(7.2643e+05f, 2.0401e+04f, 3.4736e+06f),
		XYZ(1.2575e+06f, 3.7433e+04f, 6.0397e+06f),
		XYZ(2.0097e+06f, 6.8316e+04f, 9.7204e+06f),
		XYZ(2.6566e+06f, 1.0856e+05f, 1.2962e+07f),
		XYZ(3.0740e+06f, 1.5759e+05f, 1.5183e+07f),
		XYZ(3.2590e+06f, 2.1524e+05f, 1.6344e+07f),
		XYZ(3.2570e+06f, 2.7888e+05f, 1.6676e+07f),
		XYZ(3.1460e+06f, 3.5562e+05f, 1.6578e+07f),
		XYZ(2.9822e+06f, 4.4920e+05f, 1.6316e+07f),
		XYZ(2.7212e+06f, 5.6150e+05f, 1.5616e+07f),
		XYZ(2.3497e+06f, 6.9158e+05f, 1.4296e+07f),
		XYZ(1.8281e+06f, 8.5142e+05f, 1.2046e+07f),
		XYZ(1.3297e+06f, 1.0537e+06f, 9.7471e+06f),
		XYZ(8.9496e+05f, 1.3010e+06f, 7.6052e+06f),
		XYZ(5.4227e+05f, 1.5844e+06f, 5.7646e+06f),
		XYZ(2.9953e+05f, 1.9467e+06f, 4.3518e+06f),
		XYZ(1.3756e+05f, 2.4201e+06f, 3.3052e+06f),
		XYZ(4.5852e+04f, 3.0227e+06f, 2.5446e+06f),
		XYZ(2.2458e+04f, 3.8116e+06f, 1.9861e+06f),
		XYZ(8.7025e+04f, 4.7072e+06f, 1.4800e+06f),
		XYZ(2.7230e+05f, 5.6917e+06f, 1.0450e+06f),
		XYZ(5.9205e+05f, 6.6444e+06f, 7.3204e+05f),
		XYZ(1.0256e+06f, 7.4230e+06f, 5.3558e+05f),
		XYZ(1.5487e+06f, 8.0669e+06f, 3.9441e+05f),
		XYZ(2.1125e+06f, 8.5614e+06f, 2.7916e+05f),
		XYZ(2.7174e+06f, 8.9278e+06f, 1.8991e+05f),
		XYZ(3.3659e+06f, 9.1739e+06f, 1.2536e+05f),
		XYZ(4.0560e+06f, 9.3110e+06f, 8.1857e+04f),
		XYZ(4.7915e+06f, 9.3583e+06f, 5.3792e+04f),
		XYZ(5.5631e+06f, 9.3115e+06f, 3.6485e+04f),
		XYZ(6.3482e+06f, 9.1580e+06f, 2.5727e+04f),
		XYZ(7.1314e+06f, 8.9091e+06f, 1.9646e+04f),
		XYZ(7.8837e+06f, 8.5666e+06f, 1.6839e+04f),
		XYZ(8.5743e+06f, 8.1417e+06f, 1.5436e+04f),
		XYZ(9.1573e+06f, 7.6392e+06f, 1.3097e+04f),
		XYZ(9.6036e+06f, 7.0842e+06f, 1.0291e+04f),
		XYZ(9.8881e+06f, 6.5031e+06f, 9.3551e+03f),
		XYZ(9.9396e+06f, 5.9051e+06f, 7.4841e+03f),
		XYZ(9.7842e+06f, 5.3043e+06f, 5.6131e+03f),
		XYZ(9.3819e+06f, 4.7072e+06f, 3.1807e+03f),
		XYZ(8.7811e+06f, 4.1289e+06f, 2.2452e+03f),
		XYZ(7.9955e+06f, 3.5655e+06f, 1.7775e+03f),
		XYZ(7.0313e+06f, 3.0040e+06f, 9.3551e+02f),
		XYZ(6.0113e+06f, 2.4799e+06f, 4.6776e+02f),
		XYZ(5.0709e+06f, 2.0308e+06f, 2.8065e+02f),
		XYZ(4.1912e+06f, 1.6377e+06f, 1.8710e+02f),
		XYZ(3.3762e+06f, 1.2933e+06f, 9.3551e+01f),
		XYZ(2.6529e+06f, 1.0013e+06f, 0.0000e+00f),
		XYZ(2.0465e+06f, 7.6364e+05f, 0.0000e+00f),
		XYZ(1.5431e+06f, 5.7086e+05f, 0.0000e+00f),
		XYZ(1.1341e+06f, 4.1719e+05f, 0.0000e+00f),
		XYZ(8.1785e+05f, 2.9947e+05f, 0.0000e+00f),
		XYZ(5.9514e+05f, 2.1711e+05f, 0.0000e+00f),
		XYZ(4.3765e+05f, 1.5909e+05f, 0.0000e+00f),
		XYZ(3.0786e+05f, 1.1155e+05f, 0.0000e+00f),
		XYZ(2.1242e+05f, 7.6832e+04f, 0.0000e+00f),
		XYZ(1.4822e+05f, 5.3558e+04f, 0.0000e+00f),
		XYZ(1.0629e+05f, 3.8388e+04f, 0.0000e+00f),
		XYZ(7.5898e+04f, 2.7410e+04f, 0.0000e+00f),
		XYZ(5.4183e+04f, 1.9568e+04f, 0.0000e+00f),
		XYZ(3.8426e+04f, 1.3888e+04f, 0.0000e+00f),
		XYZ(2.7131e+04f, 9.7981e+03f, 0.0000e+00f),
		XYZ(1.9175e+04f, 6.9251e+03f, 0.0000e+00f),
		XYZ(1.3475e+04f, 4.8663e+03f, 0.0000e+00f),
		XYZ(9.3571e+03f, 3.3793e+03f, 0.0000e+00f),
		XYZ(6.4574e+03f, 2.3321e+03f, 0.0000e+00f),
		XYZ(4.4544e+03f, 1.6087e+03f, 0.0000e+00f),
		XYZ(3.1095e+03f, 1.1230e+03f, 0.0000e+00f),
		XYZ(2.1974e+03f, 7.9358e+02f, 0.0000e+00f),
		XYZ(1.5548e+03f, 5.6150e+02f, 0.0000e+00f),
		XYZ(1.0987e+03f, 3.9679e+02f, 0.0000e+00f),
		XYZ(7.7738e+02f, 2.8075e+02f, 0.0000e+00f),
		XYZ(5.4935e+02f, 1.9840e+02f, 0.0000e+00f),
		XYZ(3.8843e+02f, 1.4028e+02f, 0.0000e+00f),
		XYZ(2.7467e+02f, 9.9198e+01f, 0.0000e+00f),
		XYZ(1.9346e+02f, 6.9866e+01f, 0.0000e+00f),
		XYZ(1.3624e+02f, 4.9204e+01f, 0.0000e+00f),
		XYZ(9.5952e+01f, 3.4653e+01f, 0.0000e+00f),
		XYZ(6.7575e+01f, 2.4405e+01f, 0.0000e+00f),
		XYZ(4.7591e+01f, 1.7187e+01f, 0.0000e+00f),
		XYZ(3.3515e+01f, 1.2104e+01f, 0.0000e+00f),
		XYZ(2.3605e+01f, 8.5248e+00f, 0.0000e+00f),
		XYZ(1.6624e+01f, 6.0036e+00f, 0.0000e+00f),
		XYZ(1.1708e+01f, 4.2282e+00f, 0.0000e+00f),
		XYZ(0.0000e+00f, 0.0000e+00f, 0.0000e+00f),
		XYZ(0.0000e+00f, 0.0000e+00f, 0.0000e+00f),
	};
	LASS_ASSERT(sizeof w / sizeof(TScalar) == sizeof xyz / sizeof(XYZ));
	return Observer(w, w + sizeof w / sizeof(TScalar), xyz);
}

template <typename It>
struct cdf_accessor_t: std::unary_function<It, TScalar>
{
	typedef const TScalar value_type;
	typedef const TScalar reference;
	typedef const TScalar* pointer;
	cdf_accessor_t(const XYZ& weight = XYZ()): weight_(weight) {}
	TScalar operator()(It i) const
	{
		return dot(weight_, i->cdf); 
	}
private:
	XYZ weight_;
};

template <typename It>
stde::access_iterator_t< It, cdf_accessor_t<It> > cdf_iterator(It i, const XYZ& weight)
{
	return stde::access_iterator_t< It, cdf_accessor_t<It> >(i, cdf_accessor_t<It>(weight));
}

}



// --- public ----------------------------------------------------------------


const Observer::TWavelengthRange Observer::wavelengthRange() const
{
	return std::make_pair(nodes_.front().wavelength, nodes_.back().wavelength);
}



const XYZ Observer::tristimulus(TWavelength wavelength) const
{
	const TNodes::const_iterator i = std::lower_bound(
		stde::const_member_iterator(nodes_.begin(), &Node::wavelength),
		stde::const_member_iterator(nodes_.end(), &Node::wavelength),
		wavelength).base();
	const Node& node = *(i == nodes_.begin() ? i : stde::prev(i));
	const TWavelength dw = wavelength - node.wavelength;
	LASS_ASSERT(dw >= 0);
	return node.xyz + node.dxyz_dw * dw;
}



const XYZ Observer::tristimulus(TWavelengthRange wavelength) const
{
	if (wavelength.second < wavelength.first)
	{
		std::swap(wavelength.first, wavelength.second);
	}

	const TNodes::const_iterator i1 = std::lower_bound(
		stde::const_member_iterator(nodes_.begin(), &Node::wavelength),
		stde::const_member_iterator(nodes_.end(), &Node::wavelength),
		wavelength.first).base();

	const TNodes::const_iterator i2 = std::lower_bound(
		stde::const_member_iterator(i1, &Node::wavelength),
		stde::const_member_iterator(nodes_.end(), &Node::wavelength),
		wavelength.second).base();

	const Node& node1 = *(i1 == nodes_.begin() ? i1 : stde::prev(i1));
	const Node& node2 = *(i2 == nodes_.begin() ? i2 : stde::prev(i2));

	return node2.integrate(wavelength.second) - node1.integrate(wavelength.first);
}



const XYZ Observer::chromaticity(TWavelength wavelength) const
{
	return kernel::chromaticity(tristimulus(wavelength));
}



const XYZ Observer::chromaticity(const TWavelengthRange& wavelength) const
{
	return kernel::chromaticity(tristimulus(wavelength));
}



TWavelength Observer::sample(const XYZ& weight, TScalar sample, XYZ& chromaticity, TScalar& pdf) const
{
	const XYZ w = kernel::chromaticity(weight);
	TNodes::const_iterator i = std::lower_bound(
		cdf_iterator(nodes_.begin(), w),
		cdf_iterator(nodes_.end(), w),
		sample).base();
	const Node& node = *(i == nodes_.begin() ? i : stde::prev(i));
	const TScalar p = dot(node.xyz, w);
	LASS_ASSERT(p >= 0);
	const TScalar c = dot(node.cdf, w);
	LASS_ASSERT(c <= sample);
	const TScalar dWavelength = p > 0 ? (sample - c) / p : 0;
	const TScalar wavelength = node.wavelength + dWavelength;
	const XYZ xyz = node.interpolate(wavelength);
	chromaticity = kernel::chromaticity(xyz);	
	pdf = p;
	return wavelength;
}


// --- private ----------------------------------------------------------------

void Observer::init()
{
	for (TNodes::iterator i = nodes_.begin(), last = stde::prev(nodes_.end()); i != last; ++i)
	{
		Node& node = *i;
		const Node& next = *stde::next(i);
		const TWavelength dw = next.wavelength - node.wavelength;
		node.dxyz_dw = (next.xyz - node.xyz) / dw;
	}
	nodes_.back().dxyz_dw = XYZ();

	nodes_.front().cdf = XYZ();
	for (TNodes::iterator i = stde::next(nodes_.begin()), last = nodes_.end(); i != last; ++i)
	{
		Node& node = *i;
		const Node& prev = *stde::prev(i);
		const TWavelength dw = node.wavelength - prev.wavelength;
		node.cdf = prev.cdf + (prev.xyz + node.xyz) * (dw / 2);
	}
}



const XYZ Observer::Node::interpolate(TWavelength w) const
{
	const TWavelength dw = w - wavelength;
	LASS_ASSERT(dw >= 0);
	return xyz + dxyz_dw * dw;
}



const XYZ Observer::Node::integrate(TWavelength w) const
{
	const TWavelength dw = w - wavelength;
	LASS_ASSERT(dw >= 0);
	return cdf + (xyz + dxyz_dw * (dw / 2)) * dw;
}



// --- free --------------------------------------------------------------------

const Observer& standardObserver()
{
	static const Observer observer = loadStandardObserver();
	return observer;
}

const Observer& LASS_UNUSED(forceLoadingAtStartUp) = standardObserver();

/*
const XYZ chromaticity(TFrequency frequency)
{
	return chromaticity(tristimulus(frequency));
}

const XYZ tristimulus(TFrequency frequency)
{
	return standardObserver().tristimulus(frequency);
}

TFrequency sampleFrequency(const XYZ& power, TScalar sample, XYZ& chromaticity, TScalar& pdf)
{
	return standardObserver().sample(power, sample, chromaticity, pdf);
}
*/

}

}

// EOF
