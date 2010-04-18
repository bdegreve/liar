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
#include <lass/io/binary_o_file.h>
#include <lass/io/arg_parser.h>

#include <cstdio>
#if LASS_HAVE_SYS_TYPES_H
#	include <sys/types.h>
#endif

#ifdef HAVE_STDDEF_H
#	undef HAVE_STDDEF_H
#endif
#include "jpeglib.h"

#if LASS_COMPILER_TYPE == LASS_COMPILER_TYPE_MSVC
#	pragma warning(disable: 4996)
#endif

namespace liar
{
namespace jpeglib
{

class SourceMgr: public jpeg_source_mgr
{
public:
	SourceMgr(const std::string& path):
		file_(path),
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
		bufferLength = 4096 
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

	static void do_term_source(j_decompress_ptr)
	{
	}

	io::BinaryIFile file_;
	std::vector<JOCTET> buffer_;
};


class DestMgr: public jpeg_destination_mgr
{
public:
	DestMgr(const std::string& path):
		file_(path),
		buffer_(bufferLength)
	{
		init_destination = &DestMgr::do_init_destination;
		empty_output_buffer = &DestMgr::do_empty_output_buffer;
		term_destination = &DestMgr::do_term_destination;
	}
private:
	enum 
	{ 
		bufferLength = 4096
	};

	static void do_init_destination(j_compress_ptr cinfo)
	{
		DestMgr* const self = static_cast<DestMgr*>(cinfo->dest);
		self->next_output_byte = &self->buffer_[0];
		self->free_in_buffer = self->buffer_.size();
	}

	static boolean do_empty_output_buffer(j_compress_ptr cinfo)
	{
		DestMgr* const self = static_cast<DestMgr*>(cinfo->dest);
		self->file_.write(&self->buffer_[0], self->buffer_.size());
		self->file_.flush();
		self->next_output_byte = &self->buffer_[0];
		self->free_in_buffer = self->buffer_.size();
		return TRUE;
	}

	static void do_term_destination(j_compress_ptr cinfo)
	{
		DestMgr* const self = static_cast<DestMgr*>(cinfo->dest);
		const size_t inBuffer = self->buffer_.size() - self->free_in_buffer;
		LASS_ENFORCE(inBuffer <= self->buffer_.size());
		if (inBuffer > 0)
		{
			self->file_.write(&self->buffer_[0], inBuffer);
			self->file_.flush();
		}
	}

	io::BinaryOFile file_;
	std::vector<JOCTET> buffer_;
};

struct Handle
{
	typedef std::vector<JSAMPLE> TLine;
	kernel::TRgbSpacePtr rgbSpace;
	TResolution2D resolution;
	TLine line;
	jpeg_error_mgr jerr;
	Handle(const kernel::TRgbSpacePtr& rgbSpace):
		rgbSpace(rgbSpace ? rgbSpace : kernel::RgbSpace::defaultSpace())
	{
	}
	virtual ~Handle() 
	{
	}
};

struct ReadHandle: Handle
{
	jpeg_decompress_struct cinfo;
	SourceMgr src;
	ReadHandle(const std::string& path, const kernel::TRgbSpacePtr& rgbSpace):
		Handle(rgbSpace),
		src(path)
	{
		cinfo.err = jpeg_std_error(&jerr);
		jpeg_create_decompress(&cinfo);
		cinfo.src = &src;
		jpeg_read_header(&cinfo, TRUE);
		cinfo.out_color_space = JCS_RGB;
		cinfo.output_gamma = this->rgbSpace->gamma();
		jpeg_start_decompress(&cinfo);
		LASS_ENFORCE(cinfo.output_components == 3);
		resolution = TResolution2D(cinfo.output_width, cinfo.output_height);
		line.resize(cinfo.output_width * cinfo.output_components);
	}
	~ReadHandle()
	{
		jpeg_destroy_decompress(&cinfo);
	}
};

struct WriteHandle: Handle
{
	jpeg_compress_struct cinfo;
	DestMgr dest;
	WriteHandle(const std::string& path, const TResolution2D& resolution, const kernel::TRgbSpacePtr& rgbSpace, const std::string& options):
		Handle(rgbSpace),
		dest(path)
	{
		io::ArgParser parser;
		io::ArgValue<int> quality(parser, "q" ,"quality", "compression quality: 0-100");
		parser.parse(options);

		cinfo.err = jpeg_std_error(&jerr);
		jpeg_create_compress(&cinfo);
		cinfo.dest = &dest;
		cinfo.image_width = static_cast<JDIMENSION>(resolution.x);
		cinfo.image_height = static_cast<JDIMENSION>(resolution.y);
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_RGB;
		cinfo.input_gamma = this->rgbSpace->gamma();
		jpeg_set_defaults(&cinfo);
		if (quality)
		{
			jpeg_set_quality(&cinfo, num::clamp(quality[0], 0, 100), TRUE);
		}
		jpeg_start_compress(&cinfo, TRUE);
		line.resize(cinfo.image_width * cinfo.input_components);
	}
	~WriteHandle()
	{
		jpeg_destroy_compress(&cinfo);
	}
};

class ImageCodecJpeg: public kernel::ImageCodec
{
private:
	TImageHandle doCreate(const std::string& path, const TResolution2D& resolution, const kernel::TRgbSpacePtr& rgbSpace, const std::string& options) const
	{
		return new WriteHandle(path, resolution, rgbSpace, options);
	}

	TImageHandle doOpen(const std::string& path, const kernel::TRgbSpacePtr& rgbSpace, const std::string&) const
	{
		return new ReadHandle(path, rgbSpace);
	}

	void doClose(TImageHandle handle) const
	{
		delete static_cast<Handle*>(handle);
	}

	const TResolution2D doResolution(TImageHandle handle) const
	{
		return static_cast<Handle*>(handle)->resolution;
	}

	const kernel::TRgbSpacePtr doRgbSpace(TImageHandle handle) const
	{
		return static_cast<Handle*>(handle)->rgbSpace;
	}

	void doReadLine(TImageHandle handle, kernel::XYZ* xyz, TScalar* alpha) const
	{
		ReadHandle* pimpl = static_cast<ReadHandle*>(handle);
		LASS_ENFORCE(pimpl->cinfo.output_scanline < pimpl->cinfo.output_height);
		JSAMPLE* line = &pimpl->line[0];
		jpeg_read_scanlines(&pimpl->cinfo, &line, 1);
		const kernel::RgbSpace& rgbSpace = *pimpl->rgbSpace;
		const size_t n = pimpl->line.size();
		for (size_t k = 0; k < n; k += 3)
		{
			const prim::ColorRGBA rgb(line[k] / 255.f, line[k + 1] / 255.f, line[k + 2] / 255.f);
			*xyz++ = rgbSpace.convertGamma(rgb);
		}
		if (alpha)
		{
			std::fill_n(alpha, pimpl->resolution.x, 1);
		}
		if (pimpl->cinfo.output_scanline == pimpl->cinfo.output_height)
		{
			jpeg_finish_decompress(&pimpl->cinfo);
		}
	}

	void doWriteLine(TImageHandle handle, const kernel::XYZ* xyz, const TScalar* alpha) const
	{
		WriteHandle* pimpl = static_cast<WriteHandle*>(handle);
		LASS_ENFORCE(pimpl->cinfo.next_scanline < pimpl->cinfo.image_height);
		JSAMPLE* line = &pimpl->line[0];
		const kernel::RgbSpace& rgbSpace = *pimpl->rgbSpace;
		const size_t n = pimpl->line.size();
		for (size_t k = 0; k < n; k += 3)
		{
			const prim::ColorRGBA rgba = rgbSpace.convert(*xyz++, alpha ? *alpha++ : 1);
			line[k] = static_cast<JSAMPLE>(num::clamp<TScalar>(255 * rgba.r, 0, 255));
			line[k + 1] = static_cast<JSAMPLE>(num::clamp<TScalar>(255 * rgba.g, 0, 255));
			line[k + 2] = static_cast<JSAMPLE>(num::clamp<TScalar>(255 * rgba.b, 0, 255));
		}
		jpeg_write_scanlines(&pimpl->cinfo, &line, 1);
		if (pimpl->cinfo.next_scanline == pimpl->cinfo.image_height)
		{
			jpeg_finish_compress(&pimpl->cinfo);
		}
	}
};

void postInject(PyObject*)
{
	liar::kernel::TImageCodecMap& map = liar::kernel::imageCodecs();
	map["jpg"] = map["jpeg"] = liar::kernel::TImageCodecPtr(new liar::jpeglib::ImageCodecJpeg);
	LASS_COUT << "liar.codecs.jpeglib imported (v" LIAR_VERSION_FULL " - " __DATE__ ", " __TIME__ ")\n";
}

}
}

PY_DECLARE_MODULE_DOC(jpeglib, 
	"JPEG image codec\n"
	"\n"
	"this software is based in part on the work of the Independent JPEG Group\n"
	"(http://www.ijg.org/)\n"
	)

LASS_EXECUTE_BEFORE_MAIN(
	jpeglib.setPostInject(liar::jpeglib::postInject);
	)

PY_MODULE_ENTRYPOINT(jpeglib)

// EOF
