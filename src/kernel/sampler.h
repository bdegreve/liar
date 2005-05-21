/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2005  Bram de Greve
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  http://liar.sourceforge.net
 */

/** @class liar::kernel::Sampler
 *  @brief generates samples to be feed to the ray tracer.
 *  @author Bram de Greve [BdG]
 *
 *  @warning
 *  THE INTERFACE OF THE SAMPLER IS STILL PRELIMINARY.  The main problem is how to avoid that two 
 *  engines use the same sampler and thus screwing each other.  Or if they do use the same sampler,
 *  how can we manage they don't screw each other. 
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SAMPLER_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SAMPLER_H

#include "kernel_common.h"
#include <lass/prim/vector_2d.h>

namespace liar
{
namespace kernel
{

class Sample;
class Sampler;

typedef python::PyObjectPtr<Sampler>::Type TSamplerPtr;

class LIAR_KERNEL_DLL Sampler: public python::PyObjectPlus
{
    PY_HEADER(python::PyObjectPlus)
public:

    typedef prim::Vector2D<unsigned> TResolution;

    const TResolution& resolution() const { return doResolution(); }
    const unsigned samplesPerPixel() const { return doSamplesPerPixel(); }
    void setResolution(const TResolution& iResolution) { doSetResolution(iResolution); }
    void setSamplesPerPixel(unsigned iSamplesPerPixel) { doSetSamplesPerPixel(iSamplesPerPixel); }

    const int requestSubSequence1D(unsigned iRequestedSize);
    const int requestSubSequence2D(unsigned iRequestedSize);
	void clearSubSequenceRequests();

    void sample(const TResolution& iPixel, unsigned iSubPixel, Sample& oSample);

    static TSamplerPtr& defaultSampler();

protected:

    Sampler(PyTypeObject* iType);

private:

	friend class Sample;

	typedef std::vector<unsigned> TSubSequenceSizes;

    virtual const TResolution& doResolution() const = 0;
    virtual const unsigned doSamplesPerPixel() const = 0;
    virtual void doSetResolution(const TResolution& iResolution) = 0;
    virtual void doSetSamplesPerPixel(unsigned iSamplesPerPixel) = 0;
    
	virtual void doSampleScreenCoordinate(const TResolution& iPixel, unsigned iSubPixel, 
		TPoint2D& oScreenCoordinate) = 0;
	virtual void doSampleSubSequence1D(const TResolution& iPixel, unsigned iSubPixel, 
		TScalar* oBegin, TScalar* oEnd) = 0;
	virtual void doSampleSubSequence2D(const TResolution& iPixel, unsigned iSubPixel, 
		TVector2D* oBegin, TVector2D* oEnd) = 0;

	virtual const unsigned doRoundSize1D(unsigned iRequestedSize) const;
	virtual const unsigned doRoundSize2D(unsigned iRequestedSize) const;

	const unsigned subSequenceSize1D(int iId) const { return subSequenceSize1D_[iId]; }
	const unsigned subSequenceSize2D(int iId) const { return subSequenceSize2D_[iId]; }
	const unsigned subSequenceOffset1D(int iId) const { return subSequenceOffset1D_[iId]; }
	const unsigned subSequenceOffset2D(int iId) const { return subSequenceOffset2D_[iId]; }

    static TSamplerPtr defaultSampler_;

	TSubSequenceSizes subSequenceSize1D_;
	TSubSequenceSizes subSequenceSize2D_;
	TSubSequenceSizes subSequenceOffset1D_;
	TSubSequenceSizes subSequenceOffset2D_;
	unsigned totalSubSequenceSize1D_;
	unsigned totalSubSequenceSize2D_;
};

typedef python::PyObjectPtr<Sampler>::Type TSamplerPtr;

}

}

#endif

// EOF

