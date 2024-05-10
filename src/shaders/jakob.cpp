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

#include "shaders_common.h"
#include "jakob.h"
#include "../kernel/rgb_space.h"
#include <lass/io/binary_i_memory_map.h>
#include <lass/python/export_traits_filesystem.h>

namespace liar
{
namespace shaders
{
namespace
{

constexpr size_t JAKOB_IDENTIFIER_LENGTH = 7;
constexpr const char JAKOB_IDENTIFIER[JAKOB_IDENTIFIER_LENGTH + 1] = "SCATFUN";
constexpr int JAKOB_VERSION = 1;

enum class JakobFlags: num::Tuint32
{
	BSDF = 1,
	Extrapolated = 2,
};

struct JakobHeader
{
	num::Tuint8 identifier[JAKOB_IDENTIFIER_LENGTH];
	num::Tuint8 version;
	num::Tuint32 flags;
	num::Tuint32 numNodes;
	num::Tuint32 numCoefficients;
	num::Tuint32 maxOrder;
	num::Tuint32 numChannels;
	num::Tuint32 numBases;
	num::Tuint32 numMetadataBytes;
	num::Tuint32 numParameters;
	num::Tuint32 numParameterValues;
	num::Tfloat32 eta;
	num::Tfloat32 alpha[2];
	num::Tfloat32 unused[2];
};

}

PY_DECLARE_CLASS_DOC(Jakob,
	"Fourier-based model by Wenzel Jakob et al. (2014)\n"
	"\n"
	"Jakob(path)"
)
PY_CLASS_CONSTRUCTOR_1(Jakob, std::filesystem::path)
PY_CLASS_MEMBER_R_DOC(Jakob, nodes, "values of mu for which the fourier coefficients are tabulated")
PY_CLASS_MEMBER_R_DOC(Jakob, numChannels, "number of coefficient channels (only possible values: 1 or 3)")
PY_CLASS_METHOD_DOC(Jakob, coefficients,
	"coefficients(indexIn: int, indexOut: int, channel: int)\n"
	"\n"
	"get the coefficients for a given pair of mu values"
)
PY_CLASS_MEMBER_RW_DOC(Jakob, numberOfSamples, setNumberOfSamples, "number of samples for Monte Carlo simulations")

// --- public --------------------------------------------------------------------------------------

Jakob::Jakob(const std::filesystem::path& path):
	Shader(BsdfCaps::allGlossy),
	numberOfSamples_(1)
{
	lass::io::BinaryIMemoryMap stream(path);
	load(stream);
}



Jakob::Jakob(std::vector<TValue> nodes, std::vector<TValue> cdf, std::vector<size_t> offsets, std::vector<size_t> lengths, std::vector<TValue> coefficients, size_t numChannels):
	Shader(BsdfCaps::allGlossy),
	numberOfSamples_(1)
{
	init(std::move(nodes), std::move(cdf), std::move(offsets), std::move(lengths), std::move(coefficients), numChannels);
}



const std::vector<Jakob::TValue>& Jakob::nodes() const
{
	return nodes_;
}



size_t Jakob::numChannels() const
{
	return numChannels_;
}



std::vector<Jakob::TValue> Jakob::coefficients(size_t indexIn, size_t indexOut, size_t channel) const
{
	const size_t index = indexIn * numNodes_ + indexOut;
	const size_t length = lengths_[index];
	const size_t offset = offsets_[index] + channel * length;
	std::vector<TValue> coeffs;
	coeffs.reserve(length);
	for (size_t m = 0; m < length; ++m)
	{
		coeffs.push_back(coefficients_[offset + m]);
	}
	return coeffs;
}




size_t Jakob::numberOfSamples() const
{
	return numberOfSamples_;
}



void Jakob::setNumberOfSamples(size_t number)
{
	numberOfSamples_ = number;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

TBsdfPtr Jakob::doBsdf(const Sample& sample, const IntersectionContext& context) const
{
	return TBsdfPtr(new Bsdf(this, sample, context, caps()));
}



const TPyObjectPtr Jakob::doGetState() const
{
	return python::makeTuple(nodes_, cdf_, offsets_, lengths_, coefficients_, numChannels_);
}



size_t Jakob::doNumReflectionSamples() const
{
	return numberOfSamples_;
}



size_t Jakob::doNumTransmissionSamples() const
{
	return numberOfSamples_;
}



void Jakob::doSetState(const TPyObjectPtr& state)
{
	std::vector<TValue> nodes;
	std::vector<TValue> cdf;
	std::vector<size_t> offsets;
	std::vector<size_t> lengths;
	std::vector<TValue> coefficients;
	size_t numChannels;
	python::decodeTuple(state, nodes, cdf, offsets, lengths, coefficients, numChannels);
	init(std::move(nodes), std::move(cdf), std::move(offsets), std::move(lengths), std::move(coefficients), numChannels);
}



namespace
{

	using TValue = Jakob::TValue;

	int findNode(std::function<TValue(int)> func, int size, TValue x)
	{
		if (x < func(0) || x >= func(size - 1))
		{
			return -1;
		}
		int first = 0;
		int last = size - 1;
		while (first + 1 < last)
		{
			int mid = first + (last - first) / 2;
			LASS_ASSERT(mid > first && mid < last);
			if (func(mid) <= x)
			{
				first = mid;
			}
			else
			{
				last = mid;
			}
		}
		return first;
	}

}



void Jakob::init(std::vector<TValue> nodes, std::vector<TValue> cdf, std::vector<size_t> offsets, std::vector<size_t> lengths, std::vector<TValue> coefficients, size_t numChannels)
{
	nodes_ = std::move(nodes);
	cdf_ = std::move(cdf);
	offsets_ = std::move(offsets);
	lengths_ = std::move(lengths);
	coefficients_ = std::move(coefficients);
	numChannels_ = numChannels;

	if (numChannels_ != 1 && numChannels_ != 3)
	{
		throw std::runtime_error("#channels must be 1 or 3");
	}

	numNodes_ = static_cast<int>(nodes_.size());
	if (numNodes_ < 2)
	{
		throw std::runtime_error("#nodes must be at least 2");
	}
	for (int i = 0; i < numNodes_ - 1; ++i)
	{
		if (nodes_[i] > nodes_[i + 1])
		{
			throw std::runtime_error("Nodes must be increasing");
		}
	}
	nodeZero_ = findNode([this](int i) { return nodes_[i]; }, numNodes_, TValue(0));

	if (offsets_.size() != static_cast<size_t>(numNodes_ * numNodes_))
	{
		throw std::runtime_error("#offsets must be square of #nodes");
	}
	if (lengths_.size() != static_cast<size_t>(numNodes_ * numNodes_))
	{
		throw std::runtime_error("#lengths must be square of #nodes");
	}
	maxLength_ = 0;
	for (size_t i = 0, n = offsets_.size(); i < n; ++i)
	{
		const size_t offset = offsets_[i];
		const size_t length = lengths_[i];
		maxLength_ = std::max(maxLength_, length);
		const size_t end = offset + numChannels_ * length;
		if (end > coefficients_.size())
		{
			throw std::runtime_error("Offsets and lengths exceed #coefficients");
		}
		if (i < (n - 1) && offsets_[i + 1] < end)
		{
			throw std::runtime_error("Offsets and lengths must not overlap");
		}
	}

	// figure out if we have reflection and/or transmission
	// we do this by checking if the cdf at mu=0 is the maximum or minimum
	//
	// Remember: top side of interface is muIn > 0 but muOut < 0 !
	//
	BsdfCaps caps = BsdfCaps::glossy;
	// indexIn < nodeZero_ => muIn < 0: bottom side of interface
	for (int indexIn = 0; indexIn < nodeZero_; ++indexIn)
	{
		const auto cdfZero = cdf_[indexIn * numNodes_ + nodeZero_];
		// indexOut = 0 => muOut < 0: top side of interface => transmission
		if (cdf_[indexIn * numNodes_ + 0] < cdfZero)
		{
			caps |= BsdfCaps::transmission;
		}
		// indexOut = numNodes_ - 1 => muOut > 0: bottom side of interface => reflection
		if (cdf_[indexIn * numNodes_ + (numNodes_ - 1)] > cdfZero)
		{
			caps |= BsdfCaps::reflection;
		}
	}
	// indexIn > nodeZero_ => muIn > 0: top side of interface
	for (int indexIn = nodeZero_; indexIn < numNodes_; ++indexIn)
	{
		const auto cdfZero = cdf_[indexIn * numNodes_ + nodeZero_];
		// indexOut = 0 => muOut < 0: top side of interface => reflection
		if (cdf_[indexIn * numNodes_ + 0] < cdfZero)
		{
			caps |= BsdfCaps::reflection;
		}
		// indexOut = numNodes_ - 1 => muOut > 0: bottom side of interface => transmission
		if (cdf_[indexIn * numNodes_ + (numNodes_ - 1)] > cdfZero)
		{
			caps |= BsdfCaps::transmission;
		}
	}
	setCaps(caps);
}



void Jakob::load(lass::io::BinaryIStream& stream)
{
	JakobHeader header;
	stream.read(&header, sizeof(header));
	if (!stream.good())
	{
		throw std::runtime_error("Corrupted file.");
	}
	if (memcmp(header.identifier, JAKOB_IDENTIFIER, JAKOB_IDENTIFIER_LENGTH) != 0)
	{
		throw std::runtime_error("Not a Jakob BSDF file.");
	}
	if (header.version != JAKOB_VERSION)
	{
		throw std::runtime_error("Unsupported file version.");
	}
	if (header.flags != static_cast<num::Tuint32>(JakobFlags::BSDF))
	{
		throw std::runtime_error("Unsupported BSDF flags.");
	}
	if (!(header.numChannels == 1 || header.numChannels == 3))
	{
		throw std::runtime_error("Only 1 and 3 channels are supported.");
	}
	if (header.numBases != 1)
	{
		throw std::runtime_error("Only 1 base is supported.");
	}
	if (header.numParameters != 0 || header.numParameterValues != 0)
	{
		throw std::runtime_error("Parameters are not supported.");
	}

	std::vector<TValue> nodes(header.numNodes);
	stream.read(nodes.data(), nodes.size() * sizeof(TValue));
	if (!stream.good())
	{
		throw std::runtime_error("Failed to read nodes");
	}

	std::vector<TValue> cdf(header.numNodes * header.numNodes);
	stream.read(cdf.data(), cdf.size() * sizeof(TValue));
	if (!stream.good())
	{
		throw std::runtime_error("Failed to read cdf");
	}

	std::vector<num::Tuint32> offsetAndLengths(2 * header.numNodes * header.numNodes);
	stream.read(offsetAndLengths.data(), offsetAndLengths.size() * sizeof(num::Tuint32));
	if (!stream.good())
	{
		throw std::runtime_error("Failed to read offsets and lengths");
	}
	std::vector<size_t> offsets(header.numNodes * header.numNodes);
	std::vector<size_t> lengths(header.numNodes * header.numNodes);
	for (size_t i = 0, n = header.numNodes * header.numNodes; i < n; ++i)
	{
		offsets[i] = offsetAndLengths[2 * i];
		lengths[i] = offsetAndLengths[2 * i + 1];
	}

	std::vector<TValue> coefficients(header.numCoefficients);
	stream.read(coefficients.data(), coefficients.size() * sizeof(TValue));
	if (!stream.good())
	{
		throw std::runtime_error("Failed to read coefficients");
	}

	init(std::move(nodes), std::move(cdf), std::move(offsets), std::move(lengths), std::move(coefficients), header.numChannels);
}



int Jakob::findWeights(TValue mu, TValue weights[4]) const
{
	// we do a non-uniform Catmull Rom spline interpolation between the nodes p1 and p2
	// with tangents m1 and m2:
	//
	//   p(t) = (2t^3 - 3t^2 + 1) * p1 + (t^3 - 2t^2 + t) * m1 + (-2t^3 + 3t^2) * p2 + (t^3 - t^2) * m2
	//        = a                 * p1 + b                * m1 + c              * p2 + d           * m2
	//
	// with:
	//
	//   t = (mu - mu1) / (mu2 - mu1)
	//
	// In the uniform case:
	//
	//   m1 = (p2 - p0) / 2
	//   m2 = (p3 - p1) / 2
	//
	// In the non-uniform case:
	//
	//   m1 = (p2 - p0) * (mu2 - mu1) / (mu2 - mu0)
	//   m2 = (p3 - p1) * (mu2 - mu1) / (mu3 - mu1)
	//
	// At start or end of spline, p0 or p3 does not exist, so we use:
	//
	//   m1 = m2 = (p2 - p1)
	//
	// We can finally solve for the weights w0, w1, w2, w3 such that:
	//
	//   p(t) = w0 * p0 + w1 * p1 + w2 * p2 + w3 * p3

	const int index = findNode([this](int i) { return nodes_[i]; }, numNodes_, mu);
	if (index < 0)
	{
		return -1;
	}

	const TValue mu1 = nodes_[index];
	const TValue mu2 = nodes_[index + 1];
	LIAR_ASSERT(mu1 <= mu && mu < mu2, "mu=" << mu << ", mu1=" << mu1 << ", mu2=" << mu2);

	const TValue t = (mu - mu1) / (mu2 - mu1);
	const TValue t2 = t * t;
	const TValue t3 = t2 * t;

	const TValue a = 2 * t3 - 3 * t2 + 1;
	const TValue b = t3 - 2 * t2 + t;
	const TValue c = -2 * t3 + 3 * t2;
	const TValue d = t3 - t2;

	weights[1] = a;
	weights[2] = c;

	if (index > 0)
	{
		const TValue mu0 = nodes_[index - 1];
		const TValue bb = b * (mu2 - mu1) / (mu2 - mu0);
		weights[0] = -bb;
		weights[2] += bb;
	}
	else
	{
		weights[0] = 0;
		weights[1] -= b;
		weights[2] += b;
	}

	if (index + 2 < static_cast<int>(numNodes_))
	{
		const TValue mu3 = nodes_[index + 2];
		const TValue dd = d * (mu2 - mu1) / (mu3 - mu1);
		weights[1] -= dd;
		weights[3] = dd;
	}
	else
	{
		weights[1] -= d;
		weights[2] += d;
		weights[3] = 0;
	}

	return index;
}



size_t Jakob::getCoefficients(int indexIn, const TValue weightsIn[4], int indexOut, const TValue weightsOut[4], std::vector<TValue>& coefficients) const
{
	coefficients.assign(numChannels_ * maxLength_, 0);
	size_t maxLength = 0;
	for (int i = 0; i < 4; ++i)
	{
		const int ii = indexIn + i - 1;
		if (ii < 0 || ii >= numNodes_)
		{
			continue;
		}
		for (int j = 0; j < 4; ++j)
		{
			const int jj = indexOut + j - 1;
			if (jj < 0 || jj >= numNodes_)
			{
				continue;
			}

			const TValue weight = weightsIn[i] * weightsOut[j];
			const size_t index = ii * numNodes_ + jj;
			const size_t offset = offsets_[index];
			const size_t length = lengths_[index];
			maxLength = std::max(maxLength, length);

			for (size_t k = 0; k < numChannels_; ++k)
			{
				for (size_t m = 0; m < length; ++m)
				{
					coefficients[k * maxLength_ + m] += weight * coefficients_[offset + k * length + m];
				}
			}
		}
	}
	return maxLength;
}



TValue Jakob::cdfMuOut(int indexIn, const TValue weightsIn[4], int indexOut) const
{
	// catmull-rom spline interpolation of the cdf
	TValue value = 0;
	for (int i = 0; i < 4; ++i)
	{
		const int ii = indexIn + i - 1;
		if (ii < 0 || ii >= numNodes_)
		{
			continue;
		}
		const size_t index = ii * numNodes_ + indexOut;
		value += weightsIn[i] * cdf_[index];
	}
	return value;
}

#define CHEAPOUT 0

TValue Jakob::pdfMuOut(int indexIn, const TValue weightsIn[4], int indexOut, const TValue weightsOut[4], BsdfCaps allowedCaps) const
{
	const bool allowReflection = kernel::hasCaps(allowedCaps, BsdfCaps::reflection);
	const bool allowTransmission = kernel::hasCaps(allowedCaps, BsdfCaps::transmission);
	LASS_ASSERT(allowReflection || allowTransmission);

	if (indexOut < 0 || indexOut >= numNodes_ - 1)
	{
		return 0;
	}
	auto cdf = [this, indexIn, weightsIn](int indexOut) { return this->cdfMuOut(indexIn, weightsIn, indexOut); };
	const TValue cdfMax = cdf(numNodes_ - 1);
	TValue cdfNorm;
	if (allowReflection && allowTransmission)
	{
		cdfNorm = cdfMax;
	}
	else
	{
		const TValue cdfZero = cdf(nodeZero_);
		if (allowReflection)
		{
			cdfNorm = cdfZero; // mu < 0
		}
		else
		{
			cdfNorm = cdfMax - cdfZero; // mu > 0
		}
	}
	if (cdfNorm <= 0)
	{
		return 0;
	}

#if CHEAPOUT
	// we're going to cheap out, and asume a linear cdf between the nodes
	LASS_UNUSED(weightsOut);
	const TValue dMuOut = nodes_[indexOut + 1] - nodes_[indexOut];
	LASS_ASSERT(dMuOut > 0);
	const TValue dCdf = cdf(indexOut + 1) - cdf(indexOut);
	//LASS_ASSERT(dCdf >= 0);
	return dCdf / (dMuOut * cdfNorm);
#else
	TValue value = 0;
	for (int i = 0; i < 4; ++i)
	{
		const int ii = indexIn + i - 1;
		if (ii < 0 || ii >= numNodes_)
		{
			continue;
		}
		for (int j = 0; j < 4; ++j)
		{
			const int jj = indexOut + j - 1;
			if (jj < 0 || jj >= numNodes_)
			{
				continue;
			}
			const TValue weight = weightsIn[i] * weightsOut[j];
			const size_t index = ii * numNodes_ + jj;
			const size_t offset = offsets_[index];
			const size_t length = lengths_[index];
			if (length > 0)
			{
				value += weight * coefficients_[offset];
			}
		}
	}
	return value / cdfNorm;
#endif
}



TValue Jakob::sampleMuOut(int indexIn, const TValue weightsIn[4], TValue sample, BsdfCaps allowedCaps, TValue& pdfMuOut) const
{
	const bool allowReflection = kernel::hasCaps(allowedCaps, BsdfCaps::reflection);
	const bool allowTransmission = kernel::hasCaps(allowedCaps, BsdfCaps::transmission);
	LASS_ASSERT(allowReflection || allowTransmission);

	// scale sample by maximum CDF value, and find interval so that cdf(indexOut) <= sample < cdf(indexOut + 1)
	auto cdf = [this, indexIn, weightsIn](int indexOut) { return cdfMuOut(indexIn, weightsIn, indexOut); };
	const TValue cdfMax = cdf(numNodes_ - 1);
	TValue cdfNorm;
	if (allowReflection && allowTransmission)
	{
		cdfNorm = cdfMax;
		sample *= cdfNorm;
	}
	else
	{
		const TValue cdfZero = cdf(nodeZero_);
		if (allowReflection)
		{
			// must only sample mu < 0
			cdfNorm = cdfZero;
			sample *= cdfNorm;
		}
		else
		{
			// must only sample mu > 0
			cdfNorm = cdfMax - cdfZero;
			sample = cdfZero + sample * cdfNorm;
		}
	}
	if (cdfNorm <= 0)
	{
		pdfMuOut = 0;
		return 0;
	}

	const int indexOut = findNode(cdf, numNodes_, sample);
	LIAR_ASSERT(indexOut >= 0, "indexOut=" << indexOut << ", sample=" << sample << ", cdfMax=" << cdfMax);
	if (indexOut < 0)
	{
		pdfMuOut = 0;
		return 0.5f;
	}
	LASS_ASSERT(indexOut < numNodes_ - 1);

	const TValue muOut0 = nodes_[indexOut];
	const TValue muOut1 = nodes_[indexOut + 1];
	LASS_ASSERT(muOut0 < muOut1);
	const TValue dMuOut = muOut1 - muOut0;

#if CHEAPOUT
	// we're going to cheap out, and asume a linear cdf between the nodes
	const TValue cdf0 = cdf(indexOut);
	const TValue cdf1 = cdf(indexOut + 1);
	LASS_ASSERT(cdf0 <= sample && sample < cdf1);
	const TValue dCdf = cdf1 - cdf0;
	LASS_ASSERT(dCdf > 0);

	pdfMuOut = dCdf / (dMuOut * cdfNorm);
	return  muOut0 + dMuOut * (sample - cdf0) / dCdf;
#else
	auto a0 = [this, indexIn, weightsIn](int indexOut)
	{
		// catmull-rom spline interpolation of the a0
		TValue value = 0;
		for (int i = 0; i < 4; ++i)
		{
			const int ii = indexIn + i - 1;
			if (ii < 0 || ii >= numNodes_)
			{
				continue;
			}
			const size_t index = ii * numNodes_ + indexOut;
			const size_t offset = offsets_[index];
			const size_t length = lengths_[index];
			if (length > 0)
			{
				value += weightsIn[i] * coefficients_[offset];
			}
		}
		return value;
	};

	// we're inverting a the integral of a non-uniform Catmull Rom spline interpolation between the nodes f0 and f1
	// with tangents d0 and d1:
	//
	//   fhat(t) = (2*t^3 - 3*t^2 + 1) * f0  +  (t^3 - 2*t^2 + t) * d0  +  (-2*t^3 + 3*t^2) * f1  +  (t^3 - t^2) * d1
	//           = (2*f0 - 2*f1 + d0 + d1) * t^3  +  (-3*f0 + 3*f1 - 2*d0 - d1) * t^2  +  d0 * t  +  f0
	//           = A * t^3  +  B * t^2  +  C * t  +  D
	//           = ((A * t + B) * t + C) * t + D
	//
	//   Fhat(t) = A/4 * t^4  +  B/3 * t^3  +  C/2 * t^2  +  D * t
	//
	// with:
	//
	//   t = (mu - mu0) / (mu1 - mu0)
	//
	// In the uniform case:
	//
	//   d0 = (f1 - f[-1]) / 2
	//   d1 = (f2 - f0) / 2
	//
	// In the non-uniform case:
	//
	//   d0 = (f1 - f[-1]) * (mu1 - mu0) / (mu1 - mu[-1])
	//   d1 = (f2 - f0) * (mu1 - mu0) / (mu2 - mu0)
	//
	// At start or end of spline, f[-1] or f2 does not exist, so we use:
	//
	//   d0 = d1 = (f1 - f0)
	//
	const TValue f0 = a0(indexOut);
	const TValue f1 = a0(indexOut + 1);
	const TValue df = f1 - f0;
	const TValue d0 = indexOut > 0
		? (f1 - a0(indexOut - 1)) * dMuOut / (muOut1 - nodes_[indexOut - 1])
		: df;
	const TValue d1 = indexOut < numNodes_ - 2
		? (a0(indexOut + 2) - f0) * dMuOut / (nodes_[indexOut + 2] - muOut0)
		: df;
	const TValue A = 2 * (f0 - f1) + d0 + d1;
	const TValue B = 3 * (f1 - f0) - 2 * d0 - d1;
	const TValue C = d0;
	const TValue D = f0;
	const TValue A_4 = (f0 - f1) / 2 + (d0 + d1) / 4;
	const TValue B_3 = f1 - f0 + (-2 * d0 - d1) / 3;
	const TValue C_2 = d0 / 2;
	const TValue D_1 = f0;

	// rescale sample so it maps [0, 1] over [Fhat(0), Fhat(1)]
	sample = (sample - cdf(indexOut)) / dMuOut;

	// Take a first guess at t, assuming fhat is linear, so that Fhat is quadratic:
	//
	//   fhat(t) = f0 + t * (f1 - f0)
	//   Fhat(t) = f0 * t + t^2 * (f1 - f0) / 2 = sample
	//
	// and
	//
	//    df / 2 * t^2 + f0 * t - sample = 0
	//
	// solving for t:
	//
	//    D = f0^2 - 4 * (f1 - f0) / 2 * sample
	//    t = (-f0 +/- sqrt(D)) / (f1 - f0)
	//
	// Two possible solutions, but only +sqrt(D) is valid, since -sqrt(D) would give a negative t.
	//
	TValue t = f0 != f1
		? (num::sqrt(std::max(num::sqr(f0) + 2 * df * sample, 0.f)) - f0) / df
		: sample / f0;

	TValue begin = 0;
	TValue end = 1;
	constexpr TValue tol = 1e-6f;
	while (true)
	{
		if (t < begin || t > end)
		{
			t = (begin + end) / 2;
		}
		const TValue fhat = ((A * t + B) * t + C) * t + D;
		TValue Fhat = (((A_4 * t + B_3) * t + C_2) * t + D_1) * t;
		Fhat -= sample;
		if (num::abs(Fhat) < tol || (end - begin) < tol)
		{
			pdfMuOut = fhat / cdfNorm;
			return muOut0 + t * dMuOut;
		}
		if (Fhat < 0)
		{
			begin = t;
		}
		else
		{
			end = t;
		}
		t -= Fhat / fhat;
	}
#endif
}



// --- bsdf ----------------------------------------------------------------------------------------

namespace
{

using TValue = Jakob::TValue;


double cosPhi(const TVector3D& omegaA, const TVector3D& omegaB)
{
	const prim::Vector2D<double> a(omegaA.x, omegaA.y);
	const prim::Vector2D<double> b(omegaB.x, omegaB.y);
	const double squaredNorm = a.squaredNorm() * b.squaredNorm();
	return squaredNorm > 0
		? num::clamp(dot(a, b) / num::sqrt(squaredNorm), -1., 1.)
		: 0.;
}


TValue fourier(const TValue* coefficients, size_t length, double cosPhi)
{
	LASS_ASSERT(length > 0);
	double cosKmin2Phi = cosPhi;
	double cosKmin1Phi = 1.;
	double value = coefficients[0];
	for (size_t i = 1; i < length; ++i)
	{
		const double cosKPhi = 2 * cosPhi * cosKmin1Phi - cosKmin2Phi;
		value += coefficients[i] * cosKPhi;
		cosKmin2Phi = cosKmin1Phi;
		cosKmin1Phi = cosKPhi;
	}
	return static_cast<TValue>(value);
}


double sampleFourier(const TValue* coefficients, size_t length, double sample, TValue& value, TScalar& pdfPhi)
{
	using TDoubleTraits = num::NumTraits<double>;
	constexpr double tol = 1e-6;

	LASS_ASSERT(length > 0);

	const bool flip = sample >= 0.5;
	sample = flip
		? 1 - 2 * (sample - 0.5)
		: 2 * sample;

	double begin = 0;
	double end = TDoubleTraits::pi;
	double phi = TDoubleTraits::pi / 2;
	const double maxF = coefficients[0] * 2 * TDoubleTraits::pi;
	sample *= maxF / 2;
	while (true)
	{
		const double cosPhi = num::cos(phi);
		const double sinPhi = num::sqrt(std::max(1 - num::sqr(cosPhi), 0.));
		double cosKmin2Phi = cosPhi;
		double cosKmin1Phi = 1;
		double sinKmin2Phi = -sinPhi;
		double sinKmin1Phi = 0;
		double f = coefficients[0];
		double F = coefficients[0] * phi;
		for (size_t k = 1; k < length; ++k)
		{
			const double cosKPhi = 2 * cosPhi * cosKmin1Phi - cosKmin2Phi;
			const double sinKPhi = 2 * cosPhi * sinKmin1Phi - sinKmin2Phi;
			f += coefficients[k] * cosKPhi;
			F += coefficients[k] * sinKPhi / static_cast<double>(k);
			cosKmin2Phi = cosKmin1Phi;
			cosKmin1Phi = cosKPhi;
			sinKmin2Phi = sinKmin1Phi;
			sinKmin1Phi = sinKPhi;
		}
		F -= sample;
		if (F <= 0)
		{
			begin = phi;
		}
		else
		{
			end = phi;
		}
		if (num::abs(F) < tol || (end - begin) < tol)
		{
			if (flip)
				phi = 2 * TDoubleTraits::pi - phi;
			value = static_cast<TValue>(f);
			pdfPhi = static_cast<TScalar>(f / maxF);
			return phi;
		}
		phi -= F / f;
		if (phi <= begin || phi >= end)
		{
			phi = (begin + end) / 2;
		}
	}
}

}



Jakob::Bsdf::Bsdf(const Jakob* parent, const Sample& sample, const IntersectionContext& context, BsdfCaps caps):
	kernel::Bsdf(sample, context, caps),
	coefficients_(parent->numChannels_ * parent->maxLength_),
	parent_(parent)
{
}

BsdfOut Jakob::Bsdf::doEvaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, BsdfCaps allowedCaps) const
{
	LASS_ASSERT(shaders::compatibleCaps(allowedCaps, caps()));

	if (omegaOut.z == 0)
	{
		return BsdfOut();
	}

	LASS_ASSERT(kernel::hasCaps(allowedCaps, omegaIn.z * omegaOut.z > 0 ? BsdfCaps::reflection : BsdfCaps::transmission));

	const TValue muIn = static_cast<TValue>(omegaIn.z);
	TValue weightsIn[4];
	const int indexIn = parent_->findWeights(muIn, weightsIn);
	const TValue muOut = -static_cast<TValue>(omegaOut.z);
	TValue weightsOut[4];
	const int indexOut = parent_->findWeights(muOut, weightsOut);
	if (indexIn < 0 || indexOut < 0)
	{
		return BsdfOut();
	}
	const TValue pdfMuOut = parent_->pdfMuOut(indexIn, weightsIn, indexOut, weightsOut, allowedCaps);

	const size_t length = parent_->getCoefficients(indexIn, weightsIn, indexOut, weightsOut, coefficients_);
	if (length == 0 || coefficients_[0] <= 0)
	{
		return BsdfOut();
	}
	const double cosPhi = static_cast<TValue>(shaders::cosPhi(omegaIn, -omegaOut));
	const TValue y = std::max(fourier(coefficients_.data(), length, cosPhi), 0.0f);
	const TValue pdfPhi = y / (coefficients_[0] * 2 * num::NumTraits<TValue>::pi);

	const Spectral value = spectral(y, length, cosPhi, muOut);
	const TScalar pdf = std::max(pdfMuOut * pdfPhi, 0.f);
	LIAR_ASSERT_POSITIVE_FINITE(pdf);
	return BsdfOut(value, pdf);
}



SampleBsdfOut Jakob::Bsdf::doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar, BsdfCaps allowedCaps) const
{
	LASS_ASSERT(shaders::compatibleCaps(allowedCaps, caps()));

	const TValue muIn = static_cast<TValue>(omegaIn.z);
	TValue weightsIn[4];
	const int indexIn = parent_->findWeights(muIn, weightsIn);
	if (indexIn < 0)
	{
		return SampleBsdfOut();
	}

	TValue pdfMuOut;
	const TValue muOut = parent_->sampleMuOut(indexIn, weightsIn, static_cast<TValue>(sample.x), allowedCaps, pdfMuOut);
	if (pdfMuOut <= 0)
	{
		return SampleBsdfOut();
	}
	TValue weightsOut[4];
	const int indexOut = parent_->findWeights(muOut, weightsOut);

	const size_t length = parent_->getCoefficients(indexIn, weightsIn, indexOut, weightsOut, coefficients_);
	if (length == 0)
	{
		return SampleBsdfOut();
	}

	TValue y;
	TScalar pdfPhi;
	const auto phi = sampleFourier(coefficients_.data(), length, static_cast<double>(sample.y), y, pdfPhi);
	if (pdfPhi <= 0)
	{
		return SampleBsdfOut();
	}

	const TScalar sinThetaIn = num::sqrt(std::max(1 - num::sqr(omegaIn.z), TNumTraits::zero));
	const TScalar cosPhiIn = sinThetaIn > 0 ? omegaIn.x / sinThetaIn : 1;
	const TScalar sinPhiIn = sinThetaIn > 0 ? omegaIn.y / sinThetaIn : 0;
	const auto cosPhi = num::cos(phi);
	const auto sinPhi = num::sin(phi);
	const TScalar cosPhiOut = static_cast<TScalar>(cosPhiIn * cosPhi - sinPhiIn * sinPhi);
	const TScalar sinPhiOut = static_cast<TScalar>(cosPhiIn * sinPhi + sinPhiIn * cosPhi);
	const TScalar cosThetaOut = -muOut;
	const TScalar sinThetaOut = num::sqrt(std::max(1 - num::sqr(cosThetaOut), TNumTraits::zero));
	const TVector3D omegaOut(-cosPhiOut * sinThetaOut, -sinPhiOut * sinThetaOut, cosThetaOut);

	const Spectral value = spectral(y, length, cosPhi, muOut);
	const TScalar pdf = pdfMuOut * pdfPhi;
	const BsdfCaps usedCaps = (omegaOut.z < 0 ? BsdfCaps::transmission : BsdfCaps::reflection) | BsdfCaps::glossy;
	return SampleBsdfOut(omegaOut.normal(), value, pdf, usedCaps);
}



Spectral Jakob::Bsdf::spectral(TValue y, size_t length, double cosPhi, TValue muOut) const
{
	if (parent_->numChannels_ == 1)
	{
		return Spectral(y / num::abs(muOut));
	}
	const TValue r = fourier(coefficients_.data() + parent_->maxLength_, length, cosPhi);
	const TValue b = fourier(coefficients_.data() + 2 * parent_->maxLength_, length, cosPhi);
	const TValue g = 1.39829f * y - 0.100913f * b - 0.297375f * r;
	const XYZ xyz = kernel::sRGB->toXYZlinear(prim::ColorRGBA(r, g, b, 1.f) / num::abs(muOut));
	return max(Spectral::fromXYZ(xyz, this->sample(), SpectralType::Illuminant), 0);
}


// --- free ----------------------------------------------------------------------------------------




}

}

// EOF
