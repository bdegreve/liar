/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
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

    const TRgbSpacePtr& rgbSpace() const;
    const std::string toneMapping() const;
    TScalar exposureStops() const;
    TScalar exposureCorrectionStops() const;
    bool autoExposure() const;
    TScalar middleGrey() const;

    void setRgbSpace(const TRgbSpacePtr& rgbSpace);
    void setToneMapping(const std::string& mode);
    void setExposureStops(TScalar fStops);
    void setExposureCorrectionStops(TScalar stops);
    void setAutoExposure(bool enable = true);
    void setMiddleGrey(TScalar grey);

    void nextToneMapping();

protected:

    Raster(const TResolution2D& resolution);

    typedef std::vector<XYZ> TRenderBuffer;
    typedef std::vector<TScalar> TWeightBuffer;
    typedef std::vector<prim::ColorRGBA> TTonemapBuffer;
    typedef prim::Aabb2D<size_t> TDirtyBox;

    void beginRaster();
    TDirtyBox tonemap(const TRgbSpacePtr& destSpace);

    const TTonemapBuffer& tonemapBuffer() const;
    const TWeightBuffer& totalWeight() const;
    const TDirtyBox& dirtyBox() const;
    void clearDirtyBox();

    util::CriticalSection& renderLock() const;

private:

    enum ToneMapping
    {
        tmLinear = 0,
        tmCompressY,
        tmCompressRGB,
        tmReinhard2002Y,
        tmReinhard2002RGB,
        tmExponentialY,
        tmExponentialRGB,
        tmDuikerY,
        tmDuikerRGB,
        numToneMapping
    };
    typedef util::Dictionary<std::string, ToneMapping> TToneMappingDictionary;

    const TResolution2D doResolution() const;

    void doWriteRender(const OutputSample* first, const OutputSample* last);
 
    TScalar sceneGain() const;
    TScalar averageSceneLuminance() const;
    XYZ weighted(size_t index, TScalar gain = 1.f) const;

    static TToneMappingDictionary makeToneMappingDictionary();

    TRenderBuffer renderBuffer_;
    TTonemapBuffer tonemapBuffer_;
    TWeightBuffer totalWeight_;
    TWeightBuffer alphaBuffer_;
    mutable TDirtyBox renderDirtyBox_;
    TDirtyBox allTimeDirtyBox_;
    mutable util::CriticalSection renderLock_;
    TResolution2D resolution_;
    TRgbSpacePtr rgbSpace_;
    ToneMapping toneMapping_;
    mutable TScalar exposureStops_;
    TScalar exposureCorrectionStops_;
    TScalar middleGrey_;
    TScalar maxSceneLuminance_;
    bool autoExposure_;
    mutable bool isDirtyAutoExposure_;
    
    static TToneMappingDictionary toneMappingDictionary_;
};



}

}

#endif

// EOF
