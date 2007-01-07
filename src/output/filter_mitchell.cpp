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

#include "output_common.h"
#include "filter_mitchell.h"
#include "../kernel/per_thread_buffer.h"

namespace liar
{
namespace output
{

PY_DECLARE_CLASS(FilterMitchell);
PY_CLASS_CONSTRUCTOR_1(FilterMitchell, const TRenderTargetPtr&)
PY_CLASS_CONSTRUCTOR_2(FilterMitchell, const TRenderTargetPtr&, TScalar)
PY_CLASS_MEMBER_RW(FilterMitchell, "target", target, setTarget)
PY_CLASS_MEMBER_RW(FilterMitchell, "b", b, setB)

// --- public --------------------------------------------------------------------------------------

FilterMitchell::FilterMitchell(const TRenderTargetPtr& target):
	target_(target),
	b_(TNumTraits::one / 3)
{
}



FilterMitchell::FilterMitchell(const TRenderTargetPtr& target, TScalar b):
	target_(target),
	b_(b)
{
}



const TRenderTargetPtr& FilterMitchell::target() const
{
	return target_;
}



void FilterMitchell::setTarget(const TRenderTargetPtr& target)
{
	target_ = target;
}



const TScalar FilterMitchell::b() const
{
	return b_;
}



void FilterMitchell::setB(TScalar b)
{
	b_ = b;
}



// --- private -------------------------------------------------------------------------------------

const FilterMitchell::TResolution FilterMitchell::doResolution() const
{
	return target_->resolution();
}



void FilterMitchell::doBeginRender()
{
	target_->beginRender();
}



void FilterMitchell::doWriteRender(const OutputSample* first, const OutputSample* last)
{
	const TVector2D res(resolution());
	const TVector2D invRes = res.reciprocal();
	
	static PerThreadBuffer<OutputSample> perThreadBuffer;
	perThreadBuffer.growTo(bufferSize_);
	OutputSample* buffer = perThreadBuffer.begin();

	LASS_ASSERT(bufferSize_ % filterFootprint_ == 0);

	size_t current = 0;
	while (first != last)
	{
		const Spectrum radiance = first->radiance();
		const TScalar alpha = first->alpha();

		const TVector2D p = res * first->screenCoordinate().position();
		const TVector2D p0 = p.transform(num::floor) + .5f;
		const TVector2D dp = p - p0;

		TVector2D f[filterExtent_];
		for (int i = 0; i < filterExtent_; ++i)
		{
			f[i] = filterKernel(dp + (i - filterWidth_));
		}
		for (int j = 0; j < filterExtent_; ++j)
		{
			const TScalar y = (p0.y + j - filterWidth_) * invRes.y;
			for (int i = 0; i < filterExtent_; ++i)
			{
				const TScalar x = (p0.x + i - filterWidth_) * invRes.x;
				const TScalar w = f[i].x * f[j].y;
				LASS_ASSERT(current < bufferSize_);
				buffer[current++] = OutputSample(*first, TPoint2D(x, y), w);
			}
		}

		if (current == bufferSize_)
		{
			target_->writeRender(buffer, buffer + current);
			current = 0;
		}
		++first;
	}
	if (current > 0)
	{
		target_->writeRender(buffer, buffer + current);
	}
}



void FilterMitchell::doEndRender()
{
	target_->endRender();
}



const bool FilterMitchell::doIsCanceling() const
{
	return target_->isCanceling();
}



inline const TScalar FilterMitchell::filterKernel(TScalar x) const
{
	const TScalar B = b_;
	const TScalar C = (1 - B) / 2;
	x = num::abs(x);
	if (x < 1)
	{
		return ((12 - 9 * B - 6 * C) * num::cubic(x) + (-18 + 12 * B + 6 * C) * num::sqr(x) + (6 - 2 * B)) / 6;
	}
	if (x < 2)
	{
		return ((-B - 6 * C) * num::cubic(x) + (6 * B + 30 * C) * num::sqr(x) + (-12 * B - 48 * C) * x + (8 * B + 24 * C)) / 6;
	}
	return 0;
}



inline const TVector2D FilterMitchell::filterKernel(const TVector2D& p) const
{
	return TVector2D(filterKernel(p.x), filterKernel(p.y));
}




// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
