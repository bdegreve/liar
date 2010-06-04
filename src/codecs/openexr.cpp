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

#if LASS_COMPILER_TYPE == LASS_COMPILER_TYPE_MSVC
#	pragma warning(disable: 4996) // ImfName.h(103) : warning C4996: 'strncpy': This function or variable may be unsafe.
#	pragma warning(disable: 4231) // ImfAttribute.h(410) : warning C4231: nonstandard extension used : 'extern' before template explicit instantiation
#endif

#include <ImfRgbaFile.h>
#include <ImfStandardAttributes.h>

namespace liar
{
namespace openexr
{

struct Handle
{
	typedef std::vector<Imf::Rgba> TLine;
	kernel::TRgbSpacePtr rgbSpace;
	TResolution2D resolution;
	TLine line;
	util::ScopedPtr<Imf::RgbaInputFile> input;
	util::ScopedPtr<Imf::RgbaOutputFile> output;
	int y;
	Handle(const TResolution2D& resolution, const kernel::TRgbSpacePtr& rgbSpace):
		rgbSpace(new kernel::RgbSpace((rgbSpace ? rgbSpace : kernel::sRGB)->withGamma(1))),
		resolution(resolution)
	{	
	}
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
	TImageHandle doCreate(const std::string& filename, const TResolution2D& resolution, const kernel::TRgbSpacePtr& rgbSpace, const std::string&) const
	{
		std::auto_ptr<Handle> pimpl(new Handle(resolution, rgbSpace));
		pimpl->line.resize(resolution.x);
		Imf::Header header(num::numCast<int>(resolution.x), num::numCast<int>(resolution.y));
		Imf::Chromaticities chromaticities;
		chromaticities.red = point2DToV2f(pimpl->rgbSpace->red());
		chromaticities.green = point2DToV2f(pimpl->rgbSpace->green());
		chromaticities.blue = point2DToV2f(pimpl->rgbSpace->blue());
		chromaticities.white = point2DToV2f(pimpl->rgbSpace->white());
		header.insert("chromaticities", Imf::ChromaticitiesAttribute(chromaticities));
		header.insert("comments", Imf::StringAttribute("rendered by LiAR, http://liar.bramz.net/"));
		pimpl->output.reset(new Imf::RgbaOutputFile(filename.c_str(), header, Imf::WRITE_RGBA, 0));
		return pimpl.release();
	}

	TImageHandle doOpen(const std::string& filename, const kernel::TRgbSpacePtr& rgbSpace, const std::string&) const
	{
		util::ScopedPtr<Imf::RgbaInputFile> input(new Imf::RgbaInputFile(filename.c_str(), 0));
		const Imath::Box2i& dispWin = input->header().displayWindow();
		const Imath::Box2i& dataWin = input->header().dataWindow();
		const TResolution2D resolution(dispWin.max.x - dispWin.min.x + 1, dispWin.max.y - dispWin.min.y + 1);
		const kernel::TRgbSpacePtr space = determineRgbSpaceForOpenFile(input->header(), rgbSpace);
		std::auto_ptr<Handle> pimpl(new Handle(resolution, space));
		pimpl->line.resize(dataWin.max.x - dataWin.min.x + 1);
		pimpl->input.swap(input);
		pimpl->y = dispWin.min.y;
		return pimpl.release();
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
		Handle* pimpl = static_cast<Handle*>(handle);
		const Imath::Box2i& dispWin = pimpl->input->displayWindow();
		const Imath::Box2i& dataWin = pimpl->input->dataWindow();
		LASS_ENFORCE(pimpl->y <= dispWin.max.y);
		if (pimpl->y >= dataWin.min.y && pimpl->y <= dataWin.max.y)
		{
			Handle::TLine& line = pimpl->line;
			pimpl->input->setFrameBuffer(&line[0] - pimpl->y * line.size() - dataWin.min.x, 1, line.size());
			pimpl->input->readPixels(pimpl->y);
			for (int x = dispWin.min.x; x <= dispWin.max.x; ++x)
			{
				TScalar a = 0;
				if (x >= dataWin.min.x && x <= dataWin.max.x)
				{
					const Imf::Rgba& rgba = line[x - dataWin.min.x];
					*xyz++ = pimpl->rgbSpace->convert(prim::ColorRGBA(rgba.r, rgba.g, rgba.b, rgba.a), a);
				}
				else
				{
					*xyz++ = kernel::XYZ();
				}
				if (alpha)
				{
					*alpha++ = a;
				}
			}
		}
		else
		{
			std::fill_n(xyz, pimpl->resolution.x, kernel::XYZ());
			if (alpha)
			{
				std::fill_n(alpha, pimpl->resolution.x, 0);
			}
		}
		++pimpl->y;
	}

	void doWriteLine(TImageHandle handle, const kernel::XYZ* xyz, const TScalar* alpha) const
	{
		Handle* pimpl = static_cast<Handle*>(handle);
		Imf::RgbaOutputFile& output = *pimpl->output;
		const kernel::RgbSpace& rgbSpace = *pimpl->rgbSpace;
		Handle::TLine& line = pimpl->line;
		const size_t n = pimpl->resolution.x;
		for (size_t x = 0; x < n; ++x)
		{
			const TScalar a = alpha ? *alpha++ : 1;
			const prim::ColorRGBA rgba = rgbSpace.convert(*xyz++, a);
			line[x] = Imf::Rgba(rgba.r, rgba.g, rgba.b, rgba.a);
		}
		output.setFrameBuffer(&line[0] - output.currentScanLine() * line.size(), 1, line.size());
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
		return kernel::TRgbSpacePtr();
	}
};

void postInject(PyObject*)
{
	liar::kernel::TImageCodecMap& map = liar::kernel::imageCodecs();
	map["exr"] = liar::kernel::TImageCodecPtr(new liar::openexr::ImageCodecOpenEXR);
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
