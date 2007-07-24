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
 *  http://liar.bramz.org
 */

/** @class liar::output::Splitter
 *  @brief hook several RenderTargets on the output
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_OUTPUT_SPLITTER_H
#define LIAR_GUARDIAN_OF_INCLUSION_OUTPUT_SPLITTER_H

#include "output_common.h"
#include "../kernel/render_target.h"

namespace liar
{
namespace output
{

class LIAR_OUTPUT_DLL Splitter: public RenderTarget
{
    PY_HEADER(RenderTarget)
public:

	typedef std::vector<TRenderTargetPtr> TChildren;

	Splitter();
	Splitter(const TChildren& children);

	const TChildren& children() const;
	void setChildren(const TChildren& children);
	void add(const TRenderTargetPtr& child);
	void add(const TChildren& children);

private:

	const TResolution2D doResolution() const;
    void doBeginRender();
	void doWriteRender(const OutputSample* first, const OutputSample* last);
    void doEndRender();
	const bool doIsCanceling() const;

	TChildren children_;
};



}

}

#endif

// EOF
