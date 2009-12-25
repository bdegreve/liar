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

#include "../kernel/image_codec.h"
#include <lass/io/binary_i_file.h>

#ifdef HAVE_STDDEF_H
#	undef HAVE_STDDEF_H
#endif
#include "jpeglib.h"

namespace experimental
{

template 
<
	typename T,
	typename Cascade = meta::EmptyType
>
class FileStorage: public Cascade
{
public:

	typedef FileStorage<T, Cascade> TSelf;
	typedef T* TStorage;
	typedef T* TPointer;
	typedef T& TReference;

	TStorage& storage() { return storage_; }
	const TStorage& storage() const { return storage_; }

protected:

	FileStorage(): Cascade(), storage_(defaultStorage()) {}
	explicit FileStorage(T* pointee): Cascade(), storage_(pointee) {}
	FileStorage(const TSelf& other): Cascade(other), storage_(other.storage_) {}
	template <typename U> FileStorage(const FileStorage<U, Cascade>& other): Cascade(other), storage_(other.storage()) {} 

	TPointer pointer() const { return storage_; }

	void dispose()
	{
		if (storage_)
		{
			::fclose(storage_);
		}
		storage_ = 0;
	}
	bool isNull() const { return !storage_; }
	void swap(TSelf& other) { Cascade::swap(other); std::swap(storage_, other.storage_); }
	static TStorage defaultStorage() { return 0; }

private:

	FileStorage(T* pointee, const Cascade& cascade): Cascade(cascade), storage_(pointee) {}
	TStorage storage_;
};

typedef lass::util::ScopedPtr<FILE, FileStorage> TScopedFilePtr;

}

namespace liar
{
namespace jpeg
{

struct Handle
{
	typedef std::vector<JSAMPLE> TImage;
	kernel::TRgbSpacePtr rgbSpace;
	TResolution2D resolution;
	TImage image;
};

class SourceMgr: public jpeg_source_mgr
{
public:
	SourceMgr(const std::string& filename):
		file_(filename),
		buffer_(bufferLength)
	{
		init_source = &SourceMgr::do_init_source;
		fill_input_buffer = &SourceMgr::do_fill_input_buffer;
		skip_input_data = &SourceMgr::do_skip_input_data;
		resync_to_restart = jpeg_resync_to_restart;
		term_source = &SourceMgr::do_term_source;
	}
private:
	enum 
	{ 
		bufferLength = 1024 
	};

	static void do_init_source(j_decompress_ptr cinfo)
	{
		SourceMgr* const self = static_cast<SourceMgr*>(cinfo->src);
		self->file_.seekg(0);
		do_fill_input_buffer(cinfo);
	}

	static boolean do_fill_input_buffer(j_decompress_ptr cinfo)
	{
		SourceMgr* const self = static_cast<SourceMgr*>(cinfo->src);
		self->next_input_byte = &self->buffer_[0];
		if (self->file_.good())
		{
			self->bytes_in_buffer = self->file_.read(&self->buffer_[0], bufferLength);
		}
		else
		{
			self->buffer_[0] = 0xff;
			self->buffer_[1] = JPEG_EOI;
			self->bytes_in_buffer = 2;		
		}
		return TRUE;
	}

	static void do_skip_input_data (j_decompress_ptr cinfo, long num_bytes)
	{
		SourceMgr* const self = static_cast<SourceMgr*>(cinfo->src);
		if (num_bytes <= 0)
		{
			return;
		}
		const size_t n = static_cast<size_t>(num_bytes);
		if (n < self->bytes_in_buffer)
		{
			self->next_input_byte += n;
			self->bytes_in_buffer -= n;
		}
		else
		{
			self->file_.seekg(num_bytes, std::ios_base::cur);
			do_fill_input_buffer(cinfo);
		}
	}

	static void do_term_source(j_decompress_ptr cinfo)
	{
	}

private:
	io::BinaryIFile file_;
	std::vector<JOCTET> buffer_;
};

class ImageCodecJpeg: public kernel::ImageCodec
{
private:
	const TImageHandle doCreate(
			const std::string& filename, LevelMode levelMode, 
			const TResolution2D& resolution, const kernel::TRgbSpacePtr& rgbSpace, const std::string& options) const
	{
		LASS_THROW("not supported yet");
		return 0;
	}

	const TImageHandle doOpen(
			const std::string& filename, const kernel::TRgbSpacePtr& rgbSpace, const std::string& options) const
	{
		std::auto_ptr<Handle> pimpl(new Handle);
		pimpl->rgbSpace = rgbSpace ? rgbSpace : kernel::RgbSpace::defaultSpace();

		jpeg_decompress_struct cinfo;
		jpeg_error_mgr jerr;
		cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_decompress(&cinfo);

		SourceMgr src(filename);
		cinfo.src = &src;

		jpeg_read_header(&cinfo, TRUE);
		cinfo.out_color_space = JCS_RGB;
		cinfo.output_gamma = pimpl->rgbSpace->gamma();
		jpeg_start_decompress(&cinfo);
		LASS_ENFORCE(cinfo.output_components == 3);
		pimpl->resolution = TResolution2D(cinfo.output_width, cinfo.output_height);
		const JDIMENSION output_stride = cinfo.output_width * cinfo.output_components;
		pimpl->image.resize(cinfo.output_height * output_stride);
		while (cinfo.output_scanline < cinfo.output_height) 
		{
			const size_t y = cinfo.output_height - cinfo.output_scanline - 1;
			JSAMPLE* line = &pimpl->image[y * output_stride];
			jpeg_read_scanlines(&cinfo, &line, 1);
		}
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);

		return pimpl.release();
	}

	void doClose(TImageHandle handle) const
	{
		const Handle* pimpl = static_cast<Handle*>(handle);
		delete pimpl;
	}

	const ImageCodec::LevelMode doLevelMode(TImageHandle handle) const
	{
		return lmSingleLevel;
	}

	const TLevel doLevels(TImageHandle handle) const
	{
		return TLevel(1, 1);
	}

	const TResolution2D doResolution(TImageHandle handle, const TLevel& level) const
	{
		const Handle* pimpl = static_cast<Handle*>(handle);
		return pimpl->resolution;
	}

	const kernel::TRgbSpacePtr doRgbSpace(TImageHandle handle) const
	{
		const Handle* pimpl = static_cast<Handle*>(handle);
		return pimpl->rgbSpace;
	}

	void doRead(
			TImageHandle handle, const TResolution2D& level, const TResolution2D& begin, 
			const TResolution2D& end, kernel::XYZ* xyz, TScalar* alpha) const
	{
		LASS_ASSERT(level.x == 0 && level.y == 0);
		const Handle* pimpl = static_cast<Handle*>(handle);
		const Handle::TImage& image = pimpl->image;
		for (size_t y = begin.y; y < end.y; ++y)
		{
			size_t k = 3 * (y * pimpl->resolution.x + begin.x);
			for (size_t x = begin.x; x < end.x; ++x)
			{
				LASS_ASSERT((k + 2) < image.size());
				const prim::ColorRGBA rgb(image[k] / 255.f, image[k + 1] / 255.f, image[k + 2] / 255.f);
				*xyz++ = pimpl->rgbSpace->convertGamma(rgb);
				k += 3;
			}
		}
		if (alpha)
		{
			std::fill_n(alpha, (end.x - begin.x) * (end.y * begin.y), 1);
		}
	}

	void doWrite(
			TImageHandle handle, const TResolution2D& level, const TResolution2D& begin, 
			const TResolution2D& end, const kernel::XYZ* xyz, const TScalar* alpha) const
	{
		LASS_THROW("not supported yet");
	}
};

}
}

PY_DECLARE_MODULE_DOC(jpeg, "JPEG image codec")

void jpegPostInject(PyObject*)
{
	liar::kernel::TImageCodecMap& map = liar::kernel::imageCodecs();
	map["jpg"] = map["jpeg"] = liar::kernel::TImageCodecPtr(new liar::jpeg::ImageCodecJpeg);
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.stdout.write('''liar.codecs.jpeg imported (v" LIAR_VERSION_FULL " - " __DATE__ ", " __TIME__ ")\n''')\n");
}

LASS_EXECUTE_BEFORE_MAIN(
	jpeg.setPostInject(jpegPostInject);
	)

PY_MODULE_ENTRYPOINT(jpeg)