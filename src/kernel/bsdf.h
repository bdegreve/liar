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

/** @class liar::Bsdf
 *  @brief BSDF kernel of a shader
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_BSDF_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_BSDF_H

#include "kernel_common.h"
#include "bsdf_caps.h"
#include "spectral.h"
#include <lass/util/allocator.h>

namespace liar
{
namespace kernel
{

class Shader;
class Sample;
class IntersectionContext;

enum class BsdfCaps
{
	none = 0x00,

	emission = 0x01,
	reflection = 0x02,
	transmission = 0x04,

	diffuse = 0x10,
	specular = 0x20,
	glossy = 0x40,

	all = 0xff,

	nonDiffuse = glossy | specular,

	allReflection = reflection | diffuse | nonDiffuse,
	allTransmission = transmission | diffuse | nonDiffuse,

	allDiffuse = reflection | transmission | diffuse,
	allSpecular = reflection | transmission | specular,
	allGlossy = reflection | transmission | glossy,
	allNonDiffuse = reflection | transmission | nonDiffuse,
};

inline constexpr BsdfCaps operator& (BsdfCaps a, BsdfCaps b) { return static_cast<BsdfCaps>(static_cast<std::underlying_type_t<BsdfCaps>>(a) & static_cast<std::underlying_type_t<BsdfCaps>>(b)); }
inline constexpr BsdfCaps operator| (BsdfCaps a, BsdfCaps b) { return static_cast<BsdfCaps>(static_cast<std::underlying_type_t<BsdfCaps>>(a) | static_cast<std::underlying_type_t<BsdfCaps>>(b)); }
inline constexpr BsdfCaps operator~ (BsdfCaps a) { return static_cast<BsdfCaps>(~static_cast<std::underlying_type_t<BsdfCaps>>(a)) & BsdfCaps::all; }
inline constexpr BsdfCaps& operator&= (BsdfCaps& a, BsdfCaps b) { a = a & b; return a; }
inline constexpr BsdfCaps& operator|= (BsdfCaps& a, BsdfCaps b) { a = a | b; return a; }

struct BsdfIn
{
	BsdfIn(): omegaOut(), allowedCaps() {}
	BsdfIn(const TVector3D& omegaOut, BsdfCaps allowedCaps): 
		omegaOut(omegaOut), allowedCaps(allowedCaps) {}
	TVector3D omegaOut;
	BsdfCaps allowedCaps;
};

struct BsdfOut
{
	Spectral value;
	TScalar pdf;
	BsdfOut(const Spectral& value = Spectral(), TScalar pdf = 0): value(value), pdf(pdf) {}
	bool operator!() const { return !(pdf > 0 && value); }
	explicit operator bool() const { return pdf > 0 && value; }
	BsdfOut& operator+=(const BsdfOut& other) 
	{
		value += other.value;
		pdf += other.pdf;
		return *this;
	}
};

struct SampleBsdfIn
{
	SampleBsdfIn(): sample(), allowedCaps() {}
	SampleBsdfIn(const TPoint2D& sample, BsdfCaps allowedCaps): sample(sample), allowedCaps(allowedCaps) {}
	TPoint2D sample;
	BsdfCaps allowedCaps;
};

struct SampleBsdfOut
{
	TVector3D omegaOut;
	Spectral value;
	TScalar pdf;
	BsdfCaps usedCaps;
	SampleBsdfOut(const TVector3D& omegaOut = TVector3D(), const Spectral& value = Spectral(), TScalar pdf = 0, BsdfCaps usedCaps = BsdfCaps::none):
		omegaOut(omegaOut), value(value), pdf(pdf), usedCaps(usedCaps) {}
	SampleBsdfOut(const TVector3D& omegaOut, const BsdfOut& bsdf, BsdfCaps usedCaps):
		omegaOut(omegaOut), value(bsdf.value), pdf(bsdf.pdf), usedCaps(usedCaps) {}
	bool operator!() const { return !(pdf > 0 && value); }
	explicit operator bool() const { return pdf > 0 && value; }
};

typedef util::AllocatorBinned< util::AllocatorConcurrentFreeList<> > TBsdfAllocator;

inline constexpr bool hasCaps(BsdfCaps capsUnderTest, BsdfCaps wantedCaps)
{
	return (capsUnderTest & wantedCaps) == wantedCaps;
}

inline constexpr bool compatibleCaps(BsdfCaps capsUnderTest, BsdfCaps allowedCaps)
{
	const std::underlying_type_t<BsdfCaps> overlapping = static_cast<std::underlying_type_t<BsdfCaps>>(capsUnderTest & allowedCaps);
	return (overlapping & 0xf) > 0 && (overlapping & 0xf0) > 0;
}



class LIAR_KERNEL_DLL Bsdf: public util::AllocatorClassAdaptor<TBsdfAllocator>
{
public:

	Bsdf(const Sample& sample, const IntersectionContext& context, BsdfCaps caps);
	virtual ~Bsdf();

	const IntersectionContext& context() const { return context_; }

	BsdfCaps caps() const { return caps_; }
	bool hasCaps(BsdfCaps wantedCaps) const { return kernel::hasCaps(caps_, wantedCaps); }
	bool compatibleCaps(BsdfCaps allowedCaps) const { return kernel::compatibleCaps(caps_, allowedCaps); }
	bool isDispersive() const;

	BsdfOut evaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, BsdfCaps allowedCaps) const;
	SampleBsdfOut sample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, BsdfCaps allowedCaps) const;

	const TVector3D bsdfToWorld(const TVector3D& v) const;
	const TVector3D worldToBsdf(const TVector3D& v) const;

private:

	virtual BsdfOut doEvaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, BsdfCaps allowedCaps) const = 0;
	virtual SampleBsdfOut doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, BsdfCaps allowedCaps) const = 0;
	virtual bool doIsDispersive() const;

	TVector3D omegaGeometricNormal_;
	const Sample& sample_;
	const IntersectionContext& context_; // no ownership!
	BsdfCaps caps_;
};

typedef std::unique_ptr<Bsdf> TBsdfPtr;

}

}

#endif

// EOF

