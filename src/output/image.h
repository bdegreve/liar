/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2005  Bram de Greve
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

/** @class liar::output::Image
 *  @brief render target to image file
 *  @author Bram de Greve [BdG]
 *
 *  The image render target does not apply any filter on the ouput samples.  All it does, is to finding the
 *  pixel that contains the sample and add it to the pixel value.
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_OUTPUT_IMAGE_H
#define LIAR_GUARDIAN_OF_INCLUSION_OUTPUT_IMAGE_H

#include "output_common.h"
#include "../kernel/render_target.h"
#include <lass/io/image.h>

namespace liar
{
namespace output
{

class LIAR_OUTPUT_DLL Image: public kernel::RenderTarget
{
    PY_HEADER(kernel::RenderTarget)
public:

	typedef prim::Vector2D<size_t> TSize;

	Image(const std::string& iFilename, const TSize& iSize, TScalar iGamma = 1);
    ~Image();

	const TSize& size() const;
    const TScalar gamma() const;
    void setGamma(TScalar iGamma);

private:

	typedef std::vector<TScalar> TWeights;

    virtual void doBeginRender();
    virtual void doWriteRender(const kernel::Sample& iSample, const TSpectrum& iRadiance);
    virtual void doEndRender();

    io::Image image_;
	TWeights weights_;
    const std::string filename_;
    TSize size_;
    TScalar gamma_;
    bool isSaved_;
};



}

}

#endif

// EOF
