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

/** @class liar::output::Image
 *  @brief render target to image file
 *  @author Bram de Greve [Bramz]
 *
 *  The image render target does not apply any filter on the ouput samples.  All it does, is to finding the
 *  pixel that contains the sample and add it to the pixel value.
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_OUTPUT_IMAGE_H
#define LIAR_GUARDIAN_OF_INCLUSION_OUTPUT_IMAGE_H

#include "output_common.h"
#include "../kernel/render_target.h"
#include "../kernel/rgb_space.h"
#include <lass/util/dictionary.h>

namespace liar
{
namespace output
{

class LIAR_OUTPUT_DLL Image: public RenderTarget
{
    PY_HEADER(RenderTarget)
public:

    Image(const std::wstring& filename, const TResolution2D& resolution);
    ~Image();

    void save();

    const std::wstring& path() const;
    const std::string& options() const;
    const TRgbSpacePtr& rgbSpace() const;
    const std::string toneMapping() const;
    TScalar exposureStops() const;
    TScalar exposureCorrectionStops() const;
    bool autoExposure() const;
    TScalar middleGrey() const;

    void setPath(const std::wstring& path);
    void setOptions(const std::string& options);
    void setRgbSpace(const TRgbSpacePtr& rgbSpace);
    void setToneMapping(const std::string& mode);
    void setExposureStops(TScalar fStops);
    void setExposureCorrectionStops(TScalar stops);
    void setAutoExposure(bool enable = true);
    void setMiddleGrey(TScalar grey);

protected:

    typedef std::vector<XYZ> TRenderBuffer;
    typedef std::vector<TScalar> TWeightBuffer;
    typedef prim::Aabb2D<size_t> TDirtyBox;
 
    void tonemap(const TRenderBuffer &source, const TWeightBuffer &weight, TRenderBuffer &dest, const TDirtyBox& box, TScalar gain);
    TScalar sceneGain() const;

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
    void doBeginRender();
    void doWriteRender(const OutputSample* first, const OutputSample* last);
    void doEndRender();
 
    TScalar maxSceneLuminance(const TRenderBuffer &scene, const TWeightBuffer &weight) const;

    static TToneMappingDictionary makeToneMappingDictionary();

    TRenderBuffer renderBuffer_;
    TWeightBuffer totalWeight_;
    TWeightBuffer alphaBuffer_;
    std::wstring path_;
    std::string options_;
    mutable util::CriticalSection renderLock_;
    util::CriticalSection saveLock_;
    TResolution2D resolution_;
    TRgbSpacePtr rgbSpace_;
    ToneMapping toneMapping_;
    mutable TScalar exposureStops_;
    TScalar exposureCorrectionStops_;
    TScalar middleGrey_;
    bool autoExposure_;
    mutable bool isDirtyAutoExposure_;
    bool isSaved_;
    
    static TToneMappingDictionary toneMappingDictionary_;
};



}

}

#endif

// EOF
