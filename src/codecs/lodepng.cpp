/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2021-2023  Bram de Greve (bramz@users.sourceforge.net)
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
#include <lass/io/arg_parser.h>
#include <lass/util/wchar_support.h>

#include <lodepng.h>

#include <stdio.h>

#if LASS_COMPILER_TYPE == LASS_COMPILER_TYPE_MSVC
#	pragma warning(disable: 4996)
#endif

namespace
{

class ScopedFILE: lass::util::NonCopyable
{
public:
	explicit ScopedFILE(FILE* fp): fp_(fp) {}
	~ScopedFILE() { ::fclose(fp_); }
	operator FILE* () const { return fp_; }
private:
	FILE* fp_;
};

}

namespace liar
{
namespace lodepng
{

struct Handle
{
	Handle(unsigned width, unsigned height, const kernel::TRgbSpacePtr& rgbSpace):
		rgbSpace(rgbSpace),
		width(width),
		height(height),
		y(0)
	{
	}
	virtual ~Handle() {}
	const kernel::TRgbSpacePtr rgbSpace;
	unsigned width;
	unsigned height;
	unsigned y;
};

struct ReadHandle: public Handle
{
	ReadHandle(const std::wstring& path, const kernel::TRgbSpacePtr& rgbSpace):
		Handle(0, 0, rgbSpace),
		bytes(0)
	{
#if defined(_WIN32)
		ScopedFILE fp(::_wfopen(path.c_str(), L"rb"));
#else
		std::string utf8Path = util::wcharToUtf8(path);
		ScopedFILE fp(::fopen(utf8Path.c_str(), "rb"));
#endif
		::fseek(fp, 0L, SEEK_END);
		const long signedFileSize = ::ftell(fp);
		LASS_ENFORCE(signedFileSize > 0);
		const size_t fileSize = static_cast<size_t>(signedFileSize);
		::fseek(fp, 0L, SEEK_SET);
		std::vector<unsigned char> file(fileSize, '\0');
		const size_t bytesRead = ::fread(&file[0], sizeof(unsigned char), fileSize, fp);
		LASS_ENFORCE(bytesRead == fileSize);
		const unsigned err = ::lodepng_decode32(&this->bytes, &this->width, &this->height, &file[0], fileSize);
		if (err != 0)
		{
			throw lass::util::Exception(::lodepng_error_text(err));
		}
	}

	~ReadHandle()
	{
		::free(bytes);
	}

	unsigned char* bytes;
};

struct WriteHandle: public Handle
{
	WriteHandle(const std::wstring& path, const TResolution2D& resolution, const kernel::TRgbSpacePtr& rgbSpace):
		Handle(num::numCast<unsigned>(resolution.x), num::numCast<unsigned>(resolution.y), rgbSpace),
		path(path)
	{
		bytes.resize(resolution.x * resolution.y);
	}

	void save()
	{
		unsigned char* out;
		size_t size;
		const unsigned err = lodepng_encode32(&out, &size, &this->bytes[0], this->width, this->height);
		if (err != 0)
		{
			throw lass::util::Exception(::lodepng_error_text(err));
		}
#if defined(_WIN32)
		ScopedFILE fp(::_wfopen(path.c_str(), L"wb"));
#else
		std::string utf8Path = util::wcharToUtf8(path);
		ScopedFILE fp(::fopen(utf8Path.c_str(), "wb"));
#endif
		const size_t bytesWritten = ::fwrite(out, sizeof(unsigned char), size, fp);
		::free(out);
		LASS_ENFORCE(bytesWritten == size);
	}

	~WriteHandle()
	{
	}

	const std::wstring path;
	const std::string options;
	std::vector<unsigned char> bytes;
};




class ImageCodecLodePng: public kernel::ImageCodec
{
private:
	TImageHandle doCreate(const std::wstring& path, const TResolution2D& resolution, const kernel::TRgbSpacePtr& rgbSpace, const std::string&) const
	{
		return new WriteHandle(path, resolution, rgbSpace);
	}

	TImageHandle doOpen(const std::wstring& path, const kernel::TRgbSpacePtr& rgbSpace, const std::string&) const
	{
		return new ReadHandle(path, rgbSpace);
	}

	void doClose(TImageHandle handle) const
	{
		Handle* h = static_cast<Handle*>(handle);
		if (WriteHandle* wh = dynamic_cast<WriteHandle*>(h))
			wh->save();
		delete h;
	}

	const TResolution2D doResolution(TImageHandle handle) const
	{
		Handle& h = *static_cast<Handle*>(handle);
		return TResolution2D(h.width, h.height);
	}

	const kernel::TRgbSpacePtr doRgbSpace(TImageHandle handle) const
	{
		return static_cast<Handle*>(handle)->rgbSpace;
	}

	void doReadLine(TImageHandle handle, prim::ColorRGBA* out) const
	{
		ReadHandle* pimpl = static_cast<ReadHandle*>(handle);
		LASS_ENFORCE(pimpl);
		LASS_ENFORCE(pimpl->y < pimpl->height);
		const size_t lineSize = 4 * static_cast<size_t>(pimpl->width);
		unsigned char* line = pimpl->bytes + pimpl->y * lineSize;
		for (size_t k = 0; k < lineSize; k += 4)
		{
			const prim::ColorRGBA pixel(line[k] / 255.f, line[k + 1] / 255.f, line[k + 2] / 255.f, line[k + 3] / 255.f);
			*out++ = pixel;
		}
		++pimpl->y;
	}

	void doWriteLine(TImageHandle handle, const prim::ColorRGBA* in) const
	{
		WriteHandle* pimpl = static_cast<WriteHandle*>(handle);
		LASS_ENFORCE(pimpl);
		LASS_ENFORCE(pimpl->y < pimpl->height);
		const size_t lineSize = 4 * static_cast<size_t>(pimpl->width);
		unsigned char* line = &pimpl->bytes[pimpl->y * lineSize];
		for (size_t k = 0; k < lineSize; k += 4)
		{
			const prim::ColorRGBA& pixel = *in++;
			line[k] = static_cast<unsigned char>(num::clamp<TScalar>(255 * pixel.r, 0, 255));
			line[k + 1] = static_cast<unsigned char>(num::clamp<TScalar>(255 * pixel.g, 0, 255));
			line[k + 2] = static_cast<unsigned char>(num::clamp<TScalar>(255 * pixel.b, 0, 255));
			line[k + 3] = static_cast<unsigned char>(num::clamp<TScalar>(255 * pixel.a, 0, 255));
		}
		++pimpl->y;
	}
};

void postInject(PyObject*)
{
	liar::kernel::TImageCodecMap& map = liar::kernel::imageCodecs();
	map[L"png"] = liar::kernel::TImageCodecPtr(new liar::lodepng::ImageCodecLodePng);
	LASS_COUT << "liar.codecs.lodepng imported (v" LIAR_VERSION_FULL " - " __DATE__ ", " __TIME__ ")\n";
}

}
}

PY_DECLARE_MODULE_NAME_DOC(lodepng_, "lodepng",
	"PNG image codec\n"
	"\n"
	"this software is based in part on the work of LodePNG\n"
	"(https://lodev.org/lodepng/)\n"
	)

LASS_EXECUTE_BEFORE_MAIN(
	lodepng_.setPostInject(liar::lodepng::postInject);
	)

PY_MODULE_ENTRYPOINT_NAME(lodepng_, lodepng)

// EOF
