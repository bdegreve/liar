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

#include "kernel_common.h"
#include "output_sample.h"
#include "sample.h"

namespace liar
{
namespace kernel
{

// --- public --------------------------------------------------------------------------------------

OutputSample::OutputSample():
	radiance_(),
	screenCoordinate_(),
	depth_(),
	alpha_(),
	weight_()
{
}



OutputSample::OutputSample(
		const Sample& sample, const XYZ& radiance, TScalar depth, TScalar alpha, 
		TScalar weight):
	radiance_(radiance),
	screenCoordinate_(sample.screenCoordinate()),
	depth_(depth),
	alpha_(alpha),
	weight_(weight)
{
}



OutputSample::OutputSample(
		const OutputSample& other, const TPoint2D& screenCoordinate, TScalar weight):
	radiance_(other.radiance_),
	screenCoordinate_(screenCoordinate),
	depth_(other.depth_),
	alpha_(other.alpha_),
	weight_(other.weight_ * weight)
{
}



// --- protected -----------------------------------------------------------------------------------

// --- private -------------------------------------------------------------------------------------

// --- free ----------------------------------------------------------------------------------------

io::BinaryOStream& operator<<(io::BinaryOStream& stream, const OutputSample& sample)
{
	const short version = 1;
	const XYZ& radiance = sample.radiance();
	const TPoint2D& screenCoordinate = sample.screenCoordinate();
	stream << version << screenCoordinate.x << screenCoordinate.y << radiance.x << radiance.y << radiance.z << sample.depth() << sample.alpha() << sample.weight();
	return stream;
}

}

}

// EOF
