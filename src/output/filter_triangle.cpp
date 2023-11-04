/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2021  Bram de Greve (bramz@users.sourceforge.net)
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
#include "filter_triangle.h"

namespace liar
{
namespace output
{

PY_DECLARE_CLASS_DOC(FilterTriangle, "Triangle reconstruction filter");
PY_CLASS_CONSTRUCTOR_1(FilterTriangle, const TRenderTargetPtr&)
PY_CLASS_CONSTRUCTOR_2(FilterTriangle, const TRenderTargetPtr&, FilterTriangle::TValue)
PY_CLASS_MEMBER_RW(FilterTriangle, target, setTarget)
PY_CLASS_MEMBER_RW(FilterTriangle, width, setWidth)

// --- public --------------------------------------------------------------------------------------

FilterTriangle::FilterTriangle(const TRenderTargetPtr& target):
	target_(target),
	width_(2)
{
}



FilterTriangle::FilterTriangle(const TRenderTargetPtr& target, TValue width):
	target_(target),
	width_(width)
{
}



const TRenderTargetPtr& FilterTriangle::target() const
{
	return target_;
}



void FilterTriangle::setTarget(const TRenderTargetPtr& target)
{
	target_ = target;
}



FilterTriangle::TValue FilterTriangle::width() const
{
	return width_;
}



void FilterTriangle::setWidth(TValue width)
{
	width_ = std::max(num::NumTraits<TValue>::zero, width);
	filterWidth_ = num::numCast<int>(num::ceil(width));
}



// --- private -------------------------------------------------------------------------------------

const TResolution2D FilterTriangle::doResolution() const
{
	return target_->resolution();
}



void FilterTriangle::doBeginRender()
{
	target_->beginRender();
}



void FilterTriangle::doWriteRender(const OutputSample* first, const OutputSample* last)
{
	typedef OutputSample::TValue TValue;

	const TVector2D res(resolution());
	const TVector2D invRes = res.reciprocal();

	const size_t filterExtent = 2 * static_cast<size_t>(filterWidth_) + 1;
	const size_t filterFootprint = filterExtent * filterExtent;
	const size_t bufferSize = 16 * filterFootprint;

	outputBuffer_.growTo(bufferSize);
	OutputSample* begin = outputBuffer_.begin();
	OutputSample* end = begin + bufferSize;

	OutputSample* output = begin;
	while (first != last)
	{
		const TVector2D p = res * first->screenCoordinate().position();
		const TVector2D p0 = p.transform(num::floor) + .5f;
		const prim::Vector2D<TValue> dp(p - p0);

		for (int j = -filterWidth_; j <= filterWidth_; ++j)
		{
			const TValue dy = dp.y + static_cast<TValue>(j);
			const TValue fy = filterKernel(dy);
			const TScalar y = (p0.y + static_cast<TScalar>(j)) * invRes.y;
			for (int i = -filterWidth_; i <= filterWidth_; ++i)
			{
				const TValue dx = dp.x + static_cast<TValue>(i);
				const TValue fx = filterKernel(dx);
				const TScalar x = (p0.x + static_cast<TScalar>(i)) * invRes.x;

				LASS_ASSERT(output != end);
				*output++ = OutputSample(*first, TPoint2D(x, y), fx * fy);
			}
		}

		if (output == end)
		{
			target_->writeRender(begin, end);
			output = begin;
		}
		++first;
	}
	if (output != begin)
	{
		target_->writeRender(begin, output);
	}
}



void FilterTriangle::doEndRender()
{
	target_->endRender();
}



bool FilterTriangle::doIsCanceling() const
{
	return target_->isCanceling();
}



inline FilterTriangle::TValue FilterTriangle::filterKernel(TValue x) const
{
	return std::max(num::NumTraits<TValue>::zero, num::abs(x - width_));
}




// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
