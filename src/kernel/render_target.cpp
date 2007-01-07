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
 *  http://liar.sourceforge.net
 */

#include "kernel_common.h"
#include "render_target.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS(RenderTarget)

// --- public --------------------------------------------------------------------------------------

RenderTarget::~RenderTarget()
{
	LASS_ASSERT(!isRendering_);
}



// --- protected -----------------------------------------------------------------------------------

RenderTarget::RenderTarget():
	isRendering_(false)
{
}



const RenderTarget::TResolution RenderTarget::resolution() const
{
	return doResolution();
}



void RenderTarget::beginRender()
{
	LASS_LOCK(lock_)
	{
		if (!isRendering_)
		{
			doBeginRender();
			isRendering_ = true;
		}
	}
}



void RenderTarget::writeRender(const OutputSample* first, const OutputSample* last)
{
	if (!isRendering_)
	{
		beginRender();
	}
	doWriteRender(first, last);
}



void RenderTarget::endRender()
{
	if (isRendering_)
	{
		doEndRender();
		isRendering_ = false;
	}
}



const bool RenderTarget::isRendering() const
{
	return isRendering_;
}



// --- private -------------------------------------------------------------------------------------

const bool RenderTarget::doIsCanceling() const
{
	return false;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
