/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::OutputSample
 *  @brief information about sample to send to RenderTarget
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_OUTPUT_SAMPLE_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_OUTPUT_SAMPLE_H

#include "kernel_common.h"
#include "xyz.h"

namespace liar
{
namespace kernel
{

class Sample;

class LIAR_KERNEL_DLL OutputSample
{
public:

	OutputSample();
	OutputSample(const Sample& sample, const XYZ& radiance, TScalar depth,
		TScalar alpha = 1, TScalar weight = 1);
	OutputSample(const OutputSample& other, const TPoint2D& screenCoordinate, TScalar weight);

	const XYZ& radiance() const { return radiance_; }
	const TPoint2D& screenCoordinate() const { return screenCoordinate_; }
	const TScalar depth() const { return depth_; }
	const TScalar alpha() const { return alpha_; }
	const TScalar weight() const { return weight_; }

	void setRadiance(const XYZ& radiance) { radiance_ = radiance; }

private:

	XYZ radiance_;
	TPoint2D screenCoordinate_;
	TScalar depth_;
	TScalar alpha_;
	TScalar weight_;
};

typedef std::vector<OutputSample> TOutputSamples;

}

}

#endif

// EOF
