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
	weight_()
{
}



OutputSample::OutputSample(const Sample& sample, const Spectrum& radiance):
	radiance_(radiance),
	screenCoordinate_(sample.screenCoordinate()),
	weight_(sample.weight())
{
}



// --- protected -----------------------------------------------------------------------------------

// --- private -------------------------------------------------------------------------------------

// --- free ----------------------------------------------------------------------------------------

}

}

// EOF