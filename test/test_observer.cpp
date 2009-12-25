#include "kernel/xyz.h"
#include <lass/num/random.h>
#include <lass/num/distribution.h>
#include <lass/util/environment.h>

int test_observer(int, char*[])
{
	const size_t numSamples = 1000000;

	using namespace liar;
	using namespace lass;

	const kernel::Observer& observer = kernel::standardObserver();

	num::RandomMT19937 rng;
	rng.seed(1234);
	num::DistributionUniform<TScalar, num::RandomMT19937> uniform(rng);

	typedef std::vector<kernel::XYZ> TXYZs;
	TXYZs powers;
	powers.push_back(kernel::XYZ(1, 1, 1));
	powers.push_back(kernel::XYZ(1, 0, 0));
	powers.push_back(kernel::XYZ(0, 1, 0));
	powers.push_back(kernel::XYZ(0, 0, 1));
	powers.push_back(kernel::XYZ(.2f, .5f, .3f));

	typedef std::vector<TFrequency> TBins;
	TBins bins;
	const TFrequency fmin = 3.8e14;
	const TFrequency fmax = 8.2e14;
	const size_t nBins = 10;
	for (size_t k = 0; k < nBins; ++k)
	{
		bins.push_back(fmin + k * (fmax - fmin) / nBins);
	}
	bins.push_back(fmax);

	const std::string path = util::getEnvironment<std::string>("TEST_SOURCE_DIR") + "/test_observer.out";
	std::ifstream reference(path.c_str());

	for (TXYZs::const_iterator power = powers.begin(); power != powers.end(); ++power)
	{
		typedef std::vector<size_t> THist;
		THist hist(nBins, 0);
		for (size_t k = 0; k < numSamples; ++k)
		{
			const TScalar xi = uniform();
			kernel::XYZ chromaticity;
			TScalar pdf;
			const TScalar f = observer.sample(*power, xi, chromaticity, pdf);

			const TBins::const_iterator bin = std::lower_bound(bins.begin(), bins.end(), f);
			if (bin == bins.begin() || bin == bins.end())
			{
				return 1; // f is out of range
			}
			++hist[bin - bins.begin() - 1];
		}

		std::string line;
		std::getline(reference, line);
		std::istringstream buffer(line);
		for (THist::const_iterator count = hist.begin(); count != hist.end(); ++count)
		{
			const TScalar pTest = static_cast<TScalar>(*count) / numSamples;
			TScalar pRef = -1;
			buffer >> pRef;
			if (pRef < 0 || pRef > 1)
			{
				return 1;
			}
			if (num::abs(pRef - pTest) > 1e-3)
			{
				return 1;
			}
		}	
	}

	return 0;
}