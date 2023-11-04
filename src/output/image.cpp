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

#include "output_common.h"
#include "image.h"
#include "../kernel/image_codec.h"
#include <lass/io/file_attribute.h>
#include <lass/python/export_traits_filesystem.h>

#define TONEMAP_SAMPLES

namespace liar
{
namespace output
{

PY_DECLARE_CLASS_DOC(Image, "simple image render target");
PY_CLASS_CONSTRUCTOR_2(Image, std::filesystem::path, const TResolution2D&)
PY_CLASS_METHOD(Image, save)
PY_CLASS_MEMBER_RW(Image, path, setPath)
PY_CLASS_MEMBER_RW(Image, options, setOptions)

// --- public --------------------------------------------------------------------------------------

Image::Image(const std::filesystem::path& path, const TResolution2D& resolution):
    Raster(resolution),
    path_(path),
    options_("")
{
}



Image::~Image()
{
    if (!dirtyBox().isEmpty())
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
    LASS_LOCK(saveLock_)
    {
        const TResolution2D& res = resolution();
        ImageWriter writer(path_, res, rgbSpace(), options_);
        tonemap(writer.rgbSpace());
        writer.writeFull(&tonemapBuffer()[0]);
    }
}


const std::filesystem::path& Image::path() const
{
    return path_;
}



const std::string& Image::options() const
{
    return options_;
}



void Image::setPath(const std::filesystem::path& path)
{
    path_ = path;
}



void Image::setOptions(const std::string& options)
{
    options_ = options;
}


// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void Image::doBeginRender()
{
	beginRaster();
}


void Image::doEndRender()
{
    if (!dirtyBox().isEmpty())
    {
        save();
    }
}



// --- free ----------------------------------------------------------------------------------------


}

}

// EOF
