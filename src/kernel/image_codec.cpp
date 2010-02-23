/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
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
	map["tga"] = map["targa"] = TImageCodecPtr(new ImageCodecLass);
	map["lass"] = map["hdr"] = map["pic"] = map["rgbe"] = TImageCodecPtr(new ImageCodecLass(false));
	map["igi"] = TImageCodecPtr(new ImageCodecLass(false, CIEXYZ));
)

// --- ImageCodec ----------------------------------------------------------------------------------

ImageCodec::~ImageCodec()
{
}



TImageCodecMap& imageCodecs()
{
	return *util::Singleton<TImageCodecMap>::instance();
}



const TImageCodecPtr& imageCodec(const std::string& extension)
{
	const TImageCodecMap& codecs = imageCodecs();
	const TImageCodecMap::const_iterator candidate = codecs.find(stde::tolower(extension));
	if (candidate == codecs.end())
	{
		LASS_THROW("No image codec registered for the extension '" << extension << "'");
	}
	return candidate->second;
}


void transcodeImage(const std::string& source, const std::string& dest, const TRgbSpacePtr& sourceSpace, const TRgbSpacePtr& destSpace)
{
	ImageReader reader(source, sourceSpace);
	const size_t nx = reader.resolution().x;
	const size_t ny = reader.resolution().y;
	if (nx == 0 || ny == 0)
	{
		return;
	}
	ImageWriter writer(dest, reader.resolution(), destSpace ? destSpace : reader.rgbSpace());
	std::vector<XYZ> xyz(nx);
	std::vector<TScalar> alpha(nx);
	for (size_t i = 0; i < ny; ++i)
	{
		reader.readLine(&xyz[0], &alpha[0]);
		writer.writeLine(&xyz[0], &alpha[0]);
	}
}


// --- ImageReader ---------------------------------------------------------------------------------

ImageReader::ImageReader(
		const std::string& filename, const TRgbSpacePtr& rgbSpace, const std::string& options):
	codec_(imageCodec(io::fileExtension(filename))),
	handle_(0)
{
	handle_ = codec_->open(filename, rgbSpace, options);
}

ImageReader::~ImageReader()
{
	codec_->close(handle_);
	handle_ = 0;
}

void ImageReader::readFull(XYZ* xyz, TScalar* alpha) const
{
	const TResolution2D res = resolution();
	const size_t size = res.x * res.y;
	for (size_t k = 0; k < size; k += res.x)
	{
		readLine(xyz + k, alpha ? alpha + k : 0);
	}
}



// --- ImageWriter ---------------------------------------------------------------------------------

ImageWriter::ImageWriter(
		const std::string& filename, const TResolution2D& resolution, const TRgbSpacePtr& rgbSpace, 
		const std::string& options):
	codec_(imageCodec(io::fileExtension(filename))),
	handle_(0)
{
	handle_ = codec_->create(filename, resolution, rgbSpace, options);
}

ImageWriter::~ImageWriter()
{
	codec_->close(handle_);
	handle_ = 0;
}

void ImageWriter::writeFull(XYZ* xyz, TScalar* alpha) const
{
	const TResolution2D res = resolution();
	const size_t size = res.x * res.y;
	for (size_t k = 0; k < size; k += res.x)
	{
		writeLine(xyz + k, alpha ? alpha + k : 0);
	}
}



// --- ImageCodecLass ------------------------------------------------------------------------

namespace impl
{
	struct LassImage
	{
		io::Image image;
		TRgbSpacePtr rgbSpace;
		std::string filename;
		size_t y;
		bool saveOnClose;
		LassImage(): y(0) {}
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



ImageCodec::TImageHandle ImageCodecLass::doCreate(const std::string& filename, const TResolution2D& resolution, const TRgbSpacePtr& rgbSpace, const std::string&) const
{
	typedef io::Image::TChromaticity TChromaticity;

	std::auto_ptr<impl::LassImage> pimpl(new impl::LassImage);
	pimpl->image.reset(resolution.y, resolution.x);
	pimpl->rgbSpace = selectRgbSpace(rgbSpace);
	pimpl->filename = filename;
	pimpl->saveOnClose = true;
	RgbSpace& space = *pimpl->rgbSpace;
	LASS_COUT << "RGB create: " << space.red() << " " << space.green() << " " << space.blue() << " " << space.white() << "\n";
	io::Image::ColorSpace& colorSpace = pimpl->image.colorSpace();
	colorSpace.red = TChromaticity(space.red());
	colorSpace.green = TChromaticity(space.green());
	colorSpace.blue = TChromaticity(space.blue());
	colorSpace.white = TChromaticity(space.white());
	colorSpace.gamma = static_cast<num::Tfloat32>(hasGammaCorrection_ ? space.gamma() : 1);
	return pimpl.release();
}



ImageCodec::TImageHandle ImageCodecLass::doOpen(const std::string& filename, const TRgbSpacePtr& rgbSpace, const std::string&) const
{
	std::auto_ptr<impl::LassImage> pimpl(new impl::LassImage);
	pimpl->image.open(filename);
	pimpl->rgbSpace = rgbSpace;
	if (!pimpl->rgbSpace)
	{
		io::Image::ColorSpace& colorSpace = pimpl->image.colorSpace();
		pimpl->rgbSpace.reset(new RgbSpace(
			TPoint2D(colorSpace.red), 
			TPoint2D(colorSpace.green), 
			TPoint2D(colorSpace.blue), 
			TPoint2D(colorSpace.white), 
			hasGammaCorrection_ ? colorSpace.gamma : 1));
	}
	RgbSpace& space = *pimpl->rgbSpace;
	LASS_COUT << "RGB open: " << space.red() << " " << space.green() << " " << space.blue() << " " << space.white() << "\n";
	pimpl->filename = filename;
	pimpl->saveOnClose = false;
	return pimpl.release();
}



void ImageCodecLass::doClose(TImageHandle handle) const
{
	impl::LassImage* impl = static_cast<impl::LassImage*>(handle);
	if (impl->saveOnClose)
	{
		impl->image.save(impl->filename);
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



void ImageCodecLass::doReadLine(
		TImageHandle handle, XYZ* xyz, TScalar* alpha) const
{
	impl::LassImage* pimpl = static_cast<impl::LassImage*>(handle);
	const io::Image& image = pimpl->image;
	const RgbSpace& space = *pimpl->rgbSpace;
	LASS_ENFORCE(pimpl->y < image.rows());
	for (size_t x = 0; x < image.cols(); ++x)
	{
		TScalar a;
		if (hasGammaCorrection_)
		{
			*xyz++ = space.convertGamma(image(pimpl->y, x), a);
		}
		else
		{
			*xyz++ = space.convert(image(pimpl->y, x), a);
		}
		if (alpha) *alpha++ = a;
	}
	++pimpl->y;
}



void ImageCodecLass::doWriteLine(
		TImageHandle handle, const XYZ* xyz, const TScalar* alpha) const
{
	impl::LassImage* pimpl = static_cast<impl::LassImage*>(handle);
	io::Image& image = pimpl->image;
	const RgbSpace& space = *pimpl->rgbSpace;
	LASS_ENFORCE(pimpl->y < image.rows());
	for (size_t x = 0; x < image.cols(); ++x)
	{
		const TScalar a = alpha ? *alpha++ : 1;
		if (hasGammaCorrection_)
		{
			image(pimpl->y, x) = space.convertGamma(*xyz++, a);
		}
		else
		{
			image(pimpl->y, x) = space.convert(*xyz++, a);
		}
	}
	++pimpl->y;
}

}
}

// EOF
