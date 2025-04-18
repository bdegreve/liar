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

#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfStandardAttributes.h>
#include <lass/io/binary_i_file.h>
#include <lass/io/binary_o_file.h>

namespace liar
{
namespace openexr
{

class LassIStream: public Imf::IStream
{
public:
	using pos_type = lass::io::BinaryIFile::pos_type;
	LassIStream(const std::filesystem::path& path) : Imf::IStream(path.string().c_str()), file_(path) {}
	bool read(char c[/*n*/], int n) override { file_.read(c, num::numCast<size_t>(n)); return true; }
	uint64_t tellg() override { return num::numCast<uint64_t>( file_.tellg() ); }
	void seekg(uint64_t pos) override { file_.seekg( num::numCast<pos_type>(pos) ); }
	void clear() override {}
private:
	lass::io::BinaryIFile file_;
};

class LassOStream: public Imf::OStream
{
public:
	using pos_type = lass::io::BinaryOFile::pos_type;
	LassOStream(const std::filesystem::path& path): Imf::OStream(path.string().c_str()), file_(path) {}
	void write(const char c[], int n) override { file_.write(c, num::numCast<size_t>(n)); }
	uint64_t tellp() override { return num::numCast<uint64_t>( file_.tellp() ); }
	void seekp(uint64_t pos) override { file_.seekp( num::numCast<pos_type>(pos) ); }
private:
	lass::io::BinaryOFile file_;
};

struct Handle
{
	typedef std::vector<Imf::Rgba> TLine;
	kernel::TRgbSpaceRef rgbSpace;
	TResolution2D resolution;
	TLine line;
	std::unique_ptr<LassIStream> istream;
	std::unique_ptr<LassOStream> ostream;
	std::unique_ptr<Imf::RgbaInputFile> input;
	std::unique_ptr<Imf::RgbaOutputFile> output;
	int y = 0;
	Handle(const TResolution2D& resolution, const kernel::TRgbSpacePtr& rgbSpace):
		rgbSpace((rgbSpace ? rgbSpace : kernel::sRGB.ptr())->linearSpace()),
		resolution(resolution)
	{
	}
	~Handle()
	{
		input.reset();
		output.reset();
		istream.reset();
		ostream.reset();
	};
};

inline Imath::V2f point2DToV2f(const TPoint2D& p)
{
	return Imath::V2f(static_cast<float>(p.x), static_cast<float>(p.y));
}

inline TPoint2D V2fToPoint2D(const Imath::V2f& p)
{
	return TPoint2D(static_cast<TScalar>(p.x), static_cast<TScalar>(p.y));
}

class ImageCodecOpenEXR: public kernel::ImageCodec
{
private:
	TImageHandle doCreate(const std::filesystem::path& path, const TResolution2D& resolution, const kernel::TRgbSpacePtr& rgbSpace, const std::string&) const override
	{
		std::unique_ptr<Handle> pimpl(new Handle(resolution, rgbSpace));
		pimpl->line.resize(resolution.x);
		Imf::Header header(num::numCast<int>(resolution.x), num::numCast<int>(resolution.y));
		Imf::Chromaticities chromaticities;
		chromaticities.red = point2DToV2f(pimpl->rgbSpace->red());
		chromaticities.green = point2DToV2f(pimpl->rgbSpace->green());
		chromaticities.blue = point2DToV2f(pimpl->rgbSpace->blue());
		chromaticities.white = point2DToV2f(pimpl->rgbSpace->white());
		header.insert("chromaticities", Imf::ChromaticitiesAttribute(chromaticities));
		header.insert("comments", Imf::StringAttribute("rendered by " LIAR_NAME_FULL ", " LIAR_WEBSITE));
		pimpl->ostream.reset(new LassOStream(path));
		pimpl->output.reset(new Imf::RgbaOutputFile(*pimpl->ostream, header, Imf::WRITE_RGBA, 0));
		return pimpl.release();
	}

	TImageHandle doOpen(const std::filesystem::path& path, const kernel::TRgbSpacePtr& rgbSpace, const std::string&) const override
	{
		std::unique_ptr<LassIStream> istream(new LassIStream(path));
		std::unique_ptr<Imf::RgbaInputFile> input(new Imf::RgbaInputFile(*istream, 0));
		const Imath::Box2i& dispWin = input->header().displayWindow();
		const Imath::Box2i& dataWin = input->header().dataWindow();
		const TResolution2D resolution(dispWin.max.x - dispWin.min.x + 1, dispWin.max.y - dispWin.min.y + 1);
		const kernel::TRgbSpacePtr space = determineRgbSpaceForOpenFile(input->header(), rgbSpace);
		std::unique_ptr<Handle> pimpl(new Handle(resolution, space));
		pimpl->line.resize(num::numCast<size_t>(dataWin.max.x - dataWin.min.x + 1));
		pimpl->istream.swap(istream);
		pimpl->input.swap(input);
		pimpl->y = dispWin.min.y;
		return pimpl.release();
	}

	void doClose(TImageHandle handle) const override
	{
		Handle* h = static_cast<Handle*>(handle);
		delete h;
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
		const kernel::XYZA nodata(0, 0, 0, 0);

		Handle* pimpl = static_cast<Handle*>(handle);
		const Imath::Box2i& dispWin = pimpl->input->displayWindow();
		const Imath::Box2i& dataWin = pimpl->input->dataWindow();
		LASS_ENFORCE(pimpl->y <= dispWin.max.y);

		const auto& rgbSpace = *(pimpl->rgbSpace);

		if (pimpl->y >= dataWin.min.y && pimpl->y <= dataWin.max.y)
		{
			Handle::TLine& line = pimpl->line;
			pimpl->input->setFrameBuffer(
				&line[0] - pimpl->y * static_cast<int>(line.size()) - dataWin.min.x,
				1, line.size());
			pimpl->input->readPixels(pimpl->y);
			for (int x = dispWin.min.x; x <= dispWin.max.x; ++x)
			{
				if (x >= dataWin.min.x && x <= dataWin.max.x)
				{
					const Imf::Rgba& pixel = line[static_cast<size_t>(x - dataWin.min.x)];
					const prim::ColorRGBA rgba(pixel.r, pixel.g, pixel.b, pixel.a);
					*out++ = rgbSpace.toXYZA(rgba);
				}
				else
				{
					*out++ = nodata;
				}
			}
		}
		else
		{
			std::fill_n(out, pimpl->resolution.x, nodata);
		}
		++pimpl->y;
	}

	void doWriteLine(TImageHandle handle, const prim::ColorRGBA* in) const override
	{
		Handle* pimpl = static_cast<Handle*>(handle);
		Imf::RgbaOutputFile& output = *pimpl->output;
		Handle::TLine& line = pimpl->line;
		const size_t n = pimpl->resolution.x;
		for (size_t x = 0; x < n; ++x)
		{
			const prim::ColorRGBA& pixel = *in++;
			line[x] = Imf::Rgba(pixel.r, pixel.g, pixel.b, pixel.a);
		}
		output.setFrameBuffer(&line[0] - output.currentScanLine() * static_cast<int>(line.size()), 1, line.size());
		output.writePixels(1);
	}

private:

	static kernel::TRgbSpacePtr determineRgbSpaceForOpenFile(const Imf::Header& fileHeader, const kernel::TRgbSpacePtr& customSpace)
	{
		if (customSpace)
		{
			return customSpace;
		}
		if (const Imf::ChromaticitiesAttribute* chromaticities = fileHeader.findTypedAttribute<Imf::ChromaticitiesAttribute>("chromaticities"))
		{
			const Imf::Chromaticities chromas = chromaticities->value();
			return kernel::TRgbSpacePtr(new kernel::RgbSpace(
				V2fToPoint2D(chromas.red),
				V2fToPoint2D(chromas.green),
				V2fToPoint2D(chromas.blue),
				V2fToPoint2D(chromas.white)));
		}
		return kernel::RgbSpace::defaultSpace();
	}
};

void postInject(PyObject*)
{
	liar::kernel::TImageCodecMap& map = liar::kernel::imageCodecs();
	map[".exr"] = liar::kernel::TImageCodecRef(new liar::openexr::ImageCodecOpenEXR);
	LASS_COUT << "liar.codecs.openexr imported (v" LIAR_VERSION_FULL " - " __DATE__ ", " __TIME__ ")\n";
}

}
}

PY_DECLARE_MODULE_DOC(openexr,
	"OpenEXR image codec\n"
	"\n"
	"OpenEXR\n"
	"-------\n"
	"\n"
	"This software is based in part on the OpenEXR library\n"
	"(http://www.openexr.com/):\n"
	"\n"
	"Copyright (c) 2006, Industrial Light & Magic, a division of Lucasfilm\n"
	"Entertainment Company Ltd.  Portions contributed and copyright held by\n"
	"others as indicated.  All rights reserved.\n"
	"\n"
	"Redistribution and use in source and binary forms, with or without\n"
	"modification, are permitted provided that the following conditions are\n"
	"met:\n"
	"\n"
	"    * Redistributions of source code must retain the above\n"
	"      copyright notice, this list of conditions and the following\n"
	"      disclaimer.\n"
	"\n"
	"    * Redistributions in binary form must reproduce the above\n"
	"      copyright notice, this list of conditions and the following\n"
	"      disclaimer in the documentation and/or other materials provided with\n"
	"      the distribution.\n"
	"\n"
	"    * Neither the name of Industrial Light & Magic nor the names of\n"
	"      any other contributors to this software may be used to endorse or\n"
	"      promote products derived from this software without specific prior\n"
	"      written permission.\n"
	"\n"
	"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS\n"
	"IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,\n"
	"THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR\n"
	"PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR\n"
	"CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,\n"
	"EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,\n"
	"PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR\n"
	"PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF\n"
	"LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING\n"
	"NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n"
	"SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.)\n"
	)

LASS_EXECUTE_BEFORE_MAIN(
	openexr.setPostInject(liar::openexr::postInject);
	)

PY_MODULE_ENTRYPOINT(openexr)

// EOF
