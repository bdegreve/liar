/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2020  Bram de Greve (bramz@users.sourceforge.net)
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

typedef size_t TBsdfCaps;

struct BsdfIn
{
	BsdfIn(): omegaOut(), allowedCaps() {}
	BsdfIn(const TVector3D& omegaOut, TBsdfCaps allowedCaps): 
		omegaOut(omegaOut), allowedCaps(allowedCaps) {}
	TVector3D omegaOut;
	TBsdfCaps allowedCaps;
};

struct BsdfOut
{
	Spectral value;
	TScalar pdf;
	BsdfOut(const Spectral& value = Spectral(), TScalar pdf = 0): value(value), pdf(pdf) {}
	bool operator!() const { return !(pdf > 0 && value); }
	operator num::SafeBool() const { return !*this ? num::safeFalse : num::safeTrue; }
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
	SampleBsdfIn(const TPoint2D& sample, TBsdfCaps allowedCaps): sample(sample), allowedCaps(allowedCaps) {}
	TPoint2D sample;
	TBsdfCaps allowedCaps;
};

struct SampleBsdfOut
{
	TVector3D omegaOut;
	Spectral value;
	TScalar pdf;
	TBsdfCaps usedCaps;
	SampleBsdfOut(const TVector3D& omegaOut = TVector3D(), const Spectral& value = Spectral(), TScalar pdf = 0, TBsdfCaps usedCaps = 0):
		omegaOut(omegaOut), value(value), pdf(pdf), usedCaps(usedCaps) {}
	SampleBsdfOut(const TVector3D& omegaOut, const BsdfOut& bsdf, TBsdfCaps usedCaps):
		omegaOut(omegaOut), value(bsdf.value), pdf(bsdf.pdf), usedCaps(usedCaps) {}
	bool operator!() const { return !(pdf > 0 && value); }
	operator num::SafeBool() const { return !*this ? num::safeFalse : num::safeTrue; }
};

typedef util::AllocatorBinned< util::AllocatorConcurrentFreeList<> > TBsdfAllocator;

inline bool hasCaps(TBsdfCaps capsUnderTest, TBsdfCaps wantedCaps)
{
	return (capsUnderTest & wantedCaps) == wantedCaps;
}

inline bool compatibleCaps(TBsdfCaps capsUnderTest, TBsdfCaps allowedCaps)
{
	const TBsdfCaps overlapping = capsUnderTest & allowedCaps;
	return (overlapping & 0xf) > 0 && (overlapping & 0xf0) > 0;
}



class LIAR_KERNEL_DLL Bsdf: public util::AllocatorClassAdaptor<TBsdfAllocator>
{
public:

	enum CapsFlags
	{
		capsNone = 0x00,

		capsEmission = 0x01,
		capsReflection = 0x02,
		capsTransmission = 0x04,

		capsDiffuse = 0x10,
		capsSpecular = 0x20,
		capsGlossy = 0x40,

		capsAll = 0xff,

		capsNonDiffuse = capsGlossy | capsSpecular,

		capsAllReflection = capsReflection | capsDiffuse | capsNonDiffuse,
		capsAllTransmission = capsTransmission | capsDiffuse | capsNonDiffuse,

		capsAllDiffuse = capsReflection | capsTransmission | capsDiffuse,
		capsAllSpecular = capsReflection | capsTransmission | capsSpecular,
		capsAllGlossy = capsReflection | capsTransmission | capsGlossy,
		capsAllNonDiffuse = capsReflection | capsTransmission | capsNonDiffuse,
	};

	Bsdf(const Sample& sample, const IntersectionContext& context, TBsdfCaps caps);
	virtual ~Bsdf();

	const IntersectionContext& context() const { return context_; }

	TBsdfCaps caps() const { return caps_; }
	bool hasCaps(TBsdfCaps wantedCaps) const { return kernel::hasCaps(caps_, wantedCaps); }
	bool compatibleCaps(TBsdfCaps allowedCaps) const { return kernel::compatibleCaps(caps_, allowedCaps); }
	bool isDispersive() const;

	BsdfOut evaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, TBsdfCaps allowedCaps) const;
	SampleBsdfOut sample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, TBsdfCaps allowedCaps) const;

	const TVector3D bsdfToWorld(const TVector3D& v) const;
	const TVector3D worldToBsdf(const TVector3D& v) const;

private:

	virtual BsdfOut doEvaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, TBsdfCaps allowedCaps) const = 0;
	virtual SampleBsdfOut doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, TBsdfCaps allowedCaps) const = 0;
	virtual bool doIsDispersive() const;

	TVector3D omegaGeometricNormal_;
	const Sample& sample_;
	const IntersectionContext& context_; // no ownership!
	TBsdfCaps caps_;

public:
	size_t refCount_; // oops, public???
};

typedef util::SharedPtr<Bsdf, util::ObjectStorage, util::IntrusiveCounter<Bsdf, size_t, &Bsdf::refCount_> > TBsdfPtr;

}

}

#endif

// EOF

