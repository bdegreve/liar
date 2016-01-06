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

/** @class liar::samplers::Stratifier
*  @author Bram de Greve [Bramz]
*/

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SAMPLERS_HALTON_H
#define LIAR_GUARDIAN_OF_INCLUSION_SAMPLERS_HALTON_H

#include "samplers_common.h"
#include "../kernel/sampler_progressive.h"
#include <lass/num/random.h>
#include <lass/num/distribution.h>

namespace liar
{
namespace samplers
{

class ScrambledRadicalInverse
{
public:
	typedef std::vector<size_t> TScrambler;

	ScrambledRadicalInverse(size_t base = 2) : base_(base), state_(0) {}

	void setBase(size_t base) { base_ = base; }
	void seed(size_t state) { state_ = state; }

	TScalar operator()(const TScrambler& scrambler)
	{
		LASS_ASSERT(scrambler.size() == base_);
		const TScalar invBase = 1 / static_cast<TScalar>(base_);
		TScalar weight = invBase;
		TScalar result = 0;
		size_t i = ++state_;
		while (i)
		{
			const size_t r = i % base_;
			i /= base_;
			result += scrambler[r] * weight;
			weight *= invBase;
		}
		return result;
	}
private:
	size_t base_;
	size_t state_;
};


class LIAR_SAMPLERS_DLL Halton : public SamplerProgressive
{
	PY_HEADER(SamplerProgressive)
public:

	Halton();

	size_t samplesPerTask() const;
	void setSamplesPerTask(size_t samplesPerTask);

private:

	typedef ScrambledRadicalInverse::TScrambler TScrambler;
	typedef std::vector<TScrambler> TScramblers;

	class TaskHalton: public Task
	{
	public:
		TaskHalton(size_t id, const Halton& sampler);
	private:
		bool doDrawSample(Sampler& sampler, const TimePeriod& period, Sample& sample) override;

		void seed(const Halton& halton);

		num::RandomRadicalInverse filmX_;
		num::RandomRadicalInverse filmY_;
		num::RandomRadicalInverse lensX_;
		num::RandomRadicalInverse lensY_;
		num::RandomRadicalInverse time_;
		num::RandomRadicalInverse wavelength_;
		std::vector<ScrambledRadicalInverse> subs1D_;
		std::vector<ScrambledRadicalInverse> subs2DX_;
		std::vector<ScrambledRadicalInverse> subs2DY_;
		size_t samplesLeft_;
		bool isSeeded_;
	};

	TTaskPtr doGetTask() override;

	void doSeed(TSeed randomSeed) override;
	const TSamplerPtr doClone() const  override;
	const TPyObjectPtr doGetState() const  override;
	void doSetState(const TPyObjectPtr& state)  override;

	static bool initialize();

	static std::vector<size_t> primes_;
	static TScramblers scramblers_;
	static bool isInitialized_;

	size_t samplesPerTask_;
	size_t nextId_;
};



}

}

#endif

// EOF
