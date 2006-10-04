/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2006  Bram de Greve
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

/** @class liar::OutputSample
 *  @brief information about sample to send to RenderTarget
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_OUTPUT_SAMPLE_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_OUTPUT_SAMPLE_H

#include "kernel_common.h"
#include "spectrum.h"

namespace liar
{
namespace kernel
{

class Sample;

class LIAR_KERNEL_DLL OutputSample
{
public:

	OutputSample();
	OutputSample(const Sample& sample, const Spectrum& radiance);

	const Spectrum& radiance() const { return radiance_; }
	const TPoint2D& screenCoordinate() const { return screenCoordinate_; }
	const TScalar weight() const { return weight_; }

private:

	Spectrum radiance_;
	TPoint2D screenCoordinate_;
	TScalar weight_;
};

typedef std::vector<OutputSample> TOutputSamples;

}

}

#endif

// EOF
