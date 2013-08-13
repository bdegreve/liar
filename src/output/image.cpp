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

#include "output_common.h"
#include "image.h"
#include "../kernel/image_codec.h"
#include <lass/io/file_attribute.h>

#define TONEMAP_SAMPLES

namespace liar
{
namespace output
{

PY_DECLARE_CLASS_DOC(Image, "simple image render target");
PY_CLASS_CONSTRUCTOR_2(Image, std::wstring, const TResolution2D&)
PY_CLASS_METHOD(Image, save)
PY_CLASS_MEMBER_RW(Image, path, setPath)
PY_CLASS_MEMBER_RW(Image, options, setOptions)
PY_CLASS_MEMBER_RW(Image, rgbSpace, setRgbSpace)
PY_CLASS_MEMBER_RW(Image, toneMapping, setToneMapping)
PY_CLASS_MEMBER_RW(Image, exposureStops, setExposureStops)
PY_CLASS_MEMBER_RW(Image, exposureCorrectionStops, setExposureCorrectionStops)
PY_CLASS_MEMBER_RW(Image, autoExposure, setAutoExposure)
PY_CLASS_MEMBER_RW(Image, middleGrey, setMiddleGrey);
PY_CLASS_STATIC_CONST(Image, "TM_LINEAR", "linear");
PY_CLASS_STATIC_CONST(Image, "TM_COMPRESS_Y", "compress_y");
PY_CLASS_STATIC_CONST(Image, "TM_COMPRESS_XYZ", "compress_xyz");
PY_CLASS_STATIC_CONST(Image, "TM_COMPRESS_RGB", "compress_rgb");
PY_CLASS_STATIC_CONST(Image, "TM_REINHARD2002_Y", "reinhard2002_y");
PY_CLASS_STATIC_CONST(Image, "TM_EXPONENTIAL_Y", "exponential_y");
PY_CLASS_STATIC_CONST(Image, "TM_EXPONENTIAL_XYZ", "exponential_xyz");
PY_CLASS_STATIC_CONST(Image, "TM_EXPONENTIAL_RGB", "exponential_rgb");
PY_CLASS_STATIC_CONST(Image, "TM_DUIKER_Y", "duiker_y");
PY_CLASS_STATIC_CONST(Image, "TM_DUIKER_XYZ", "duiker_xyz");
PY_CLASS_STATIC_CONST(Image, "TM_DUIKER_RGB", "duiker_rgb");
 
Image::TToneMappingDictionary Image::toneMappingDictionary_ = Image::makeToneMappingDictionary();
 
namespace
{
	inline float filmic(float x)
	{
		x = std::max(0.f, x - 0.004f);
		const float y = (x * (6.2f * x + .5f)) / (x * (6.2f * x + 1.7f) + 0.06f); 
		return num::pow(y, 2.2f); // undo gamma
	}
	inline float invFilmic(float y)
	{
		y = std::max(0.f, std::min(y, 0.99999f));
		y = num::pow(y, 1.f / 2.2f); // apply gamma
		// a*x*x + b*y + c == 0
		const float a = 6.2f * y - 6.2f;
		const float b = 1.7f * y - .5f;
		const float c = .06f * y;
		const float D = b * b - 4 * a * c;
		const float x = (-b - num::sqrt(D)) / (2 * a);
		return x + 0.004f;
	}
}



// --- public --------------------------------------------------------------------------------------

Image::Image(const std::wstring& path, const TResolution2D& resolution):
    path_(path),
    options_(""),
    resolution_(resolution),
    rgbSpace_(),
    toneMapping_(tmLinear),
    exposureStops_(0.f),
    exposureCorrectionStops_(0.f),
    middleGrey_(.184f),
    autoExposure_(false),
    isDirtyAutoExposure_(false),
    isSaved_(true)
{
}



Image::~Image()
{
    if (!isSaved_)
    {
        try
        {
            endRender();
        }
        catch(const std::exception& error)
        {
            std::cerr << "Failed to save Image: " << error.what() << "\n";
        }
        catch(...)
        {
            std::cerr << "Failed to save Image\n";
        }
    }
}



void Image::save()
{
    TRenderBuffer render;
    TWeightBuffer alpha, weight;
    TScalar gain = 1;

    LASS_LOCK(saveLock_)
    {
        LASS_LOCK(renderLock_)
        {
            render = renderBuffer_;
            alpha = alphaBuffer_;
            weight = totalWeight_;
            gain = sceneGain();
        }
        
        const TDirtyBox box( TDirtyBox::TPoint(0, 0), TDirtyBox::TPoint( resolution_.x - 1, resolution_.y - 1 ) );
        tonemap(render, weight, render, box, gain);
        ImageWriter writer(path_, resolution_, rgbSpace_, options_);
        writer.writeFull(&render[0], &alpha[0]);
        isSaved_ = true;
    }
}


const std::wstring& Image::path() const
{
    return path_;
}



const std::string& Image::options() const
{
    return options_;
}



const TRgbSpacePtr& Image::rgbSpace() const
{
    return rgbSpace_;
}



const std::string Image::toneMapping() const
{
    return toneMappingDictionary_.key(toneMapping_);
}



TScalar Image::exposureStops() const
{
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
        TScalar totalLogLuminance = 0;
        size_t coverage = 0;
        const TScalar minY = 1e-10f;
        for (size_t k = 0, n = renderBuffer_.size(); k < n; ++k)
        {
            const TScalar w = totalWeight_[k];
            if (w > 0)
            {
                const XYZ xyz = renderBuffer_[k];
                totalLogLuminance += num::log(std::max(xyz.y / w, minY));
                ++coverage;
            }
        }
        const TScalar sceneLuminance = num::exp(totalLogLuminance / coverage);
        TScalar autoGain = 1;
        if (sceneLuminance > 0)
        {
            switch (toneMapping_)
            {
            case tmLinear: // a = g * y
                autoGain = middleGrey_ / sceneLuminance;
                break;
            case tmCompressY: // a = g * y / (1 + g * y)  ->  g * y = a / (1 - a)
            case tmCompressRGB: 
            case tmReinhard2002Y:
            case tmReinhard2002RGB:
                autoGain = middleGrey_ / (sceneLuminance * (1 - middleGrey_));
                break;
            case tmExponentialY: // a = 1 - exp(-g * y)  ->  g * y = -ln(1 - a)
            case tmExponentialRGB:
                autoGain = -num::log1p(-middleGrey_) / sceneLuminance;
                break;
            case tmDuikerY:
            case tmDuikerRGB:
                autoGain = invFilmic(middleGrey_) / sceneLuminance;
                break;
            default:
                LASS_ENFORCE_UNREACHABLE;
            };
        }
        exposureStops_ = num::log2(autoGain);
        isDirtyAutoExposure_ = false;
    }
    return exposureStops_;
}



TScalar Image::exposureCorrectionStops() const
{
    return exposureCorrectionStops_;
}



bool Image::autoExposure() const
{
    return autoExposure_;
}



TScalar Image::middleGrey() const
{
    return middleGrey_;
}



void Image::setPath(const std::wstring& path)
{
    path_ = path;
}



void Image::setOptions(const std::string& options)
{
    options_ = options;
}



void Image::setRgbSpace(const TRgbSpacePtr& rgbSpace)
{
    rgbSpace_ = rgbSpace;
}



void Image::setToneMapping(const std::string& toneMapping)
{
    toneMapping_ = toneMappingDictionary_[stde::tolower(toneMapping)];
}



void Image::setExposureStops(TScalar stops)
{
    exposureStops_ = stops;
    autoExposure_ = false;
}



void Image::setExposureCorrectionStops(TScalar stops)
{
    exposureCorrectionStops_ = stops;
}



void Image::setAutoExposure(bool enable)
{
    autoExposure_ = enable;
}



void Image::setMiddleGrey(TScalar level)
{
    middleGrey_ = level;
}



// --- protected -----------------------------------------------------------------------------------

void Image::tonemap(const TRenderBuffer &source, const TWeightBuffer &weight, TRenderBuffer &dest, const TDirtyBox& box, TScalar gain)
{
    const TDirtyBox::TPoint min = box.min();
    const TDirtyBox::TPoint max = box.max();
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
                const TScalar w = weight[k];
                dest[k] = w > 0 ? source[k] * (gain / w) : 0;
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
                const TScalar w = weight[k];
                const XYZ linear = w > 0 ? source[k] * (gain / w) : 0;
                dest[k] = linear / (1 + linear.y);
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
                const TScalar w = weight[k];
                const prim::ColorRGBA linear = rgbSpace_->linearConvert(w > 0 ? source[k] * (gain / w) : XYZ(0));
                const prim::ColorRGBA tonemapped(linear.r / (1 + linear.r), linear.g / (1 + linear.g), linear.b / (1 + linear.b));
                dest[k] = rgbSpace_->linearConvert(tonemapped);
            }
        }
        break;

    case tmReinhard2002Y:
        {
            const TScalar Lw = gain * maxSceneLuminance(source, weight);
            const TScalar invLwSquared = num::inv(Lw * Lw);
            for (size_t j = min.y; j <= max.y; ++j)
            {
                const size_t kBegin = j * resolution_.x + min.x;
                const size_t kEnd = kBegin + nx;
                for (size_t k = kBegin; k < kEnd; ++k)
                {
                    const TScalar w = weight[k];
                    const XYZ linear = w > 0 ? source[k] * (gain / w) : 0;
                    const TScalar L = linear.y;
                    dest[k] = linear * ((1 + L * invLwSquared) / (1 + L));
                }
            }
        }
        break;

    case tmReinhard2002RGB:
        {
            const TScalar Lw = gain * maxSceneLuminance(source, weight);
            const TScalar invLwSquared = num::inv(Lw * Lw);
            for (size_t j = min.y; j <= max.y; ++j)
            {
                const size_t kBegin = j * resolution_.x + min.x;
                const size_t kEnd = kBegin + nx;
                for (size_t k = kBegin; k < kEnd; ++k)
                {
                    const TScalar w = weight[k];
                    const prim::ColorRGBA linear = rgbSpace_->linearConvert(w > 0 ? source[k] * (gain / w) : XYZ(0));
                    const prim::ColorRGBA tonemapped(
                        static_cast<prim::ColorRGBA::TValue>(linear.r * (1 + linear.r * invLwSquared) / (1 + linear.r)), 
                        static_cast<prim::ColorRGBA::TValue>(linear.g * (1 + linear.g * invLwSquared) / (1 + linear.g)), 
                        static_cast<prim::ColorRGBA::TValue>(linear.b * (1 + linear.b * invLwSquared) / (1 + linear.b)));
                    dest[k] = rgbSpace_->linearConvert(tonemapped);
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
                const TScalar w = weight[k];
                const XYZ linear = w > 0 ? source[k] * (gain / w) : 0;
                dest[k] = linear * (-num::expm1(-linear.y) / linear.y);
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
                const TScalar w = weight[k];
                const prim::ColorRGBA linear = rgbSpace_->linearConvert(w > 0 ? source[k] * (gain / w) : XYZ(0));
                const prim::ColorRGBA tonemapped(-num::expm1(-linear.r), -num::expm1(-linear.g), -num::expm1(-linear.b));
                dest[k] = rgbSpace_->linearConvert(tonemapped);
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
                const TScalar w = weight[k];
                const XYZ linear = w > 0 ? source[k] * (gain / w) : 0;
                dest[k] = linear * (filmic(linear.y) / std::max(linear.y, 0.004));
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
                const TScalar w = weight[k];
                const prim::ColorRGBA linear = rgbSpace_->linearConvert(w > 0 ? source[k] * (gain / w) : XYZ(0));
                const prim::ColorRGBA tonemapped(filmic(linear.r), filmic(linear.g), filmic(linear.b));
                dest[k] = rgbSpace_->linearConvert(tonemapped);
            }
        }
        break;

    default:
        LASS_ENFORCE_UNREACHABLE;
    }
}



TScalar Image::maxSceneLuminance(const TRenderBuffer &scene, const TWeightBuffer &weight) const
{
    LASS_ASSERT(scene.size() == weight.size());
    TScalar maxLuminance = 0;
    for (size_t k = 0, n = scene.size(); k < n; ++k)
    {
        TScalar w = weight[k];
        if (w > 0)
        {
            maxLuminance = std::max(scene[k].y / w, maxLuminance);
        }
    }
    return maxLuminance;
}



TScalar Image::sceneGain() const
{
    const TScalar stops = exposureStops() + exposureCorrectionStops();
    return num::pow(TScalar(2), stops);
}



// --- private -------------------------------------------------------------------------------------

const TResolution2D Image::doResolution() const
{
    return resolution_;
}



void Image::doBeginRender()
{
    LASS_LOCK(renderLock_)
    {
        const size_t n = resolution_.x * resolution_.y;
        renderBuffer_.assign(n, XYZ());
        totalWeight_.assign(n, 0);
        alphaBuffer_.assign(n, 0);
        isDirtyAutoExposure_ = true;
        isSaved_ = false;
    }
}



void Image::doWriteRender(const OutputSample* first, const OutputSample* last)
{
    LASS_ASSERT(resolution_.x > 0 && resolution_.y > 0);
    LASS_LOCK(renderLock_)
    {
        while (first != last)
        {
            const TPoint2D& position = first->screenCoordinate();
            const int i = static_cast<int>(num::floor(position.x * resolution_.x));
            const int j = static_cast<int>(num::floor(position.y * resolution_.y));
            if (i >= 0 && static_cast<size_t>(i) < resolution_.x && j >= 0 && static_cast<size_t>(j) < resolution_.y)
            {
                const size_t k = j * resolution_.x + i;
                TScalar alpha = first->weight() * first->alpha();
                renderBuffer_[k] += first->radiance() * alpha;
                alphaBuffer_[k] += alpha;
                totalWeight_[k] += first->weight();
            }
            ++first;
        }
        isDirtyAutoExposure_ = true;
        isSaved_ = false;
    }
}



void Image::doEndRender()
{
    if (!isSaved_)
    {
        save();
    }
}

 
 
Image::TToneMappingDictionary Image::makeToneMappingDictionary()
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
