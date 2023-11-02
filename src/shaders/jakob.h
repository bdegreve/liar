/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2023  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::shaders::Jakob
 *  @brief Fourier-based model by Wenzel Jakob et al. (2014)
 *  @author Bram de Greve [Bramz]
 *
 *  @par reference:
 *		@arg W. Jakob, E. d'Eon, O. Jakob, S. Marschner,
 *		<i>A comprehensive framework for rendering layered materials</i>.
 *		ACM Trans. Graph. 33, 4, Article 118 (July 2014), 14 pages.
 *		https://doi.org/10.1145/2601097.2601139
 *
 *		@arg M. Pharr, W. Jakob, G. Humphreys,
 *		<i>Physically Based Rendering: From Theory to Implementation</i>.
 *		3rd edition, Morgan Kaufmann, 2016.
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SHADERS_JAKOB_H
#define LIAR_GUARDIAN_OF_INCLUSION_SHADERS_JAKOB_H

#include "shaders_common.h"
#include "../kernel/shader.h"
#include "../kernel/texture.h"
#include <lass/stde/static_vector.h>
#include <lass/io/binary_i_stream.h>
#include <filesystem>
namespace liar
{
namespace shaders
{

class LIAR_SHADERS_DLL Jakob: public Shader
{
	PY_HEADER(Shader)
public:

	using TValue = Spectral::TValue;

	Jakob(const std::filesystem::path& path);
	Jakob(std::vector<TValue> nodes, std::vector<TValue> cdf, std::vector<size_t> offsets, std::vector<size_t> lengths, std::vector<TValue> coefficients, size_t numChannels);

	const std::vector<TValue>& nodes() const;
	size_t numChannels() const;

	std::vector<TValue> coefficients(size_t indexIn, size_t indexOut, size_t channel) const;

	size_t numberOfSamples() const;
	void setNumberOfSamples(size_t number);

private:

	void init(std::vector<TValue> nodes, std::vector<TValue> cdf, std::vector<size_t> offsets, std::vector<size_t> lengths, std::vector<TValue> coefficients, size_t numChannels);
	void load(lass::io::BinaryIStream& stream);

	class Bsdf: public kernel::Bsdf
	{
	public:
		Bsdf(const Jakob* parent, const Sample& sample, const IntersectionContext& context, BsdfCaps caps);
	private:
		BsdfOut doEvaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, BsdfCaps allowedCaps) const;
		SampleBsdfOut doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, BsdfCaps allowedCaps) const;
		Spectral spectral(TValue y, size_t length, TScalar cosPhi, TValue muOut) const;
		mutable std::vector<TValue> coefficients_;
		const Jakob* parent_;
	};

	TBsdfPtr doBsdf(const Sample& sample, const IntersectionContext& context) const override;

	size_t doNumReflectionSamples() const override;
	size_t doNumTransmissionSamples() const override;

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

	int findWeights(TValue mu, TValue weight[4]) const;
	size_t getCoefficients(int indexIn, const TValue weightsIn[4], int indexOut, const TValue weightsOut[4], std::vector<TValue>& coefficients) const;
	TValue cdfMuOut(int indexIn, const TValue weightsIn[4], int indexOut) const;
	TValue pdfMuOut(int indexIn, const TValue weightsIn[4], int indexOut, const TValue weightsOut[4]) const;
	TValue sampleMuOut(int indexIn, const TValue weightsIn[4], TValue sample, TValue& pdfMuOut) const;

	std::vector<TValue> nodes_;
	std::vector<TValue> cdf_;
	std::vector<size_t> offsets_;
	std::vector<size_t> lengths_;
	std::vector<TValue> coefficients_;
	size_t numChannels_;
	size_t maxLength_;
	int numNodes_;
	size_t numberOfSamples_;
};


}

}

#endif

// EOF
