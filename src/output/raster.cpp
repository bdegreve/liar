/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2021  Bram de Greve (bramz@users.sourceforge.net)
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
#include "raster.h"

#define TONEMAP_SAMPLES

namespace liar
{
namespace output
{

PY_DECLARE_CLASS_DOC(Raster, "rasterized render target");
PY_CLASS_MEMBER_RW(Raster, rgbSpace, setRgbSpace)
PY_CLASS_MEMBER_RW(Raster, toneMapping, setToneMapping)
PY_CLASS_MEMBER_RW(Raster, exposureStops, setExposureStops)
PY_CLASS_MEMBER_RW(Raster, exposureCorrectionStops, setExposureCorrectionStops)
PY_CLASS_MEMBER_RW(Raster, autoExposure, setAutoExposure)
PY_CLASS_MEMBER_RW(Raster, middleGrey, setMiddleGrey);
PY_CLASS_STATIC_CONST(Raster, "TM_LINEAR", "linear");
PY_CLASS_STATIC_CONST(Raster, "TM_COMPRESS_Y", "compress_y");
PY_CLASS_STATIC_CONST(Raster, "TM_COMPRESS_XYZ", "compress_xyz");
PY_CLASS_STATIC_CONST(Raster, "TM_COMPRESS_RGB", "compress_rgb");
PY_CLASS_STATIC_CONST(Raster, "TM_REINHARD2002_Y", "reinhard2002_y");
PY_CLASS_STATIC_CONST(Raster, "TM_EXPONENTIAL_Y", "exponential_y");
PY_CLASS_STATIC_CONST(Raster, "TM_EXPONENTIAL_XYZ", "exponential_xyz");
PY_CLASS_STATIC_CONST(Raster, "TM_EXPONENTIAL_RGB", "exponential_rgb");
PY_CLASS_STATIC_CONST(Raster, "TM_DUIKER_Y", "duiker_y");
PY_CLASS_STATIC_CONST(Raster, "TM_DUIKER_XYZ", "duiker_xyz");
PY_CLASS_STATIC_CONST(Raster, "TM_DUIKER_RGB", "duiker_rgb");
 
Raster::TToneMappingDictionary Raster::toneMappingDictionary_ = Raster::makeToneMappingDictionary();

namespace
{
    typedef Raster::TValue TValue;

    inline TValue filmic(TValue x)
    {
        x = std::max(0.f, x - 0.004f);
        const TValue y = (x * (6.2f * x + .5f)) / (x * (6.2f * x + 1.7f) + 0.06f);
        return num::pow(y, 2.2f); // undo gamma
    }
    inline TValue invFilmic(float y)
    {
        y = std::max(0.f, std::min(y, 0.99999f));
        y = num::pow(y, 1.f / 2.2f); // apply gamma
        // a*x*x + b*y + c == 0
        const TValue a = 6.2f * y - 6.2f;
        const TValue b = 1.7f * y - .5f;
        const TValue c = .06f * y;
        const TValue D = b * b - 4 * a * c;
        const TValue x = (-b - num::sqrt(D)) / (2 * a);
        return x + 0.004f;
    }
}


// --- public --------------------------------------------------------------------------------------

const TRgbSpacePtr& Raster::rgbSpace() const
{
    return rgbSpace_;
}



const std::string Raster::toneMapping() const
{
    return toneMappingDictionary_.key(toneMapping_);
}



Raster::TValue Raster::exposureStops() const
{
    const TValue stopDivisions = 3.f; // in how many discrete steps is one stop divided?

    if (!autoExposure_)
    {
        return exposureStops_;
    }
    LASS_LOCK(renderLock_)
    {
        if (!isDirtyAutoExposure_)
        {
            return exposureStops_;
        }
        TValue autoGain = 1;
        const TValue y = averageSceneLuminance();
        if (y > 0)
        {
            switch (toneMapping_)
            {
            case tmLinear: // a = g * y
                autoGain = middleGrey_ / y;
                break;
            case tmCompressY: // a = g * y / (1 + g * y)  ->  g * y = a / (1 - a)
            case tmCompressRGB: 
            case tmReinhard2002Y:
            case tmReinhard2002RGB:
                autoGain = middleGrey_ / (y * (1 - middleGrey_));
                break;
            case tmExponentialY: // a = 1 - exp(-g * y)  ->  g * y = -ln(1 - a)
            case tmExponentialRGB:
                autoGain = -num::log1p(-middleGrey_) / y;
                break;
            case tmDuikerY:
            case tmDuikerRGB:
                autoGain = invFilmic(middleGrey_) / y;
                break;
            default:
                LASS_ENFORCE_UNREACHABLE;
            };
        }
        const TValue stops = num::log2(autoGain);
        const TValue exposureStops = num::round(stopDivisions * stops) / stopDivisions; // round to nearest discrete step.
        if (exposureStops != exposureStops_)
        {
            exposureStops_ = exposureStops;
            renderDirtyBox_ += allTimeDirtyBox_;
        }
        isDirtyAutoExposure_ = false;
    }
    return exposureStops_;
}



Raster::TValue Raster::exposureCorrectionStops() const
{
    return exposureCorrectionStops_;
}



bool Raster::autoExposure() const
{
    return autoExposure_;
}



Raster::TValue Raster::middleGrey() const
{
    return middleGrey_;
}



void Raster::setRgbSpace(const TRgbSpacePtr& rgbSpace)
{
    LASS_LOCK(renderLock_)
    {
        rgbSpace_ = rgbSpace;
        renderDirtyBox_ += allTimeDirtyBox_;
    }
}



void Raster::setToneMapping(const std::string& toneMapping)
{
    LASS_LOCK(renderLock_)
    {
        const ToneMapping t = toneMappingDictionary_[stde::tolower(toneMapping)];
        if (toneMapping_ == t)
        {
            return;
        }
        toneMapping_ = t;
        renderDirtyBox_ += allTimeDirtyBox_;
    }
}



void Raster::setExposureStops(TValue stops)
{
    LASS_LOCK(renderLock_)
    {
        autoExposure_ = false;
        if (exposureStops_ == stops)
        {
            return;
        }
        exposureStops_ = stops;
        renderDirtyBox_ += allTimeDirtyBox_;
    }
}



void Raster::setExposureCorrectionStops(TValue stops)
{
    LASS_LOCK(renderLock_)
    {
        if (exposureCorrectionStops_ == stops)
        {
            return;
        }
        exposureCorrectionStops_ = stops;
        renderDirtyBox_ += allTimeDirtyBox_;
    }
}



void Raster::setAutoExposure(bool enable)
{
    LASS_LOCK(renderLock_)
    {
        if (autoExposure_ == enable)
        {
            return;
        }
        autoExposure_ = enable;
        renderDirtyBox_ += allTimeDirtyBox_;
    }
}



void Raster::setMiddleGrey(TValue level)
{
    LASS_LOCK(renderLock_)
    {
        if (middleGrey_ == level)
        {
            return;
        }
        middleGrey_ = level;
        renderDirtyBox_ += allTimeDirtyBox_;
    }
}



void Raster::nextToneMapping()
{
    LASS_LOCK(renderLock_)
    {
        toneMapping_ = static_cast<ToneMapping>((toneMapping_ + 1) % numToneMapping);
        renderDirtyBox_ += allTimeDirtyBox_;
    }
}



// --- protected -----------------------------------------------------------------------------------

Raster::Raster(const TResolution2D& resolution):
    resolution_(resolution),
    rgbSpace_(RgbSpace::defaultSpace()),
    toneMapping_(tmLinear),
    exposureStops_(0.f),
    exposureCorrectionStops_(0.f),
    middleGrey_(.184f),
    maxSceneLuminance_(0),
    autoExposure_(true),
    isDirtyAutoExposure_(false)
{
}



void Raster::beginRaster()
{
    LASS_LOCK(renderLock_)
    {
        const size_t n = resolution_.x * resolution_.y;
        renderBuffer_.assign(n, XYZ());
        tonemapBuffer_.assign(n, prim::ColorRGBA(0,0,0,0));
        totalWeight_.assign(n, 0);
        alphaBuffer_.assign(n, 0);
        renderDirtyBox_.clear();
        allTimeDirtyBox_.clear();
        maxSceneLuminance_ = 0;
        isDirtyAutoExposure_ = true;
    }
}



Raster::TDirtyBox Raster::tonemap(const TRgbSpacePtr& destSpace)
{
    // do not automatically use rgbSpace_ as destSpace, as some image writers may use a slightly different space,
    // so destSpace should be provided by the actual output device

    LASS_LOCK(renderLock_)
    {
        const TValue gain = sceneGain(); // get gain first, as it can change the dirtybox

        const TDirtyBox::TPoint min = renderDirtyBox_.min();
        const TDirtyBox::TPoint max = renderDirtyBox_.max();
        const size_t nx = max.x - min.x + 1;

        switch (toneMapping_)
        {
        case tmLinear:
            for (size_t j = min.y; j <= max.y; ++j)
            {
                const size_t kBegin = j * resolution_.x + min.x;
                const size_t kEnd = kBegin + nx;
                for (size_t k = kBegin; k < kEnd; ++k)
                {
                    const XYZ linear = weighted(k, gain);
                    tonemapBuffer_[k] = destSpace->convert(linear, alphaBuffer_[k]);
                }
            }
            break;

        case tmCompressY:
            for (size_t j = min.y; j <= max.y; ++j)
            {
                const size_t kBegin = j * resolution_.x + min.x;
                const size_t kEnd = kBegin + nx;
                for (size_t k = kBegin; k < kEnd; ++k)
                {
                    const XYZ linear = weighted(k, gain);
                    const XYZ tonemapped = linear / (1 + linear.y);
                    tonemapBuffer_[k] = destSpace->convert(tonemapped, alphaBuffer_[k]);
                }
            }
            break;

        case tmCompressRGB:
            for (size_t j = min.y; j <= max.y; ++j)
            {
                const size_t kBegin = j * resolution_.x + min.x;
                const size_t kEnd = kBegin + nx;
                for (size_t k = kBegin; k < kEnd; ++k)
                {
                    const prim::ColorRGBA linear = destSpace->linearConvert(weighted(k, gain), alphaBuffer_[k]);
                    const prim::ColorRGBA tonemapped(linear.r / (1 + linear.r), linear.g / (1 + linear.g), linear.b / (1 + linear.b), linear.a);
                    tonemapBuffer_[k] = destSpace->toGamma(tonemapped);
                }
            }
            break;

        case tmReinhard2002Y:
            {
                const TValue Lw = gain * maxSceneLuminance_;
                const TValue invLwSquared = num::inv(Lw * Lw);
                for (size_t j = min.y; j <= max.y; ++j)
                {
                    const size_t kBegin = j * resolution_.x + min.x;
                    const size_t kEnd = kBegin + nx;
                    for (size_t k = kBegin; k < kEnd; ++k)
                    {
                        const XYZ linear = weighted(k, gain);
                        const TValue L = linear.y;
                        const XYZ tonemapped = linear * ((1 + L * invLwSquared) / (1 + L));
                        tonemapBuffer_[k] = destSpace->convert(tonemapped, alphaBuffer_[k]);
                    }
                }
            }
            break;

        case tmReinhard2002RGB:
            {
                const TValue Lw = gain * maxSceneLuminance_;
                const TValue invLwSquared = num::inv(Lw * Lw);
                for (size_t j = min.y; j <= max.y; ++j)
                {
                    const size_t kBegin = j * resolution_.x + min.x;
                    const size_t kEnd = kBegin + nx;
                    for (size_t k = kBegin; k < kEnd; ++k)
                    {
                        const prim::ColorRGBA linear = destSpace->linearConvert(weighted(k, gain), alphaBuffer_[k]);
                        const prim::ColorRGBA tonemapped(
                            static_cast<prim::ColorRGBA::TValue>(linear.r * (1 + linear.r * invLwSquared) / (1 + linear.r)), 
                            static_cast<prim::ColorRGBA::TValue>(linear.g * (1 + linear.g * invLwSquared) / (1 + linear.g)), 
                            static_cast<prim::ColorRGBA::TValue>(linear.b * (1 + linear.b * invLwSquared) / (1 + linear.b)),
                            linear.a);
                        tonemapBuffer_[k] = destSpace->toGamma(tonemapped);
                    }
                }
            }
            break;

        case tmExponentialY:
            for (size_t j = min.y; j <= max.y; ++j)
            {
                const size_t kBegin = j * resolution_.x + min.x;
                const size_t kEnd = kBegin + nx;
                for (size_t k = kBegin; k < kEnd; ++k)
                {
                    const XYZ linear = weighted(k, gain);
                    const XYZ::TValue scale = linear.y < 1e-8
                        ? 1 - linear.y / 2
                        : (-num::expm1(-linear.y) / linear.y);
                    tonemapBuffer_[k] = destSpace->convert(linear * scale, alphaBuffer_[k]);
                }
            }
            break;

        case tmExponentialRGB:
            for (size_t j = min.y; j <= max.y; ++j)
            {
                const size_t kBegin = j * resolution_.x + min.x;
                const size_t kEnd = kBegin + nx;
                for (size_t k = kBegin; k < kEnd; ++k)
                {
                    const prim::ColorRGBA linear = destSpace->linearConvert(weighted(k, gain), alphaBuffer_[k]);
                    const prim::ColorRGBA tonemapped(-num::expm1(-linear.r), -num::expm1(-linear.g), -num::expm1(-linear.b), linear.a);
                    tonemapBuffer_[k] = destSpace->toGamma(tonemapped);
                }
            }
            break;

        case tmDuikerY:
            for (size_t j = min.y; j <= max.y; ++j)
            {
                const size_t kBegin = j * resolution_.x + min.x;
                const size_t kEnd = kBegin + nx;
                for (size_t k = kBegin; k < kEnd; ++k)
                {
                    const XYZ linear = weighted(k, gain);
                    const XYZ tonemapped = linear * (filmic(linear.y) / std::max(linear.y, static_cast<XYZ::TValue>(0.004)));
                    tonemapBuffer_[k] = destSpace->convert(tonemapped, alphaBuffer_[k]);
                }
            }
            break;

        case tmDuikerRGB:
            for (size_t j = min.y; j <= max.y; ++j)
            {
                const size_t kBegin = j * resolution_.x + min.x;
                const size_t kEnd = kBegin + nx;
                for (size_t k = kBegin; k < kEnd; ++k)
                {
                    const prim::ColorRGBA linear = destSpace->linearConvert(weighted(k, gain), alphaBuffer_[k]);
                    const prim::ColorRGBA tonemapped(filmic(linear.r), filmic(linear.g), filmic(linear.b), linear.a);
                    tonemapBuffer_[k] = destSpace->toGamma(tonemapped);
                }
            }
            break;

        default:
            LASS_ENFORCE_UNREACHABLE;
        }
    }

    TDirtyBox box = renderDirtyBox_;
    renderDirtyBox_.clear();
    return box;
}



const Raster::TTonemapBuffer& Raster::tonemapBuffer() const
{
    return tonemapBuffer_;
}



const Raster::TValueBuffer& Raster::totalWeight() const
{
    return totalWeight_;
}



const Raster::TDirtyBox& Raster::dirtyBox() const
{
    return renderDirtyBox_;
}



void Raster::clearDirtyBox()
{
    renderDirtyBox_ = TDirtyBox();
}



util::CriticalSection& Raster::renderLock() const
{
    return renderLock_;
}



// --- private -------------------------------------------------------------------------------------

const TResolution2D Raster::doResolution() const
{
    return resolution_;
}



void Raster::doWriteRender(const OutputSample* first, const OutputSample* last)
{
    LASS_ASSERT(resolution_.x > 0 && resolution_.y > 0);
    LASS_LOCK(renderLock_)
    {
        while (first != last)
        {
            const TPoint2D& position = first->screenCoordinate();
            if (position.x >= 0 && position.y >= 0)
            {
                const size_t i = static_cast<size_t>(num::floor(position.x * static_cast<TScalar>(resolution_.x)));
                const size_t j = static_cast<size_t>(num::floor(position.y * static_cast<TScalar>(resolution_.y)));
                if (i < resolution_.x && j < resolution_.y)
                {
                    const size_t k = j * resolution_.x + i;
                    TValue& w = totalWeight_[k];
                    w += first->weight();
                    const TValue alpha = first->weight() * first->alpha();
                    alphaBuffer_[k] += alpha;
                    XYZ& xyz = renderBuffer_[k];
                    xyz += first->radiance() * alpha;
    
                    renderDirtyBox_ += TDirtyBox::TPoint(i, j);
                    if (w > 0 && xyz.y > 0)
                    {
                        maxSceneLuminance_ = std::max(xyz.y / w, maxSceneLuminance_);
                    }
                }
            }
            ++first;
        }
        allTimeDirtyBox_ += renderDirtyBox_;
        isDirtyAutoExposure_ = true;
    }
}



Raster::TValue Raster::sceneGain() const
{
    const TValue stops = exposureStops() + exposureCorrectionStops();
    return num::pow(TValue(2), stops);
}



Raster::TValue Raster::averageSceneLuminance() const
{
    TValue sumLogY = 0;
    size_t coverage = 0;
    LASS_LOCK(renderLock_)
    {
        for (size_t k = 0, n = renderBuffer_.size(); k < n; ++k)
        {
            const TValue w = totalWeight_[k];
            const TValue y = renderBuffer_[k].y;
            if (w > 0 && y > 0)
            {
                sumLogY += num::log(y / w);
                ++coverage;
            }
        }
    }
    if (!coverage)
    {
        return 0;
    }
    const TValue avgLogY = sumLogY / static_cast<TValue>(coverage);
    return num::exp(avgLogY);
}



inline XYZ Raster::weighted(size_t index, TValue gain) const
{
    const TValue w = totalWeight_[index];
    return w > 0 ? renderBuffer_[index] * (gain / w) : XYZ(0);
}



Raster::TToneMappingDictionary Raster::makeToneMappingDictionary()
{
    TToneMappingDictionary result;
    result.enableSuggestions();
    result.add("linear", tmLinear);
    result.add("compress_y", tmCompressY);
    result.add("compress_rgb", tmCompressRGB);
    result.add("reinhard2002_y", tmReinhard2002Y);
    result.add("reinhard2002_rgb", tmReinhard2002RGB);
    result.add("exponential_y", tmExponentialY);
    result.add("exponential_rgb", tmExponentialRGB);
    result.add("duiker_y", tmDuikerY);
    result.add("duiker_rgb", tmDuikerRGB);
    return result;
}


// --- free ----------------------------------------------------------------------------------------


}

}

// EOF
