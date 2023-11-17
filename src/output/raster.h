/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2023  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::output::Raster
 *  @brief common base for raster based render targets like Image and Display.
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_OUTPUT_RASTER_H
#define LIAR_GUARDIAN_OF_INCLUSION_OUTPUT_RASTER_H

#include "output_common.h"
#include "../kernel/render_target.h"
#include "../kernel/rgb_space.h"
#include <lass/util/dictionary.h>

namespace liar
{
namespace output
{

class LIAR_OUTPUT_DLL Raster: public RenderTarget
{
    PY_HEADER(RenderTarget)
public:
    typedef OutputSample::TValue TValue;

    enum class ToneMapping
    {
        Linear = 0,
        CompressY,
        CompressRGB,
        Reinhard2002Y,
        Reinhard2002RGB,
        ExponentialY,
        ExponentialRGB,
        DuikerY,
        DuikerRGB,
        size
    };

    const TRgbSpacePtr& rgbSpace() const;
    ToneMapping toneMapping() const;
    TValue exposureStops() const;
    TValue exposureCorrectionStops() const;
    bool autoExposure() const;
    TValue middleGrey() const;

    void setRgbSpace(const TRgbSpacePtr& rgbSpace);
    void setToneMapping(ToneMapping mode);
    void setExposureStops(TValue fStops);
    void setExposureCorrectionStops(TValue stops);
    void setAutoExposure(bool enable = true);
    void setMiddleGrey(TValue grey);

    void nextToneMapping();

protected:

    Raster(const TResolution2D& resolution);

    typedef std::vector<XYZ> TRenderBuffer;
    typedef std::vector<OutputSample::TValue> TValueBuffer;
    typedef std::vector<prim::ColorRGBA> TTonemapBuffer;
    typedef prim::Aabb2D<size_t> TDirtyBox;

    void beginRaster();
    TDirtyBox tonemap(const TRgbSpacePtr& destSpace);

    const TTonemapBuffer& tonemapBuffer() const;
    const TValueBuffer& totalWeight() const;
    const TDirtyBox& dirtyBox() const;
    void clearDirtyBox();

private:

    const TResolution2D doResolution() const;

    void doWriteRender(const OutputSample* first, const OutputSample* last);

    TValue sceneGain() const;
    TValue averageSceneLuminance() const;
    XYZ weighted(size_t index, TValue gain = 1) const;

    TRenderBuffer renderBuffer_;
    TTonemapBuffer tonemapBuffer_;
    TValueBuffer totalWeight_;
    TValueBuffer alphaBuffer_;
    mutable TDirtyBox renderDirtyBox_;
    TDirtyBox allTimeDirtyBox_;
    mutable std::recursive_mutex   renderLock_;
    TResolution2D resolution_;
    TRgbSpacePtr rgbSpace_;
    ToneMapping toneMapping_;
    mutable TValue exposureStops_;
    TValue exposureCorrectionStops_;
    TValue middleGrey_;
    TValue maxSceneLuminance_;
    bool autoExposure_;
    mutable bool isDirtyAutoExposure_;
};

}

}

PY_SHADOW_STR_ENUM(LASS_DLL_EXPORT, liar::output::Raster::ToneMapping)

#endif

// EOF
