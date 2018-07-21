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

#include "samplers_common.h"
#include "halton.h"
#include "../kernel/sample.h"

namespace liar
{
namespace samplers
{

PY_DECLARE_CLASS_DOC(Halton, "Progressive sampler using scrambled halton sequences");
PY_CLASS_CONSTRUCTOR_0(Halton)

// --- public --------------------------------------------------------------------------------------

Halton::Halton():
	samplesPerTask_(1024),
	nextId_(0)
{
}


size_t Halton::samplesPerTask() const
{
	return samplesPerTask_;
}


void Halton::setSamplesPerTask(size_t samplesPerTask)
{
	samplesPerTask_ = std::max<size_t>(samplesPerTask, 1);
}


// --- protected -----------------------------------------------------------------------------------




// --- private -------------------------------------------------------------------------------------


Halton::TTaskPtr Halton::doGetTask()
{
	return TTaskPtr(new TaskHalton(nextId_++, *this));
}


void Halton::doSeed(TSeed randomSeed)
{
	nextId_ = randomSeed;
}


const TSamplerPtr Halton::doClone() const
{
	return TSamplerPtr(new Halton(*this));
}


const TPyObjectPtr Halton::doGetState() const
{
	return python::makeTuple(nextId_, samplesPerTask_);
}


void Halton::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, nextId_, samplesPerTask_);
}


bool Halton::initialize()
{
	num::RandomMT19937 rng;

	const size_t n = primes_.size();
	scramblers_.resize(n);
	for (size_t i = 0; i < n; ++i)
	{
		const size_t prime = primes_[i];
		TScrambler& scrambler = scramblers_[i];
		scrambler.resize(prime);
		for (size_t k = 0; k < prime; ++k)
		{
			scrambler[k] = k;
		}
		std::random_shuffle(scrambler.begin(), scrambler.end(), rng);
	}

	return true;
}

std::vector<size_t> Halton::primes_ = {
	// 2, 3, 5, 7, 11, 13, 
	17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109,
	113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223,
	227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331,
	337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443,
	449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571,
	577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683,
	691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823,
	827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953,
	967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063,
	1069, 1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181, 1187,
	1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291, 1297,
	1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423, 1427, 1429, 1433,
	1439, 1447, 1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523, 1531, 1543,
	1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637,
	1657, 1663, 1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747, 1753, 1759, 1777,
	1783, 1787, 1789, 1801, 1811, 1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889
};

Halton::TScramblers Halton::scramblers_;
bool Halton::isInitialized_ = Halton::initialize();




// --- free ----------------------------------------------------------------------------------------

Halton::TaskHalton::TaskHalton(size_t id, const Halton& sampler):
	Task(id),
	filmX_(2),
	filmY_(3),
	lensX_(5),
	lensY_(7),
	time_(11),
	wavelength_(13),	
	samplesLeft_(sampler.samplesPerTask()),
	isSeeded_(false)
{
	const size_t n1D = sampler.numSubSequences1D();
	const size_t n2D = sampler.numSubSequences2D();
	if (n1D + 2 * n2D > sampler.primes_.size())
	{
		LASS_THROW("Too many subsequences requested.");
	}

	subs1D_.resize(n1D);
	subs2DX_.resize(n2D);
	subs2DY_.resize(n2D);

	size_t primeIndex = 0;
	for (size_t i = 0; i < n1D; ++i)
	{
		subs1D_[i].setBase(sampler.primes_.at(primeIndex++));
	}
	for (size_t i = 0, n = sampler.numSubSequences2D(); i < n; ++i)
	{
		subs2DX_[i].setBase(sampler.primes_.at(primeIndex++));
		subs2DY_[i].setBase(sampler.primes_.at(primeIndex++));
	}
}


bool Halton::TaskHalton::doDrawSample(Sampler& sampler, const TimePeriod& period, Sample& sample)
{
	typedef Sample::TSample2D TSample2D;

	const Halton& halton = static_cast<Halton&>(sampler);

	if (!samplesLeft_)
	{
		return false;
	}
	if (!isSeeded_)
	{
		seed(halton);
		isSeeded_ = true;
	}
	--samplesLeft_;

	sample.setScreenSample(TSample2D(filmX_(), filmY_()));
	sample.setLensSample(TSample2D(lensX_(), lensY_()));
	sample.setTime(period.interpolate(time_()));
	sample.setWavelengthSample(wavelength_());

	size_t primeIndex = 0;
	for (size_t i = 0, n = halton.numSubSequences1D(); i < n; ++i)
	{
		const TSubSequenceId id = static_cast<TSubSequenceId>(i);
		auto& rng = subs1D_[i];
		const auto& scrambler = halton.scramblers_.at(primeIndex++);
		for (size_t k = 0, m = halton.subSequenceSize1D(id); k < m; ++k)
		{
			sample.setSubSample1D(id, k, rng(scrambler));
		}
	}

	for (size_t i = 0, n = halton.numSubSequences2D(); i < n; ++i)
	{
		const TSubSequenceId id = static_cast<TSubSequenceId>(i);
		auto& rngX = subs2DX_[i];
		auto& rngY = subs2DY_[i];
		const auto& scramblerX = halton.scramblers_.at(primeIndex++);
		const auto& scramblerY = halton.scramblers_.at(primeIndex++);
		for (size_t k = 0, m = halton.subSequenceSize2D(id); k < m; ++k)
		{
			sample.setSubSample2D(id, k, TSample2D(rngX(scramblerX), rngY(scramblerY)));
		}
	}

	return true;
}


void Halton::TaskHalton::seed(const Halton& halton)
{
	const size_t s = (id() + 1) * halton.samplesPerTask() - samplesLeft_;
	filmX_.seed(s);
	filmY_.seed(s);
	lensX_.seed(s);
	lensY_.seed(s);
	time_.seed(s);
	wavelength_.seed(s);

	const size_t n1D = halton.numSubSequences1D();
	subs1D_.resize(n1D);
	for (size_t i = 0; i < n1D; ++i)
	{
		const size_t m = halton.subSequenceSize1D(static_cast<TSubSequenceId>(i));
		subs1D_[i].seed(s * m);
	}

	const size_t n2D = halton.numSubSequences2D();
	for (size_t i = 0, n = n2D; i < n; ++i)
	{
		const size_t m = halton.subSequenceSize2D(static_cast<TSubSequenceId>(i));
		subs2DX_[i].seed(s * m);
		subs2DY_[i].seed(s * m);
	}
}


}

}

// EOF
