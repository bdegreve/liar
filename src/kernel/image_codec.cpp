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

#include "kernel_common.h"
#include "image_codec.h"
#include <lass/io/file_attribute.h>
#include <lass/io/image.h>
#include <lass/util/singleton.h>
#include <lass/stde/extended_string.h>

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS_DOC(ImageCodec, "Abstract base class of image codecs")

PY_DECLARE_CLASS_DOC(ImageCodecLass, "Lass based image codec")
PY_CLASS_CONSTRUCTOR_0(ImageCodecLass)
LASS_EXECUTE_BEFORE_MAIN_EX(ImageCodecLass,
	TImageCodecMap& map = imageCodecs();
	map[L"tga"] = map[L"targa"] = TImageCodecPtr(new ImageCodecLass);
	map[L"lass"] = map[L"hdr"] = map[L"pic"] = map[L"rgbe"] = map[L"pfm"] = TImageCodecPtr(new ImageCodecLass(false));
	map[L"igi"] = TImageCodecPtr(new ImageCodecLass(false, CIEXYZ));
)

// --- ImageCodec ----------------------------------------------------------------------------------

ImageCodec::~ImageCodec()
{
}



TImageCodecMap& imageCodecs()
{
	return *util::Singleton<TImageCodecMap>::instance();
}



const TImageCodecPtr& imageCodec(const std::wstring& extension)
{
	const TImageCodecMap& codecs = imageCodecs();
	const TImageCodecMap::const_iterator candidate = codecs.find(stde::tolower(extension));
	if (candidate == codecs.end())
	{
		LASS_THROW("No image codec registered for the extension '" << util::wcharToUtf8(extension) << "'");
	}
	return candidate->second;
}


void transcodeImage(const std::wstring& source, const std::wstring& dest, TRgbSpacePtr sourceSpace, const TRgbSpacePtr& destSpace)
{
	ImageReader reader(source, sourceSpace);
	sourceSpace = reader.rgbSpace(); // use actual source space
	const size_t nx = reader.resolution().x;
	const size_t ny = reader.resolution().y;
	if (nx == 0 || ny == 0)
	{
		return;
	}
	ImageWriter writer(dest, reader.resolution(), destSpace ? destSpace : sourceSpace);
	std::vector<prim::ColorRGBA> line(nx);
	for (size_t i = 0; i < ny; ++i)
	{
		reader.readLine(&line[0]);
		if (destSpace)
		{
			for (size_t k = 0; k < nx; ++k)
			{
				TScalar alpha;
				const XYZ xyz = sourceSpace->convert(line[k], alpha);
				line[k] = destSpace->convert(xyz, alpha);
			}
		}
		writer.writeLine(&line[0]);
	}
}


// --- ImageReader ---------------------------------------------------------------------------------

ImageReader::ImageReader(
		const std::wstring& path, const TRgbSpacePtr& rgbSpace, const std::string& options):
	codec_(imageCodec(io::fileExtension(path))),
	handle_(0)
{
	handle_ = codec_->open(path, rgbSpace, options);
}

ImageReader::~ImageReader()
{
	codec_->close(handle_);
	handle_ = 0;
}

void ImageReader::readFull(prim::ColorRGBA* out) const
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
		const std::wstring& path, const TResolution2D& resolution, const TRgbSpacePtr& rgbSpace, 
		const std::string& options):
	codec_(imageCodec(io::fileExtension(path))),
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
		std::wstring path;
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



ImageCodec::TImageHandle ImageCodecLass::doCreate(const std::wstring& path, const TResolution2D& resolution, const TRgbSpacePtr& rgbSpace, const std::string&) const
{
	typedef io::Image::TChromaticity TChromaticity;

	std::auto_ptr<impl::LassImage> pimpl(new impl::LassImage);
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



ImageCodec::TImageHandle ImageCodecLass::doOpen(const std::wstring& path, const TRgbSpacePtr& rgbSpace, const std::string&) const
{
	std::auto_ptr<impl::LassImage> pimpl(new impl::LassImage);
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



void ImageCodecLass::doReadLine(TImageHandle handle, prim::ColorRGBA* out) const
{
	impl::LassImage* pimpl = static_cast<impl::LassImage*>(handle);
	const io::Image& image = pimpl->image;
	LASS_ENFORCE(pimpl->y < image.rows());
	for (size_t x = 0; x < image.cols(); ++x)
	{
		*out++ = image(pimpl->y, x);
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
