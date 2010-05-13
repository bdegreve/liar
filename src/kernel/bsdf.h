/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
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
#include "xyz.h"
#include <lass/util/allocator.h>

namespace liar
{
namespace kernel
{

class Shader;
class Sample;
class IntersectionContext;

struct BsdfIn
{
	BsdfIn(): omegaOut(), allowedCaps() {}
	BsdfIn(const TVector3D& omegaOut, unsigned allowedCaps): 
		omegaOut(omegaOut), allowedCaps(allowedCaps) {}
	TVector3D omegaOut;
	unsigned allowedCaps;
};

struct BsdfOut
{
	XYZ value;
	TScalar pdf;
	BsdfOut(const XYZ& value = XYZ(), TScalar pdf = 0): value(value), pdf(pdf) {}
	bool operator!() const { return !(pdf > 0 && value); }
	operator num::SafeBool() const { return !*this ? num::safeFalse : num::safeTrue; }
};

struct SampleBsdfIn
{
	SampleBsdfIn(): sample(), allowedCaps() {}
	SampleBsdfIn(const TPoint2D& sample, unsigned allowedCaps): sample(sample), allowedCaps(allowedCaps) {}
	TPoint2D sample;
	unsigned allowedCaps;
};

struct SampleBsdfOut
{
	TVector3D omegaOut;
	XYZ value;
	TScalar pdf;
	unsigned usedCaps;
	SampleBsdfOut(const TVector3D& omegaOut = TVector3D(), const XYZ& value = XYZ(), TScalar pdf = 0, unsigned usedCaps = 0):
		omegaOut(omegaOut), value(value), pdf(pdf), usedCaps(usedCaps) {}
	bool operator!() const { return !(pdf > 0 && value); }
	operator num::SafeBool() const { return !*this ? num::safeFalse : num::safeTrue; }
};

typedef util::AllocatorBinned< util::AllocatorConcurrentFreeList<> > TBsdfAllocator;

class LIAR_KERNEL_DLL Bsdf: public util::AllocatorClassAdaptor<TBsdfAllocator>
{
public:

	Bsdf(const Sample& sample, const IntersectionContext& context);
	virtual ~Bsdf();

	BsdfOut call(const TVector3D& omegaIn, const TVector3D& omegaOut, unsigned allowedCaps) const
	{
		//LASS_ASSERT(omegaIn.z >= 0);
		return doCall(omegaIn, omegaOut, allowedCaps);
	}

	SampleBsdfOut sample(const TVector3D& omegaIn, const TPoint2D& sample, unsigned allowedCaps) const
	{
		//LASS_ASSERT(omegaIn.z >= 0);
		return doSample(omegaIn, sample, allowedCaps);
	}

	const TVector3D bsdfToWorld(const TVector3D& v) const;
	const TVector3D worldToBsdf(const TVector3D& v) const;

private:

	virtual BsdfOut doCall(const TVector3D& omegaIn, const TVector3D& omegaOut, unsigned allowedCaps) const;
	virtual SampleBsdfOut doSample(const TVector3D& omegaIn, const TPoint2D& sample, unsigned allowedCaps) const;

	const Sample& sample_;
	const IntersectionContext& context_;

public:
	size_t refCount_;
};

typedef util::SharedPtr<Bsdf, util::ObjectStorage, util::IntrusiveCounter<Bsdf, size_t, &Bsdf::refCount_> > TBsdfPtr;

}

}

#endif

// EOF

