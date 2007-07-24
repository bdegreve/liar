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
 *  http://liar.bramz.org
 */

#include "kernel_common.h"
#include "sample.h"
#include "sampler.h"

namespace liar
{
namespace kernel
{

// --- public --------------------------------------------------------------------------------------

Sample::Sample():
    screenCoordinate_(TSample2D(0, 0)),
    lensCoordinate_(TSample2D(0, 0)),
	time_(0.f),
    weight_(1),
	sampler_(0)
{
}



const Sample::TSample2D& Sample::screenCoordinate() const
{
    return screenCoordinate_;
}



const Sample::TSample2D& Sample::lensCoordinate() const
{
    return lensCoordinate_;
}


const TTime Sample::time() const
{
	return time_;
}



const TScalar Sample::weight() const
{
    return weight_;
}



void Sample::setWeight(TScalar iWeight)
{
    weight_ = iWeight;
}



const Sample::TSubSequence1D Sample::subSequence1D(int id) const
{
	if (id < 0)
	{
		return TSubSequence1D();
	}
	LASS_ASSERT(sampler_);
	const unsigned k = sampler_->subSequenceOffset1D(id);
	const unsigned n = sampler_->subSequenceSize1D(id);
	return TSubSequence1D(&subSequences1D_[0] + k, &subSequences1D_[0] + k + n);
}



const Sample::TSubSequence2D Sample::subSequence2D(int id) const
{
	if (id < 0)
	{
		return TSubSequence2D();
	}
	LASS_ASSERT(sampler_);
	const unsigned k = sampler_->subSequenceOffset2D(id);
	const unsigned n = sampler_->subSequenceSize2D(id);
	return TSubSequence2D(&subSequences2D_[0] + k, &subSequences2D_[0] + k + n);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
