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

/** @class liar::SamplerTiled
 *  @brief generates samples per pixel and groups tiles of pixels in tasks
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SAMPLER_TILED_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SAMPLER_TILED_H

#include "kernel_common.h"
#include "sampler.h"

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL SamplerTiled: public Sampler
{
	PY_HEADER(Sampler)
public:

	const TResolution2D& resolution() const { return doResolution(); }
	void setResolution(const TResolution2D& resolution) { doSetResolution(resolution); }

	size_t samplesPerPixel() const { return doSamplesPerPixel(); }
	void setSamplesPerPixel(size_t samplesPerPixel) { doSetSamplesPerPixel(samplesPerPixel); }

protected:

	SamplerTiled();

private:
	class TaskTiled : public Task
	{
	public:
		TaskTiled(size_t id, const TResolution2D& begin, const TResolution2D& end, size_t samplesPerPixel);
	private:
		bool doDrawSample(Sampler& sampler, const TimePeriod& period, Sample& sample) override;
		TResolution2D begin_;
		TResolution2D end_;
		TResolution2D pixel_;
		size_t samplesPerPixel_;
		size_t subPixel_;
	};

	TTaskPtr doGetTask() override;

	void sample(const TResolution2D& pixel, size_t subPixel, const TimePeriod& period, Sample& sample);

	virtual const TResolution2D& doResolution() const = 0;
	virtual size_t doSamplesPerPixel() const = 0;
	virtual void doSetResolution(const TResolution2D& resolution) = 0;
	virtual void doSetSamplesPerPixel(size_t samplesPerPixel) = 0;

	virtual void doSampleScreen(const TResolution2D& pixel, size_t subPixel, TSample2D& screenCoordinate) = 0;
	virtual void doSampleLens(const TResolution2D& pixel, size_t subPixel, TSample2D& lensCoordinate) = 0;
	virtual void doSampleTime(const TResolution2D& pixel, size_t subPixel, const TimePeriod& period, TTime& time) = 0;
	virtual void doSampleWavelength(const TResolution2D& pixel, size_t subPixel, TWavelength& wavelength, TScalar& pdf) = 0;
	virtual void doSampleSubSequence1D(const TResolution2D& pixel, size_t subPixel, TSubSequenceId id, TSample1D* first, TSample1D* last) = 0;
	virtual void doSampleSubSequence2D(const TResolution2D& pixel, size_t subPixel, TSubSequenceId id, TSample2D* first, TSample2D* last) = 0;

	size_t nextId_;
};


}

}

#endif

// EOF
