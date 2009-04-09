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

PY_DECLARE_CLASS_DOC(ImageCodecLassLDR, "Lass based image codec for Low Dynamic Range images")
PY_CLASS_CONSTRUCTOR_0(ImageCodecLassLDR)
LASS_EXECUTE_BEFORE_MAIN_EX(ImageCodecLassLDR,
	TImageCodecMap& map = imageCodecs();
	map["tga"] = map["targa"] = TImageCodecPtr(new ImageCodecLassLDR);
)

PY_DECLARE_CLASS_DOC(ImageCodecLassHDR, "Lass based image codec for High Dynamic Range images")
PY_CLASS_CONSTRUCTOR_0(ImageCodecLassHDR)
LASS_EXECUTE_BEFORE_MAIN_EX(ImageCodecLassHDR,
	TImageCodecMap& map = imageCodecs();
	map["lass"] = map["hdr"] = map["pic"] = map["rgbe"] = map["igi"] = 
		TImageCodecPtr(new ImageCodecLassHDR);
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



// --- ImageWriter ---------------------------------------------------------------------------------

ImageWriter::ImageWriter(
		const std::string& filename, ImageCodec::LevelMode levelMode, 
		const TResolution2D& resolution, const TRgbSpacePtr& rgbSpace, const std::string& options):
	codec_(imageCodec(io::fileExtension(filename))),
	handle_(0)
{
	handle_ = codec_->create(filename, levelMode, resolution, rgbSpace, options);
}

ImageWriter::~ImageWriter()
{
	codec_->close(handle_);
	handle_ = 0;
}



// --- ImageCodecLassCommon ------------------------------------------------------------------------

namespace impl
{
	struct LassImage
	{
		io::Image image;
		TRgbSpacePtr rgbSpace;
		std::string filename;
		bool saveOnClose;
	};
}



const ImageCodec::TImageHandle ImageCodecLassCommon::doCreate(
		const std::string& filename, ImageCodec::LevelMode levelMode,
		const TResolution2D& resolution, const TRgbSpacePtr& rgbSpace, 
		const std::string& options) const
{
	typedef io::Image::TChromaticity TChromaticity;

	if (levelMode != lmSingleLevel)
	{
		LASS_THROW("ImageCodecLass only supports single level images");
	}

	std::auto_ptr<impl::LassImage> pimpl(new impl::LassImage);
	pimpl->image.reset(resolution.y, resolution.x);
	pimpl->rgbSpace = rgbSpace;
	pimpl->filename = filename;
	pimpl->saveOnClose = true;
	if (!pimpl->rgbSpace)
	{
		pimpl->rgbSpace = RgbSpace::defaultSpace();
	}
	RgbSpace& space = *pimpl->rgbSpace;
	LASS_COUT << "RGB create: " << space.red() << " " << space.green() << " " << space.blue() << " " << space.white() << "\n";
	io::Image::ColorSpace& colorSpace = pimpl->image.colorSpace();
	colorSpace.red = TChromaticity(space.red());
	colorSpace.green = TChromaticity(space.green());
	colorSpace.blue = TChromaticity(space.blue());
	colorSpace.white = TChromaticity(space.white());
	colorSpace.gamma = space.gamma(); // may be forced to one later.
	return pimpl.release();
}



const ImageCodec::TImageHandle ImageCodecLassCommon::doOpen(
		const std::string& filename, const TRgbSpacePtr& rgbSpace, const std::string& options) const
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
			colorSpace.gamma));
	}
	RgbSpace& space = *pimpl->rgbSpace;
	LASS_COUT << "RGB open: " << space.red() << " " << space.green() << " " << space.blue() << " " << space.white() << "\n";
	pimpl->filename = filename;
	pimpl->saveOnClose = false;
	return pimpl.release();
}



void ImageCodecLassCommon::doClose(TImageHandle handle) const
{
	impl::LassImage* impl = static_cast<impl::LassImage*>(handle);
	if (impl->saveOnClose)
	{
		impl->image.save(impl->filename);
	}
	delete impl;
}



const ImageCodec::LevelMode ImageCodecLassCommon::doLevelMode(TImageHandle handle) const
{
	return lmSingleLevel;
}



const ImageCodec::TLevel ImageCodecLassCommon::doLevels(TImageHandle handle) const
{
	return TLevel(1, 1);
}



const TResolution2D ImageCodecLassCommon::doResolution(TImageHandle handle, const TLevel& level) const
{
	LASS_ENFORCE(level.x == 0 && level.y == 0);
	impl::LassImage* impl = static_cast<impl::LassImage*>(handle);
	return TResolution2D(impl->image.cols(), impl->image.rows());
}



const TRgbSpacePtr ImageCodecLassCommon::doRgbSpace(TImageHandle handle) const
{
	return static_cast<impl::LassImage*>(handle)->rgbSpace;
}



// --- ImageCodecLassLDR ---------------------------------------------------------------------------

void ImageCodecLassLDR::doRead(
		TImageHandle handle, const TResolution2D& level, const TResolution2D& begin, 
		const TResolution2D& end, TVector3D* xyz, TScalar* alpha) const
{
	const impl::LassImage* impl = static_cast<impl::LassImage*>(handle);
	const io::Image& image = impl->image;
	const RgbSpace& space = *impl->rgbSpace;
	for (unsigned y = begin.y; y < end.y; ++y)
	{
		for (unsigned x = begin.x; x < end.x; ++x)
		{
			TScalar a;
			*xyz++ = space.convertGamma(image(y, x), a);
			if (alpha) *alpha++ = a;
		}
	}
}

void ImageCodecLassLDR::doWrite(
		TImageHandle handle, const TResolution2D& level, const TResolution2D& begin, 
		const TResolution2D& end, const TVector3D* xyz, const TScalar* alpha) const
{
	impl::LassImage* impl = static_cast<impl::LassImage*>(handle);
	io::Image& image = impl->image;
	const RgbSpace& space = *impl->rgbSpace;
	for (unsigned y = begin.y; y < end.y; ++y)
	{
		for (unsigned x = begin.x; x < end.x; ++x)
		{
			const TScalar a = alpha ? *alpha++ : 1;
			image(y, x) = space.convertGamma(*xyz++, a);
		}
	}
}



// --- ImageCodecLassHDR ---------------------------------------------------------------------------

void ImageCodecLassHDR::doRead(
		TImageHandle handle, const TResolution2D& level, const TResolution2D& begin, 
		const TResolution2D& end, TVector3D* xyz, TScalar* alpha) const
{
	const impl::LassImage* impl = static_cast<impl::LassImage*>(handle);
	const io::Image& image = impl->image;
	const RgbSpace& space = *impl->rgbSpace;
	for (unsigned y = begin.y; y < end.y; ++y)
	{
		for (unsigned x = begin.x; x < end.x; ++x)
		{
			TScalar a;
			*xyz++ = space.convert(image(y, x), a);
			if (alpha) *alpha++ = a;
		}
	}
}

void ImageCodecLassHDR::doWrite(
		TImageHandle handle, const TResolution2D& level, const TResolution2D& begin, 
		const TResolution2D& end, const TVector3D* xyz, const TScalar* alpha) const
{
	impl::LassImage* impl = static_cast<impl::LassImage*>(handle);
	io::Image& image = impl->image;
	image.colorSpace().gamma = 1.f; // force gamma to one.
	const RgbSpace& space = *impl->rgbSpace;
	for (unsigned y = begin.y; y < end.y; ++y)
	{
		for (unsigned x = begin.x; x < end.x; ++x)
		{
			const TScalar a = alpha ? *alpha++ : 1;
			image(y, x) = space.convert(*xyz++, a);
		}
	}
}

}
}

// EOF
