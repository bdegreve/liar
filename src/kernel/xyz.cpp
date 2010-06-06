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
#include "xyz.h"
#include <lass/stde/extended_iterator.h>
#include <lass/stde/access_iterator.h>

namespace liar
{
namespace kernel
{
namespace impl
{

/** load the XYZ sensitivity curves of the CIE-1964 standard observer.
 * 
 *  http://www.isc.tamu.edu/~astro/color/cie_xyz1964.html
 *  The curves are rescaled to have area 1 for all channels.
 */
Observer loadStandardObserver()
{
	const TScalar w[] =
	{
		3.70e-007f,
		3.75e-007f,
		3.80e-007f,
		3.85e-007f,
		3.90e-007f,
		3.95e-007f,
		4.00e-007f,
		4.05e-007f,
		4.10e-007f,
		4.15e-007f,
		4.20e-007f,
		4.25e-007f,
		4.30e-007f,
		4.35e-007f,
		4.40e-007f,
		4.45e-007f,
		4.50e-007f,
		4.55e-007f,
		4.60e-007f,
		4.65e-007f,
		4.70e-007f,
		4.75e-007f,
		4.80e-007f,
		4.85e-007f,
		4.90e-007f,
		4.95e-007f,
		5.00e-007f,
		5.05e-007f,
		5.10e-007f,
		5.15e-007f,
		5.20e-007f,
		5.25e-007f,
		5.30e-007f,
		5.35e-007f,
		5.40e-007f,
		5.45e-007f,
		5.50e-007f,
		5.55e-007f,
		5.60e-007f,
		5.65e-007f,
		5.70e-007f,
		5.75e-007f,
		5.80e-007f,
		5.85e-007f,
		5.90e-007f,
		5.95e-007f,
		6.00e-007f,
		6.05e-007f,
		6.10e-007f,
		6.15e-007f,
		6.20e-007f,
		6.25e-007f,
		6.30e-007f,
		6.35e-007f,
		6.40e-007f,
		6.45e-007f,
		6.50e-007f,
		6.55e-007f,
		6.60e-007f,
		6.65e-007f,
		6.70e-007f,
		6.75e-007f,
		6.80e-007f,
		6.85e-007f,
		6.90e-007f,
		6.95e-007f,
		7.00e-007f,
		7.05e-007f,
		7.10e-007f,
		7.15e-007f,
		7.20e-007f,
		7.25e-007f,
		7.30e-007f,
		7.35e-007f,
		7.40e-007f,
		7.45e-007f,
		7.50e-007f,
		7.55e-007f,
		7.60e-007f,
		7.65e-007f,
		7.70e-007f,
		7.75e-007f,
		7.80e-007f,
	};
	const XYZ xyz[] =
	{
		XYZ(0.0000e000f, 0.0000e000f, 0.0000e000f),
		XYZ(0.0000e000f, 0.0000e000f, 0.0000e000f),
		XYZ(1.7146e003f, 0.0000e000f, 5.9997e003f),
		XYZ(6.0010e003f, 8.5718e002f, 2.4856e004f),
		XYZ(2.0575e004f, 2.5715e003f, 8.9996e004f),
		XYZ(6.1725e004f, 6.8574e003f, 2.7684e005f),
		XYZ(1.6374e005f, 1.7144e004f, 7.3711e005f),
		XYZ(3.7206e005f, 3.8573e004f, 1.6894e006f),
		XYZ(7.2612e005f, 7.5432e004f, 3.3376e006f),
		XYZ(1.2053e006f, 1.2429e005f, 5.6295e006f),
		XYZ(1.7532e006f, 1.8344e005f, 8.3353e006f),
		XYZ(2.2692e006f, 2.5287e005f, 1.0992e007f),
		XYZ(2.6979e006f, 3.3173e005f, 1.3315e007f),
		XYZ(3.0665e006f, 4.2516e005f, 1.5415e007f),
		XYZ(3.2894e006f, 5.3231e005f, 1.6862e007f),
		XYZ(3.3151e006f, 6.4031e005f, 1.7376e007f),
		XYZ(3.1780e006f, 7.6717e005f, 1.7098e007f),
		XYZ(2.9405e006f, 9.1118e005f, 1.6291e007f),
		XYZ(2.5916e006f, 1.0989e006f, 1.4960e007f),
		XYZ(2.1784e006f, 1.3098e006f, 1.3327e007f),
		XYZ(1.6769e006f, 1.5875e006f, 1.1293e007f),
		XYZ(1.1342e006f, 1.8849e006f, 8.8299e006f),
		XYZ(6.9012e005f, 2.1738e006f, 6.6177e006f),
		XYZ(3.5235e005f, 2.5518e006f, 4.8863e006f),
		XYZ(1.3888e005f, 2.9067e006f, 3.5596e006f),
		XYZ(4.3722e004f, 3.3893e006f, 2.5919e006f),
		XYZ(3.2577e004f, 3.9499e006f, 1.8728e006f),
		XYZ(1.3202e005f, 4.5550e006f, 1.3645e006f),
		XYZ(3.2148e005f, 5.2005e006f, 9.5996e005f),
		XYZ(6.1210e005f, 5.8777e006f, 7.0454e005f),
		XYZ(1.0090e006f, 6.5300e006f, 5.2026e005f),
		XYZ(1.4831e006f, 7.0571e006f, 3.6941e005f),
		XYZ(2.0275e006f, 7.5020e006f, 2.6142e005f),
		XYZ(2.6079e006f, 7.9186e006f, 1.7656e005f),
		XYZ(3.2303e006f, 8.2460e006f, 1.1742e005f),
		XYZ(3.8715e006f, 8.4192e006f, 6.7711e004f),
		XYZ(4.5419e006f, 8.5015e006f, 3.4284e004f),
		XYZ(5.2817e006f, 8.5641e006f, 9.4281e003f),
		XYZ(6.0456e006f, 8.5486e006f, 0.0000e000f),
		XYZ(6.8051e006f, 8.4209e006f, 0.0000e000f),
		XYZ(7.5330e006f, 8.1912e006f, 0.0000e000f),
		XYZ(8.1545e006f, 7.8449e006f, 0.0000e000f),
		XYZ(8.6946e006f, 7.4480e006f, 0.0000e000f),
		XYZ(9.2098e006f, 7.0769e006f, 0.0000e000f),
		XYZ(9.5888e006f, 6.6637e006f, 0.0000e000f),
		XYZ(9.7242e006f, 6.1751e006f, 0.0000e000f),
		XYZ(9.6359e006f, 5.6428e006f, 0.0000e000f),
		XYZ(9.3367e006f, 5.0908e006f, 0.0000e000f),
		XYZ(8.8343e006f, 4.5259e006f, 0.0000e000f),
		XYZ(8.1502e006f, 3.9584e006f, 0.0000e000f),
		XYZ(7.3410e006f, 3.4124e006f, 0.0000e000f),
		XYZ(6.4717e006f, 2.9110e006f, 0.0000e000f),
		XYZ(5.5509e006f, 2.4301e006f, 0.0000e000f),
		XYZ(4.5873e006f, 1.9569e006f, 0.0000e000f),
		XYZ(3.7001e006f, 1.5412e006f, 0.0000e000f),
		XYZ(2.9465e006f, 1.2018e006f, 0.0000e000f),
		XYZ(2.3001e006f, 9.2232e005f, 0.0000e000f),
		XYZ(1.7514e006f, 6.9603e005f, 0.0000e000f),
		XYZ(1.3082e006f, 5.1688e005f, 0.0000e000f),
		XYZ(9.6188e005f, 3.7802e005f, 0.0000e000f),
		XYZ(6.9697e005f, 2.7258e005f, 0.0000e000f),
		XYZ(4.9637e005f, 1.9372e005f, 0.0000e000f),
		XYZ(3.5063e005f, 1.3629e005f, 0.0000e000f),
		XYZ(2.4518e005f, 9.5147e004f, 0.0000e000f),
		XYZ(1.7060e005f, 6.6003e004f, 0.0000e000f),
		XYZ(1.1831e005f, 4.6288e004f, 0.0000e000f),
		XYZ(8.2300e004f, 3.1716e004f, 0.0000e000f),
		XYZ(5.6581e004f, 2.2287e004f, 0.0000e000f),
		XYZ(3.9435e004f, 1.5429e004f, 0.0000e000f),
		XYZ(2.6576e004f, 1.0286e004f, 0.0000e000f),
		XYZ(1.8860e004f, 6.8574e003f, 0.0000e000f),
		XYZ(1.2859e004f, 5.1431e003f, 0.0000e000f),
		XYZ(8.5729e003f, 3.4287e003f, 0.0000e000f),
		XYZ(6.0010e003f, 2.5715e003f, 0.0000e000f),
		XYZ(4.2864e003f, 1.7144e003f, 0.0000e000f),
		XYZ(3.4291e003f, 8.5718e002f, 0.0000e000f),
		XYZ(2.5719e003f, 8.5718e002f, 0.0000e000f),
		XYZ(1.7146e003f, 8.5718e002f, 0.0000e000f),
		XYZ(8.5729e002f, 0.0000e000f, 0.0000e000f),
		XYZ(8.5729e002f, 0.0000e000f, 0.0000e000f),
		XYZ(8.5729e002f, 0.0000e000f, 0.0000e000f),
		XYZ(0.0000e000f, 0.0000e000f, 0.0000e000f),
		XYZ(0.0000e000f, 0.0000e000f, 0.0000e000f),
	};

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

namespace 
{
	const TFrequency c0 = 299792458; // speed of light in vacuum
}

void Observer::init()
{
	nodes_.front().cdf = XYZ();
	for (TNodes::iterator i = stde::next(nodes_.begin()); i != nodes_.end(); ++i)
	{
		Node& node = *i;
		const Node& prev = *stde::prev(i);
		node.cdf = prev.cdf + prev.xyz * (node.wavelength - prev.wavelength);
	}
	for (TNodes::iterator i = nodes_.begin(); i != stde::prev(nodes_.end()); ++i)
	{
		Node& node = *i;
		const Node& next = *stde::next(i);
		node.dxyz_dw = (next.xyz - node.xyz) / (next.wavelength - node.wavelength);
	}
	nodes_.back().dxyz_dw = XYZ();
}

const Observer::TFrequencyRange Observer::frequencyRange() const
{
	return std::make_pair(c0 / nodes_.front().wavelength, c0 / nodes_.back().wavelength);
}

const XYZ Observer::tristimulus(TFrequency frequency) const
{
	const TScalar wavelength = c0 / frequency;
	typedef const Node TConstNode;
	const TNodes::const_iterator i = std::lower_bound(
		stde::const_member_iterator(nodes_.begin(), &Node::wavelength),
		stde::const_member_iterator(nodes_.end(), &Node::wavelength),
		wavelength).base();
	const Node& node = *(i == nodes_.begin() ? i : stde::prev(i));
	return node.xyz + node.dxyz_dw * (wavelength - node.wavelength);
}

const XYZ Observer::chromaticity(TFrequency frequency) const
{
	return kernel::chromaticity(tristimulus(frequency));
}

TFrequency Observer::sample(const XYZ& weight, TScalar sample, XYZ& chromaticity, TScalar& pdf) const
{
	const XYZ w = kernel::chromaticity(weight);
	TNodes::const_iterator i = std::lower_bound(
		impl::cdf_iterator(nodes_.begin(), w),
		impl::cdf_iterator(nodes_.end(), w),
		sample).base();
	const Node& node = *(i == nodes_.begin() ? i : stde::prev(i));
	const TScalar p = dot(node.xyz, w);
	LASS_ASSERT(p >= 0);
	const TScalar c = dot(node.cdf, w);
	LASS_ASSERT(c <= sample);
	const TScalar dWavelength = p > 0 ? (sample - c) / p : 0;
	const TScalar wavelength = node.wavelength + dWavelength;
	const XYZ xyz = node.xyz + node.dxyz_dw * dWavelength;
	chromaticity = kernel::chromaticity(xyz);	
	pdf = p;
	return c0 / wavelength;
}

const XYZ chromaticity(const XYZ& xyz)
{
	const TScalar sum = xyz.x + xyz.y + xyz.z;
	if (sum == 0)
	{
		return 0;
	}
	return xyz / sum;	
}

const Observer& standardObserver()
{
	static const Observer observer = impl::loadStandardObserver();
	return observer;
}

const Observer& forceLoadingAtStartUp = standardObserver();

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

}

}

// EOF
