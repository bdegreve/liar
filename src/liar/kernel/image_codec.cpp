/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2025  Bram de Greve (bramz@users.sourceforge.net)
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

#include "kernel_common.h"
#include "image_codec.h"
#include <lass/io/file_attribute.h>
#include <lass/io/image.h>
#include <lass/util/singleton.h>
#include <lass/stde/extended_string.h>
#include <lass/python/export_traits_filesystem.h>

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(ImageCodec, "Abstract base class of image codecs")

PY_DECLARE_CLASS_DOC(ImageCodecLass, "Lass based image codec")
PY_CLASS_CONSTRUCTOR_0(ImageCodecLass)
LASS_EXECUTE_BEFORE_MAIN_EX(ImageCodecLass,
	TImageCodecMap& map = imageCodecs();
	map[".tga"] = map[".targa"] = TImageCodecRef(new ImageCodecLass);
	map[".lass"] = map[".hdr"] = map[".pic"] = map[".rgbe"] = map[".pfm"] = TImageCodecRef(new ImageCodecLass(false));
	map[".igi"] = TImageCodecRef(new ImageCodecLass(false, CIEXYZ));
)

// --- ImageCodec ----------------------------------------------------------------------------------

ImageCodec::~ImageCodec()
{
}



TImageCodecMap& imageCodecs()
{
	return *util::Singleton<TImageCodecMap>::instance();
}



const TImageCodecRef& imageCodec(const std::string& extension)
{
	const TImageCodecMap& codecs = imageCodecs();
	const TImageCodecMap::const_iterator candidate = codecs.find(stde::tolower(extension));
	if (candidate == codecs.end())
	{
		LASS_THROW_EX(ImageCodecError, "No image codec registered for the extension '" << extension << "'");
	}
	return candidate->second;
}


void transcodeImage(const std::filesystem::path& source, const std::filesystem::path& dest, const TRgbSpacePtr& sourceSpace, TRgbSpacePtr destSpace)
{
	ImageReader reader(source, sourceSpace);
	const size_t nx = reader.resolution().x;
	const size_t ny = reader.resolution().y;
	if (nx == 0 || ny == 0)
	{
		return;
	}
	if (!destSpace)
	{
		destSpace = reader.rgbSpace(); // use actual input space
	}
	ImageWriter writer(dest, reader.resolution(), destSpace ? destSpace : sourceSpace);
	std::vector<XYZA> src(nx);
	std::vector<prim::ColorRGBA> dst(nx);
	for (size_t i = 0; i < ny; ++i)
	{
		reader.readLine(&src[0]);
		for (size_t k = 0; k < nx; ++k)
		{
			dst[k] = destSpace->toRGBA(src[k]);
		}
		writer.writeLine(&dst[0]);
	}
}


// --- ImageReader ---------------------------------------------------------------------------------

ImageReader::ImageReader(
		const std::filesystem::path& path, const TRgbSpacePtr& rgbSpace, const std::string& options):
	codec_(imageCodec(path.extension().string())),
	handle_(0)
{
	handle_ = codec_->open(path, rgbSpace, options);
}

ImageReader::~ImageReader()
{
	codec_->close(handle_);
	handle_ = 0;
}

void ImageReader::readFull(XYZA* out) const
{
	const TResolution2D res = resolution();
	const size_t size = res.x * res.y;
	for (size_t k = 0; k < size; k += res.x)
	{
		readLine(out + k);
	}
}



// --- ImageWriter ---------------------------------------------------------------------------------

ImageWriter::ImageWriter(
		const std::filesystem::path& path, const TResolution2D& resolution, const TRgbSpacePtr& rgbSpace,
		const std::string& options):
	codec_(imageCodec(path.extension().string())),
	handle_(0)
{
	handle_ = codec_->create(path, resolution, rgbSpace, options);
}

ImageWriter::~ImageWriter()
{
	codec_->close(handle_);
	handle_ = 0;
}

void ImageWriter::writeFull(const prim::ColorRGBA* in) const
{
	const TResolution2D res = resolution();
	const size_t size = res.x * res.y;
	for (size_t k = 0; k < size; k += res.x)
	{
		writeLine(in + k);
	}
}



// --- ImageCodecLass ------------------------------------------------------------------------

namespace impl
{
	struct LassImage
	{
		io::Image image;
		TRgbSpacePtr rgbSpace;
		std::filesystem::path path;
		size_t y = 0;
		bool saveOnClose = false;
	};
}



ImageCodecLass::ImageCodecLass(bool hasGammaCorrection, const TRgbSpacePtr& defaultCodecSpace):
	defaultCodecSpace_(defaultCodecSpace),
	hasGammaCorrection_(hasGammaCorrection)
{
}



TRgbSpacePtr ImageCodecLass::selectRgbSpace(const TRgbSpacePtr& customSpace) const
{
	if (customSpace)
	{
		return customSpace;
	}
	if (defaultCodecSpace_)
	{
		return defaultCodecSpace_;
	}
	return RgbSpace::defaultSpace();
}



ImageCodec::TImageHandle ImageCodecLass::doCreate(const std::filesystem::path& path, const TResolution2D& resolution, const TRgbSpacePtr& rgbSpace, const std::string&) const
{
	typedef io::Image::TChromaticity TChromaticity;

	std::unique_ptr<impl::LassImage> pimpl(new impl::LassImage);
	pimpl->image.reset(resolution.y, resolution.x);
	pimpl->rgbSpace = selectRgbSpace(rgbSpace);
	if (!hasGammaCorrection_)
	{
		pimpl->rgbSpace = pimpl->rgbSpace->linearSpace();
	}
	pimpl->path = path;
	pimpl->saveOnClose = true;
	RgbSpace& space = *pimpl->rgbSpace;
	LASS_COUT << "RGB create: " << space.red() << " " << space.green() << " " << space.blue() << " " << space.white() << "\n";
	io::Image::ColorSpace& colorSpace = pimpl->image.colorSpace();
	colorSpace.red = TChromaticity(space.red());
	colorSpace.green = TChromaticity(space.green());
	colorSpace.blue = TChromaticity(space.blue());
	colorSpace.white = TChromaticity(space.white());
	colorSpace.gamma = static_cast<num::Tfloat32>(space.gamma());
	return pimpl.release();
}



ImageCodec::TImageHandle ImageCodecLass::doOpen(const std::filesystem::path& path, const TRgbSpacePtr& rgbSpace, const std::string&) const
{
	std::unique_ptr<impl::LassImage> pimpl(new impl::LassImage);
	pimpl->image.open(path);
	pimpl->rgbSpace = rgbSpace;
	if (!pimpl->rgbSpace)
	{
		io::Image::ColorSpace& colorSpace = pimpl->image.colorSpace();
		pimpl->rgbSpace.reset(new RgbSpace(
			TPoint2D(colorSpace.red),
			TPoint2D(colorSpace.green),
			TPoint2D(colorSpace.blue),
			TPoint2D(colorSpace.white),
			colorSpace.gamma));
	}
	if (!hasGammaCorrection_)
	{
		pimpl->rgbSpace = pimpl->rgbSpace->linearSpace();
	}
	RgbSpace& space = *pimpl->rgbSpace;
	LASS_COUT << "RGB open: " << space.red() << " " << space.green() << " " << space.blue() << " " << space.white() << "\n";
	pimpl->path = path;
	pimpl->saveOnClose = false;
	return pimpl.release();
}



void ImageCodecLass::doClose(TImageHandle handle) const
{
	impl::LassImage* impl = static_cast<impl::LassImage*>(handle);
	if (impl->saveOnClose)
	{
		impl->image.save(impl->path);
	}
	delete impl;
}



const TResolution2D ImageCodecLass::doResolution(TImageHandle handle) const
{
	impl::LassImage* impl = static_cast<impl::LassImage*>(handle);
	return TResolution2D(impl->image.cols(), impl->image.rows());
}



const TRgbSpacePtr ImageCodecLass::doRgbSpace(TImageHandle handle) const
{
	return static_cast<impl::LassImage*>(handle)->rgbSpace;
}



void ImageCodecLass::doReadLine(TImageHandle handle, XYZA* out) const
{
	impl::LassImage* pimpl = static_cast<impl::LassImage*>(handle);
	const io::Image& image = pimpl->image;
	const auto& rgbSpace = *pimpl->rgbSpace;
	LASS_ENFORCE(pimpl->y < image.rows());
	for (size_t x = 0; x < image.cols(); ++x)
	{
		*out++ = rgbSpace.toXYZA(image(pimpl->y, x));
	}
	++pimpl->y;
}



void ImageCodecLass::doWriteLine(
		TImageHandle handle, const prim::ColorRGBA* in) const
{
	impl::LassImage* pimpl = static_cast<impl::LassImage*>(handle);
	io::Image& image = pimpl->image;
	LASS_ENFORCE(pimpl->y < image.rows());
	for (size_t x = 0; x < image.cols(); ++x)
	{
		image(pimpl->y, x) = *in++;
	}
	++pimpl->y;
}

}
}

// EOF
