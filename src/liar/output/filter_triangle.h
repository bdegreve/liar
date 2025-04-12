/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2021-2025  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::output::FilterTriangle
 *  @brief Applies Mitchell & Netravali filter to OutputSamples
 *  @author Bram de Greve [Bramz]
 *
 *  @par reference:
 *		@arg Don P. Mitchell, Arun N. Netravali <i>Reconstruction Filters in Computer Graphics</i>,
 *		Computer Graphics (SIGGRAPH88), <b>22</b> (4), 221--228 (1988).
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_OUTPUT_FILTER_TRIANGLE_H
#define LIAR_GUARDIAN_OF_INCLUSION_OUTPUT_FILTER_TRIANGLE_H

#include "output_common.h"
#include "../kernel/render_target.h"
#include "../kernel/per_thread_buffer.h"

namespace liar
{
namespace output
{

class LIAR_OUTPUT_DLL FilterTriangle: public RenderTarget
{
	PY_HEADER(RenderTarget)
public:

	typedef OutputSample::TValue TValue;

	FilterTriangle(const TRenderTargetRef& target);
	FilterTriangle(const TRenderTargetRef& target, TValue width);

	const TRenderTargetRef& target() const;
	void setTarget(const TRenderTargetRef& target);
	TValue width() const;
	void setWidth(TValue width);

private:

	typedef PerThreadBuffer<OutputSample> TOutputBuffer;

	const TResolution2D doResolution() const override;
	void doBeginRender() override;
	void doWriteRender(const OutputSample* first, const OutputSample* last) override;
	void doEndRender() override;
	bool doIsCanceling() const override;

	TValue filterKernel(TValue x) const;

	TOutputBuffer outputBuffer_;
	TRenderTargetRef target_;
	TValue width_;
	int filterWidth_;
};



}

}

#endif

// EOF
