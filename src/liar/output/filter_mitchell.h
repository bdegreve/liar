/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2025  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::output::FilterMitchell
 *  @brief Applies Mitchell & Netravali filter to OutputSamples
 *  @author Bram de Greve [Bramz]
 *
 *  @par reference:
 *		@arg Don P. Mitchell, Arun N. Netravali <i>Reconstruction Filters in Computer Graphics</i>,
 *		Computer Graphics (SIGGRAPH88), <b>22</b> (4), 221--228 (1988).
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_OUTPUT_FILTER_MITCHELL_H
#define LIAR_GUARDIAN_OF_INCLUSION_OUTPUT_FILTER_MITCHELL_H

#include "output_common.h"
#include "../kernel/render_target.h"
#include "../kernel/per_thread_buffer.h"

namespace liar
{
namespace output
{

class LIAR_OUTPUT_DLL FilterMitchell: public RenderTarget
{
	PY_HEADER(RenderTarget)
public:

	FilterMitchell(const TRenderTargetRef& target);
	FilterMitchell(const TRenderTargetRef& target, TScalar b);

	const TRenderTargetRef& target() const;
	void setTarget(const TRenderTargetRef& target);
	TScalar b() const;
	void setB(TScalar b);

private:

	typedef PerThreadBuffer<OutputSample> TOutputBuffer;

	static constexpr int filterWidth_ = 2;
	static constexpr int filterExtent_ = 2 * filterWidth_ + 1;
	static constexpr size_t filterFootprint_ = filterExtent_ * filterExtent_;
	static constexpr size_t bufferSize_ = filterFootprint_ * 16;

	const TResolution2D doResolution() const override;
	void doBeginRender() override;
	void doWriteRender(const OutputSample* first, const OutputSample* last) override;
	void doEndRender() override;
	bool doIsCanceling() const override;

	TScalar filterKernel(TScalar x) const;
	const TVector2D filterKernel(const TVector2D& p) const;

	TOutputBuffer outputBuffer_;
	TRenderTargetRef target_;
	TScalar b_;
};



}

}

#endif

// EOF
