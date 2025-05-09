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

#include "../kernel/image_codec.h"
#include <lass/io/binary_i_file.h>
#include <lass/io/binary_o_file.h>
#include <lass/io/arg_parser.h>
#include "../kernel/icc_space.h"

#include <cstdio>
#include <csetjmp>
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
	SourceMgr(const std::filesystem::path& path):
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
	DestMgr(const std::filesystem::path& path):
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


struct ErrorMgr : public jpeg_error_mgr
{
	ErrorMgr()
	{
		jpeg_std_error(this);
		this->error_exit = &ErrorMgr::errorExit;
		lastErrorMsg[0] = '\0';
	}
	static void errorExit(j_common_ptr cinfo)
	{
		ErrorMgr* self = static_cast<ErrorMgr*>(cinfo->err);
		(*(self->format_message)) (cinfo, self->lastErrorMsg);
		jpeg_destroy(cinfo); // detroy any temp files
		longjmp(self->setjmpBuffer, 1);
	}
	char lastErrorMsg[JMSG_LENGTH_MAX];
	jmp_buf setjmpBuffer;
};


struct Handle
{
	typedef std::vector<JSAMPLE> TLine;
	kernel::TRgbSpacePtr rgbSpace;
	TResolution2D resolution;
	TLine line;
	ErrorMgr jerr = {};
	Handle(const kernel::TRgbSpacePtr& rgbSpace):
		rgbSpace(rgbSpace)
	{
	}
	virtual ~Handle()
	{
	}
};


struct JoctetDeleter
{
	void operator()(JOCTET* data) const { free(data); }
};


struct ReadHandle: Handle
{
	jpeg_decompress_struct cinfo;
	SourceMgr src;
	ReadHandle(const std::filesystem::path& path, const kernel::TRgbSpacePtr& rgbSpace):
		Handle(rgbSpace),
		src(path)
	{
		cinfo.err = &jerr;
		if (setjmp(jerr.setjmpBuffer))
		{
			LASS_THROW_EX(kernel::ImageCodecError, "Failed to read " << path << ": " << jerr.lastErrorMsg);
		}
		jpeg_create_decompress(&cinfo);
		cinfo.src = &src;
		jpeg_save_markers(&cinfo, JPEG_APP0 + 2, 0xFFFF);
		jpeg_read_header(&cinfo, TRUE);

#if LIAR_HAVE_LCMS2_H
		if (this->rgbSpace)
		{
			colorSpace.reset(new kernel::IccSpace(this->rgbSpace));
		}
		else
		{
			std::unique_ptr<JOCTET, JoctetDeleter> iccData;
			JOCTET* pIccData;
			unsigned int iccDataLen;
			if (jpeg_read_icc_profile(&cinfo, &pIccData, &iccDataLen))
			{
				iccData.reset(pIccData);
				colorSpace.reset(new kernel::IccSpace(iccData.get(), iccDataLen));
				this->rgbSpace = colorSpace->rgbSpace();
			}
			else
			{
				this->rgbSpace = kernel::RgbSpace::defaultSpace();
				colorSpace.reset(new kernel::IccSpace(this->rgbSpace));
			}
		}
#else
		if (!this->rgbSpace)
		{
			this->rgbSpace = kernel::RgbSpace::defaultSpace();
		}
		colorSpace = this->rgbSpace;
#endif

		cinfo.out_color_space = JCS_RGB;
		jpeg_start_decompress(&cinfo);
		LASS_ENFORCE(cinfo.output_components == 3);
		resolution = TResolution2D(cinfo.output_width, cinfo.output_height);
		line.resize(cinfo.output_width * static_cast<JDIMENSION>(cinfo.output_components));
	}
	~ReadHandle()
	{
		jpeg_destroy_decompress(&cinfo);
	}
#if LIAR_HAVE_LCMS2_H
	kernel::TIccSpacePtr colorSpace;
#else
	kernel::TRgbSpacePtr colorSpace;
#endif
};

struct WriteHandle: Handle
{
	jpeg_compress_struct cinfo;
	DestMgr dest;
	WriteHandle(const std::filesystem::path& path, const TResolution2D& resolution, const kernel::TRgbSpacePtr& rgbSpace, const std::string& options):
		Handle(rgbSpace),
		dest(path)
	{
		this->resolution = resolution;

		io::ArgParser parser;
		io::ArgValue<int> quality(parser, "q" ,"quality", "compression quality: 0-100");
		parser.parse(options);

		cinfo.err = &jerr;
		if (setjmp(jerr.setjmpBuffer))
		{
			LASS_THROW_EX(kernel::ImageCodecError, "Failed to write " << path << ": " << jerr.lastErrorMsg);
		}
		jpeg_create_compress(&cinfo);
		cinfo.dest = &dest;
		cinfo.image_width = static_cast<JDIMENSION>(resolution.x);
		cinfo.image_height = static_cast<JDIMENSION>(resolution.y);
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_RGB;
		jpeg_set_defaults(&cinfo);
		if (quality)
		{
			jpeg_set_quality(&cinfo, num::clamp(quality[0], 0, 100), TRUE);
		}
#if LIAR_HAVE_LCMS2_H
		colorSpace.reset(new kernel::IccSpace(this->rgbSpace));
		const auto iccProfile = colorSpace->iccProfile();
		const auto iccProfileSize = num::numCast<unsigned int>(iccProfile.size());
		jpeg_write_icc_profile(&cinfo, reinterpret_cast<const JOCTET*>(iccProfile.data()), iccProfileSize);
#endif
		jpeg_start_compress(&cinfo, TRUE);
		line.resize(cinfo.image_width * static_cast<JDIMENSION>(cinfo.input_components));
	}
	~WriteHandle()
	{
		jpeg_destroy_compress(&cinfo);
	}
#if LIAR_HAVE_LCMS2_H
	kernel::TIccSpacePtr colorSpace;
#endif
};

class ImageCodecJpeg: public kernel::ImageCodec
{
private:
	TImageHandle doCreate(const std::filesystem::path& path, const TResolution2D& resolution, const kernel::TRgbSpacePtr& rgbSpace, const std::string& options) const override
	{
		return new WriteHandle(path, resolution, rgbSpace, options);
	}

	TImageHandle doOpen(const std::filesystem::path& path, const kernel::TRgbSpacePtr& rgbSpace, const std::string&) const override
	{
		return new ReadHandle(path, rgbSpace);
	}

	void doClose(TImageHandle handle) const override
	{
		delete static_cast<Handle*>(handle);
	}

	const TResolution2D doResolution(TImageHandle handle) const override
	{
		return static_cast<Handle*>(handle)->resolution;
	}

	const kernel::TRgbSpacePtr doRgbSpace(TImageHandle handle) const override
	{
		return static_cast<Handle*>(handle)->rgbSpace;
	}

	void doReadLine(TImageHandle handle, kernel::XYZA* out) const override
	{
		ReadHandle* pimpl = static_cast<ReadHandle*>(handle);
		const auto& colorSpace = *pimpl->colorSpace;
		LASS_ENFORCE(pimpl->cinfo.output_scanline < pimpl->cinfo.output_height);
		JSAMPLE* line = &pimpl->line[0];
		jpeg_read_scanlines(&pimpl->cinfo, &line, 1);
		const size_t n = pimpl->line.size();
		for (size_t k = 0; k < n; k += 3)
		{
			const prim::ColorRGBA pixel(line[k] / 255.f, line[k + 1] / 255.f, line[k + 2] / 255.f, 1.f);
			*out++ = colorSpace.toXYZA(pixel);
		}
		if (pimpl->cinfo.output_scanline == pimpl->cinfo.output_height)
		{
			jpeg_finish_decompress(&pimpl->cinfo);
		}
	}

	void doWriteLine(TImageHandle handle, const prim::ColorRGBA* in) const override
	{
		WriteHandle* pimpl = static_cast<WriteHandle*>(handle);
		LASS_ENFORCE(pimpl->cinfo.next_scanline < pimpl->cinfo.image_height);
		JSAMPLE* line = &pimpl->line[0];
		const size_t n = pimpl->line.size();
		for (size_t k = 0; k < n; k += 3)
		{
			const prim::ColorRGBA& pixel = *in++;
			line[k] = static_cast<JSAMPLE>(num::clamp<TScalar>(255 * pixel.r, 0, 255));
			line[k + 1] = static_cast<JSAMPLE>(num::clamp<TScalar>(255 * pixel.g, 0, 255));
			line[k + 2] = static_cast<JSAMPLE>(num::clamp<TScalar>(255 * pixel.b, 0, 255));
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
	map[".jpg"] = map[".jpeg"] = liar::kernel::TImageCodecRef(new liar::jpeglib::ImageCodecJpeg);
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
