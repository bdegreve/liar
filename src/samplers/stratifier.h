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

/** @class liar::samplers::Stratifier
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_SAMPLERS_STRATIFIER_H
#define LIAR_GUARDIAN_OF_INCLUSION_SAMPLERS_STRATIFIER_H

#include "samplers_common.h"
#include "../kernel/sampler.h"
#include <lass/num/random.h>
#include <lass/num/distribution.h>

namespace liar
{
namespace samplers
{

class LIAR_SAMPLERS_DLL Stratifier: public kernel::Sampler
{
    PY_HEADER(kernel::Sampler)
public:

    Stratifier();

private:

    typedef num::RandomMT19937 TNumberGenerator;
    typedef num::DistributionUniform<TScalar, TNumberGenerator, num::rtRightOpen> TJitterGenerator;

    virtual const TResolution& doResolution() const;
    virtual const unsigned doSamplesPerPixel() const;
    virtual void doSetResolution(const TResolution& iResolution);
    virtual void doSetSamplesPerPixel(unsigned iSamplesPerPixel);
	virtual void doSeed(unsigned iRandomSeed);

	virtual void doSampleScreenCoordinate(const TResolution& iPixel, unsigned iSubPixel, 
		TPoint2D& oScreenCoordinate);
	virtual void doSampleTime(const TResolution& iPixel, unsigned iSubPixel, 
		const kernel::TimePeriod& iPeriod, TTime& oTime);
	virtual void doSampleSubSequence1D(const TResolution& iPixel, unsigned iSubPixel, 
		TScalar* oBegin, TScalar* oEnd);
	virtual void doSampleSubSequence2D(const TResolution& iPixel, unsigned iSubPixel, 
		TVector2D* oBegin, TVector2D* oEnd);

	virtual const unsigned doRoundSize1D(unsigned iRequestedSize) const;
	virtual const unsigned doRoundSize2D(unsigned iRequestedSize) const;

	void shuffleTimeStrata();

    TNumberGenerator numberGenerator_;
    TJitterGenerator jitterGenerator_;

    TResolution resolution_;
    TVector2D reciprocalResolution_;
	TVector2D screenCoordinateStratumSize_;
    TScalar timeStratumSize_;
    std::vector<TVector2D> screenCoordinateStrata_;
	std::vector<TScalar> timeStrata_;
    unsigned strataPerPixel_;
	std::vector<size_t> randomStrataSequence_;
};



}

}

#endif

// EOF
