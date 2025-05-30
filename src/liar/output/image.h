/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2024  Bram de Greve (bramz@users.sourceforge.net)
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
#include "raster.h"
#include <filesystem>

namespace liar
{
namespace output
{

class LIAR_OUTPUT_DLL Image: public Raster
{
    PY_HEADER(Raster)
public:

    Image(const std::filesystem::path& filename, const TResolution2D& resolution);
    ~Image();

    void save();

    const std::filesystem::path& path() const;
    const std::string& options() const;

    void setPath(const std::filesystem::path& path);
    void setOptions(const std::string& options);

private:

    void doBeginRender() override;
    void doEndRender() override;

    std::filesystem::path path_;
    std::string options_;
    util::CriticalSection saveLock_;
};



}

}

#endif

// EOF
