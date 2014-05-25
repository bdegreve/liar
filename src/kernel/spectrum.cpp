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

#if LIAR_FULL_SPECTRAL

Spectrum::Spectrum()
{
	std::fill(v_, v_ + numBands, 0);
}


Spectrum::Spectrum(TValue f)
{
	std::fill(v_, v_ + numBands, f);
}


Spectrum::Spectrum(const XYZ& xyz)
{
	const prim::ColorRGBA rgb = rgbSpace_->convert(xyz);
	if (rgb.r <= rgb.g && rgb.r <= rgb.b)
	{
		std::fill(v_, v_ + numBands, rgb.r);
		if (rgb.g <= rgb.b)
		{
			*this += (rgb.g - rgb.r) * cyan_;
			*this += (rgb.b - rgb.g) * blue_;
		}
		else
		{
			*this += (rgb.b - rgb.r) * cyan_;
			*this += (rgb.g - rgb.b) * green_;
		}
	}	
	else if (rgb.g <= rgb.b)
	{
		std::fill(v_, v_ + numBands, rgb.g);
		if (rgb.r <= rgb.b)
		{
			*this += (rgb.r - rgb.g) * magenta_;
			*this += (rgb.b - rgb.r) * blue_;
		}
		else
		{
			*this += (rgb.b - rgb.g) * magenta_;
			*this += (rgb.r - rgb.b) * red_;
		}
	}
	else
	{
		std::fill(v_, v_ + numBands, rgb.b);
		if (rgb.r <= rgb.g)
		{
			*this += (rgb.r - rgb.b) * yellow_;
			*this += (rgb.g - rgb.r) * green_;
		}
		else
		{
			*this += (rgb.g - rgb.b) * yellow_;
			*this += (rgb.r - rgb.g) * red_;
		}		
	}
}


const XYZ Spectrum::xyz() const
{
	XYZ sum = 0;
	for (size_t k = 0; k < numBands; ++k)
	{
		sum += observer_[k] * v_[k];
	}
	return sum;
}


Spectrum& Spectrum::operator+=(const Spectrum& other)
{
	for (size_t k = 0; k < numBands; ++k)
	{
		v_[k] += other.v_[k];
	}
	return *this;
}


Spectrum& Spectrum::operator-=(const Spectrum& other)
{
	for (size_t k = 0; k < numBands; ++k)
	{
		v_[k] -= other.v_[k];
	}
	return *this;
}


Spectrum& Spectrum::operator*=(const Spectrum& other)
{
	for (size_t k = 0; k < numBands; ++k)
	{
		v_[k] *= other.v_[k];
	}
	return *this;
}


Spectrum& Spectrum::operator/=(const Spectrum& other)
{
	for (size_t k = 0; k < numBands; ++k)
	{
		v_[k] *= other.v_[k];
	}
	return *this;
}



TScalar Spectrum::total() const
{
	TScalar sum = 0;
	for (size_t k = 0; k < numBands; ++k)
	{
		sum += v_[k];
	}
	return sum;
}



TScalar Spectrum::absTotal() const
{
	TScalar sum = 0;
	for (size_t k = 0; k < numBands; ++k)
	{
		sum += num::abs(v_[k]);
	}
	return sum;
}



bool Spectrum::isZero() const
{
	for (size_t k = 0; k < numBands; ++k)
	{
		if (v_[k] != 0)
		{
			return false;
		}
	}
	return true;
}


Spectrum& Spectrum::inplaceAbs()
{
	for (size_t k = 0; k < numBands; ++k)
	{
		v_[k] = num::abs(v_[k]);
	}
	return *this;
}


Spectrum& Spectrum::inplaceMax(const Spectrum& other)
{
	for (size_t k = 0; k < numBands; ++k)
	{
		v_[k] = std::max(v_[k], other.v_[k]);
	}
	return *this;
}


Spectrum& Spectrum::inplacePow(TScalar f)
{
	for (size_t k = 0; k < numBands; ++k)
	{
		v_[k] = std::pow(v_[k], f);
	}
	return *this;
}


Spectrum& Spectrum::inplaceExp()
{
	for (size_t k = 0; k < numBands; ++k)
	{
		v_[k] = std::exp(v_[k]);
	}
	return *this;
}


Spectrum& Spectrum::inplaceClamp(TScalar min, TScalar max)
{
	for (size_t k = 0; k < numBands; ++k)
	{
		v_[k] = num::clamp(v_[k], min, max);
	}
	return *this;
}


Spectrum& Spectrum::inplaceLerp(const Spectrum& other, TScalar f)
{
	for (size_t k = 0; k < numBands; ++k)
	{
		v_[k] = num::lerp(v_[k], other.v_[k], f);
	}
	return *this;
}


bool Spectrum::operator==(const Spectrum& other) const
{
	for (size_t k = 0; k < numBands; ++k)
	{
		if (v_[k] != other.v_[k])
		{
			return false;
		}
	}
	return true;
}


TScalar dot(const Spectrum& a, const Spectrum& b)
{
	TScalar sum = 0;
	for (size_t k = 0; k < Spectrum::numBands; ++k)
	{
		sum += a[k] * b[k];
	}
	return sum;
}


TRgbSpacePtr Spectrum::initData()
{
	/* start autogen block initData */
	// Generated by generate_spectrum_data.py with 10 bins from 3.6e-07 to 8e-07, using 100000 iterations
	TRgbSpacePtr rgbSpace(new RgbSpace(
	    TPoint2D(0.640000f, 0.330000f),
	    TPoint2D(0.300000f, 0.600000f),
	    TPoint2D(0.150000f, 0.0600000f),
	    TPoint2D(0.333333f, 0.333333f),
	    1.f));
	
	const XYZ A[10] =
	{
	    XYZ(0.00179643f, 5.03579e-05f, 0.00850556f),
	    XYZ(0.0855167f, 0.00471796f, 0.423513f),
	    XYZ(0.0787251f, 0.0429831f, 0.500759f),
	    XYZ(0.0263265f, 0.239723f, 0.0636773f),
	    XYZ(0.233830f, 0.396741f, 0.00321433f),
	    XYZ(0.404543f, 0.248992f, 0.000305128f),
	    XYZ(0.155119f, 0.0616462f, 8.50378e-06f),
	    XYZ(0.0134617f, 0.00490018f, 0.00000f),
	    XYZ(0.000648948f, 0.000234413f, 0.00000f),
	    XYZ(2.84654e-05f, 1.02801e-05f, 0.00000f),
	};
	
	const TWavelength w[11] =
	{
	    3.60000e-07f,
	    4.04000e-07f,
	    4.48000e-07f,
	    4.92000e-07f,
	    5.36000e-07f,
	    5.80000e-07f,
	    6.24000e-07f,
	    6.68000e-07f,
	    7.12000e-07f,
	    7.56000e-07f,
	    8.00000e-07f,
	};
	
	const TScalar yellow[10] =
	{
	    0.0178598f,
	    8.40471e-06f,
	    0.148979f,
	    0.911440f,
	    0.999954f,
	    0.998828f,
	    0.961676f,
	    0.932549f,
	    0.941175f,
	    0.957232f,
	};
	
	const TScalar magenta[10] =
	{
	    0.999124f,
	    0.999963f,
	    0.906628f,
	    0.0104402f,
	    0.0158742f,
	    0.813288f,
	    0.999843f,
	    0.999803f,
	    0.999587f,
	    0.999619f,
	};
	
	const TScalar cyan[10] =
	{
	    0.895247f,
	    0.923534f,
	    1.01807f,
	    1.01806f,
	    1.01805f,
	    0.191134f,
	    0.000884009f,
	    0.00152188f,
	    0.0179338f,
	    0.0418141f,
	};
	
	const TScalar red[10] =
	{
	    0.161772f,
	    0.0752262f,
	    -0.0179659f,
	    -0.0179478f,
	    -0.0179678f,
	    0.808297f,
	    1.00032f,
	    0.999017f,
	    1.00015f,
	    0.995732f,
	};
	
	const TScalar green[10] =
	{
	    0.00921844f,
	    9.40340e-05f,
	    0.0935372f,
	    0.986918f,
	    0.986582f,
	    0.185182f,
	    0.000670868f,
	    0.000211370f,
	    0.00854307f,
	    0.00399903f,
	};
	
	const TScalar blue[10] =
	{
	    0.999770f,
	    0.999964f,
	    0.850725f,
	    0.0889831f,
	    2.77556e-17f,
	    0.000348130f,
	    0.0412482f,
	    0.0580465f,
	    0.0625632f,
	    0.0817432f,
	};
	/* end autogen block initData */

	for (size_t k = 0; k < numBands; ++k)
	{
		observer_[k] = A[k];
		bands_[k] = w[k];
		yellow_[k] = yellow[k];
		magenta_[k] = magenta[k];
		cyan_[k] = cyan[k];
		red_[k] = red[k];
		green_[k] = green[k];
		blue_[k] = blue[k];
	}
	bands_[numBands] = w[numBands];
	
	return rgbSpace;
}

XYZ Spectrum::observer_[Spectrum::numBands] = {};
TWavelength Spectrum::bands_[Spectrum::numBands + 1] = {};
Spectrum Spectrum::yellow_;
Spectrum Spectrum::magenta_;
Spectrum Spectrum::cyan_;
Spectrum Spectrum::red_;
Spectrum Spectrum::green_;
Spectrum Spectrum::blue_;
TRgbSpacePtr Spectrum::rgbSpace_ = Spectrum::initData(); // must be the last one.

#endif

}
}

// EOF
