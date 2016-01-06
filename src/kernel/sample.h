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

/** @class liar::Sample
 *  @brief representation of a single sample that must be rendered
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SAMPLE_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SAMPLE_H

#include "kernel_common.h"
#include "bands.h"
#include <lass/prim/point_2d.h>
#include <lass/stde/iterator_range.h>

namespace liar
{
namespace kernel
{

class Sampler;

class LIAR_KERNEL_DLL Sample
{
public:

	//typedef  TSampleSequence1D;
	//typedef  TSampleSequence2D;

	typedef TScalar TSample1D;
	typedef TPoint2D TSample2D;
	typedef stde::iterator_range<const TSample1D*> TSubSequence1D;
	typedef stde::iterator_range<const TSample2D*> TSubSequence2D;

	Sample();

	const TSample2D& screenSample() const;
	void setScreenSample(const TSample2D& sample);

	const TSample2D& lensSample() const;
	void setLensSample(const TSample2D& sample);

	TTime time() const;
	void setTime(TTime time);

	TWavelength wavelength() const;
	TWavelength wavelength(TScalar &pdf) const;
	void setWavelengthSample(TSample1D sample);

	TScalar weight() const;
	void setWeight(TScalar weight);

	const TSubSequence1D subSequence1D(int id) const;
	const TSubSequence2D subSequence2D(int id) const;

	void setSubSample1D(int id, size_t offset, TSample1D sample);
	void setSubSample2D(int id, size_t offset, TSample2D sample);

private:

	friend class Sampler;
	friend class SamplerTiled; // until we cleared up the mess.

	TSample2D screenSample_;
	TSample2D lensSample_;
	TTime time_;
	TWavelength wavelength_;
	TScalar wavelengthPdf_;
	TScalar weight_;
	std::vector<TSample1D> subSequences1D_;
	std::vector<TSample2D> subSequences2D_;
	Sampler* sampler_;
};

}

}

#endif

// EOF
