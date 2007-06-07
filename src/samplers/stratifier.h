/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
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
 *  http://liar.sourceforge.net
 */

/** @class liar::samplers::Stratifier
 *  @author Bram de Greve [Bramz]
 */

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

class LIAR_SAMPLERS_DLL Stratifier: public Sampler
{
    PY_HEADER(Sampler)
public:

    Stratifier();
    Stratifier(const TResolution2D& resolution, unsigned numberOfSamplesPerPixel);
    Stratifier(const TResolution2D& resolution);

	const bool jittered() const;
	void setJittered(bool enabled);

private:

	typedef std::vector<TVector2D> TStrata2D;
	typedef std::vector<TScalar> TStrata1D;

    typedef num::RandomMT19937 TNumberGenerator;
    typedef num::DistributionUniform<TScalar, TNumberGenerator, num::rtRightOpen> TJitterGenerator;

	void init(const TResolution2D& resolution = TResolution2D(320, 240), unsigned iNumberOfSamples = 1);

	virtual const TResolution2D& doResolution() const;
    virtual const unsigned doSamplesPerPixel() const;
    virtual void doSetResolution(const TResolution2D& resolution);
    virtual void doSetSamplesPerPixel(unsigned samplesPerPixel);
	virtual void doSeed(unsigned randomSeed);

	virtual void doSampleScreen(const TResolution2D& pixel, unsigned subPixel, 
		TSample2D& oScreenCoordinate);
	virtual void doSampleLens(const TResolution2D& pixel, unsigned subPixel, 
		TSample2D& oScreenCoordinate);
	virtual void doSampleTime(const TResolution2D& pixel, unsigned subPixel, 
		const TimePeriod& period, TTime& oTime);
	virtual void doSampleSubSequence1D(const TResolution2D& pixel, unsigned subPixel, 
		TSample1D* oBegin, TSample1D* oEnd);
	virtual void doSampleSubSequence2D(const TResolution2D& pixel, unsigned subPixel, 
		TSample2D* oBegin, TSample2D* oEnd);

	virtual const unsigned doRoundSize1D(unsigned requestedSize) const;
	virtual const unsigned doRoundSize2D(unsigned requestedSize) const;

	virtual const TSamplerPtr doClone() const;

	virtual const TPyObjectPtr doGetState() const;
	virtual void doSetState(const TPyObjectPtr& state);

	void shuffleTimeStrata();

    TNumberGenerator numberGenerator_;
    TJitterGenerator jitterGenerator_;

    TResolution2D resolution_;
    TVector2D reciprocalResolution_;
	TVector2D stratum2DSize_;
    TScalar stratum1DSize_;
    TStrata2D screenStrata_;
    TStrata2D lensStrata_;
	TStrata1D timeStrata_;
    unsigned strataPerPixel_;
	bool isJittered_;
};



}

}

#endif

// EOF
