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

#include "output_common.h"
#include "depth_channel.h"

namespace liar
{
namespace output
{

PY_DECLARE_CLASS_DOC(DepthChannel, "converts depth channel to color information");
PY_CLASS_CONSTRUCTOR_2(DepthChannel, const TRenderTargetPtr&, TScalar)
PY_CLASS_MEMBER_RW(DepthChannel, target, setTarget)
PY_CLASS_MEMBER_RW(DepthChannel, unit, setUnit)

// --- public --------------------------------------------------------------------------------------

DepthChannel::DepthChannel(const TRenderTargetPtr& target, TScalar unit):
	target_(target),
	unit_(unit),
	invUnit_(num::inv(unit))
{
}



const TRenderTargetPtr& DepthChannel::target() const
{
	return target_;
}



void DepthChannel::setTarget(const TRenderTargetPtr& target)
{
	target_ = target;
}



TScalar DepthChannel::unit() const
{
	return unit_;
}



void DepthChannel::setUnit(TScalar unit)
{
	if (unit <= 0)
	{
		unit = 1;
	}
	unit_ = unit;
	invUnit_ = num::inv(unit);
}



// --- private -------------------------------------------------------------------------------------

const TResolution2D DepthChannel::doResolution() const
{
	return target_->resolution();
}



void DepthChannel::doBeginRender()
{
	target_->beginRender();
}



void DepthChannel::doWriteRender(const OutputSample* first, const OutputSample* last)
{
	outputBuffer_.growTo(bufferSize_);
	OutputSample* begin = outputBuffer_.begin();
	OutputSample* end = begin + bufferSize_;

	OutputSample* output = begin;
	while (first != last)
	{
		*output = *first++;
		output->setRadiance(XYZ(static_cast<XYZ::TValue>(output->depth() * invUnit_)));
		if (++output == end)
		{
			target_->writeRender(begin, end);
			output = begin;
		}
	}
	if (output != begin)
	{
		target_->writeRender(begin, output);
	}
}



void DepthChannel::doEndRender()
{
	target_->endRender();
}



bool DepthChannel::doIsCanceling() const
{
	return target_->isCanceling();
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
