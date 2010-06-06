/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
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

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_IMAGE_CODEC_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_IMAGE_CODEC_H

#include "kernel_common.h"
#include "rgb_space.h"
#include <lass/prim/aabb_2d.h>

namespace liar
{
namespace kernel
{

/** Low level interface to image codecs
 */
class LIAR_KERNEL_DLL ImageCodec: public python::PyObjectPlus
{
	PY_HEADER(python::PyObjectPlus)
public:

	typedef void* TImageHandle;
	virtual ~ImageCodec();

	TImageHandle create(const std::string& filename, const TResolution2D& resolution, const TRgbSpacePtr& rgbSpace, const std::string& options) const 
	{
		return doCreate(filename, resolution, rgbSpace, options);
	}
	TImageHandle open(const std::string& filename, const TRgbSpacePtr& rgbSpace, const std::string& options) const 
	{
		return doOpen(filename, rgbSpace, options); 
	}
	void close(TImageHandle handle) const 
	{ 
		if (handle) doClose(handle); 
	}

	const TResolution2D resolution(TImageHandle handle) const
	{
		return doResolution(handle);
	}
	const TRgbSpacePtr rgbSpace(TImageHandle handle) const
	{
		return doRgbSpace(handle);
	}

	void readLine(TImageHandle handle, XYZ* xyz, TScalar* alpha) const
	{
		doReadLine(handle, xyz, alpha);
	}
	void writeLine(TImageHandle handle, const XYZ* xyz, const TScalar* alpha) const
	{
		doWriteLine(handle, xyz, alpha);
	}

private:
	virtual TImageHandle doCreate(const std::string& filename, const TResolution2D& resolution, const TRgbSpacePtr& rgbSpace, const std::string& options) const = 0;
	virtual TImageHandle doOpen(const std::string& filename, const TRgbSpacePtr& rgbSpace, const std::string& options) const = 0;
	virtual void doClose(TImageHandle) const = 0;

	virtual const TResolution2D doResolution(TImageHandle handle) const = 0;
	virtual const TRgbSpacePtr doRgbSpace(TImageHandle handle) const = 0;
	
	virtual void doReadLine(TImageHandle handle, XYZ* xyz, TScalar* alpha) const = 0;
	virtual void doWriteLine(TImageHandle handle, const XYZ* xyz, const TScalar* alpha) const = 0;
};

typedef python::PyObjectPtr<ImageCodec>::Type TImageCodecPtr;
typedef std::map<std::string, TImageCodecPtr> TImageCodecMap;

LIAR_KERNEL_DLL TImageCodecMap& LASS_CALL imageCodecs();
LIAR_KERNEL_DLL const TImageCodecPtr& LASS_CALL imageCodec(const std::string& extension);

LIAR_KERNEL_DLL void transcodeImage(const std::string& source, const std::string& dest, const TRgbSpacePtr& sourceSpace, const TRgbSpacePtr& destSpace);

class LIAR_KERNEL_DLL ImageReader: util::NonCopyable
{
public:

	ImageReader(const std::string& filename, const TRgbSpacePtr& rgbSpace = TRgbSpacePtr(), const std::string& options = "");
	~ImageReader();

	const TResolution2D resolution() const { return codec_->resolution(handle_); }
	const TRgbSpacePtr rgbSpace() const { return codec_->rgbSpace(handle_); }
	void readLine(XYZ* xyz, TScalar* alpha = 0) const { codec_->readLine(handle_, xyz, alpha); }
	void readFull(XYZ* xyz, TScalar* alpha = 0) const;
private:
	TImageCodecPtr codec_;
	ImageCodec::TImageHandle handle_;
};



class LIAR_KERNEL_DLL ImageWriter: util::NonCopyable
{
public:

	ImageWriter(const std::string& filename, const TResolution2D& resolution, const TRgbSpacePtr& rgbSpace = TRgbSpacePtr(), const std::string& options = "");
	~ImageWriter();

	const TResolution2D resolution() const { return codec_->resolution(handle_); }
	const TRgbSpacePtr rgbSpace() const { return codec_->rgbSpace(handle_); }
	void writeLine(const XYZ* xyz, const TScalar* alpha) const { codec_->writeLine(handle_, xyz, alpha); }
	void writeFull(XYZ* xyz, TScalar* alpha = 0) const;
private:
	TImageCodecPtr codec_;
	ImageCodec::TImageHandle handle_;
};


// built-in codecs

class LIAR_KERNEL_DLL ImageCodecLass: public ImageCodec
{
	PY_HEADER(ImageCodec)
public:
	ImageCodecLass(bool hasGammaCorrection = true, const TRgbSpacePtr& defaultSpace = TRgbSpacePtr());
protected:
	TRgbSpacePtr selectRgbSpace(const TRgbSpacePtr& defaultCodecSpace = TRgbSpacePtr()) const;
private:
	virtual TImageHandle doCreate(const std::string& filename, const TResolution2D& resolution, const TRgbSpacePtr& rgbSpace, const std::string& options) const;
	virtual TImageHandle doOpen(const std::string& filename, const TRgbSpacePtr& rgbSpace, const std::string& options) const;
	virtual void doClose(TImageHandle handle) const;

	virtual const TResolution2D doResolution(TImageHandle handle) const;
	virtual const TRgbSpacePtr doRgbSpace(TImageHandle handle) const;
	virtual void doReadLine(TImageHandle handle, XYZ* xyz, TScalar* alpha) const;	
	virtual void doWriteLine(TImageHandle handle, const XYZ* xyz, const TScalar* alpha) const;

	TRgbSpacePtr defaultCodecSpace_;
	bool hasGammaCorrection_;
};



}

}

#endif

// EOF
