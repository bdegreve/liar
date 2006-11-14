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

/** @class liar::RenderTarget
 *  @brief render target, canvas or film of camera
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_RENDER_TARGET_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_RENDER_TARGET_H

#include "kernel_common.h"
#include "differential_ray.h"
#include "output_sample.h"
#include "sample.h"
#include "spectrum.h"
#include <lass/prim/vector_2d.h>

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL RenderTarget: public python::PyObjectPlus
{
    PY_HEADER(python::PyObjectPlus)
public:

    typedef prim::Vector2D<unsigned> TResolution;

    virtual ~RenderTarget();

	const TResolution resolution() const;	

    void beginRender();
    void writeRender(const OutputSample* first, const OutputSample* last);
    void endRender();
	const bool isRendering() const;

	const bool isCanceling() const { return doIsCanceling(); }

protected:

    RenderTarget();

private:

	virtual const TResolution doResolution() const = 0;

    virtual void doBeginRender() = 0;
    virtual void doWriteRender(const OutputSample* first, const OutputSample* last) = 0;
    virtual void doEndRender() = 0;
	virtual const bool doIsCanceling() const;

	util::CriticalSection lock_;
    bool isRendering_;
};

typedef python::PyObjectPtr<RenderTarget>::Type TRenderTargetPtr;

}

}

#endif

// EOF
