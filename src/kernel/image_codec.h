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

	enum LevelMode
	{
		lmSingleLevel,
		lmIsotropicMipMapping,
		lmAnisotropicMipMapping,
		numLevelModes
	};

	typedef void* TImageHandle;
	typedef TResolution2D TLevel;

	virtual ~ImageCodec();

	const TImageHandle create(
			const std::string& filename, LevelMode levelMode, const TResolution2D& resolution, 
			const TRgbSpacePtr& rgbSpace, const std::string& options) const 
	{
		return doCreate(filename, levelMode, resolution, rgbSpace, options);
	}
	const TImageHandle open(
			const std::string& filename, const TRgbSpacePtr& rgbSpace, 
			const std::string& options) const 
	{
		return doOpen(filename, rgbSpace, options); 
	}
	void close(TImageHandle handle) const 
	{ 
		if (handle) doClose(handle); 
	}

	const LevelMode levelMode(TImageHandle handle) const
	{
		return doLevelMode(handle);
	}
	const TLevel levels(TImageHandle handle) const
	{
		return doLevels(handle);
	}
	const TResolution2D resolution(TImageHandle handle, const TLevel& level) const
	{
		return doResolution(handle, level);
	}
	const TRgbSpacePtr rgbSpace(TImageHandle handle) const
	{
		return doRgbSpace(handle);
	}

	void read(TImageHandle handle, const TLevel& level, const TResolution2D& begin, 
		const TResolution2D& end, XYZ* xyz, TScalar* alpha) const
	{
		doRead(handle, level, begin, end, xyz, alpha);
	}
	void write(TImageHandle handle, const TLevel& level, const TResolution2D& begin, 
		const TResolution2D& end, const XYZ* xyz, const TScalar* alpha) const
	{
		doWrite(handle, level, begin, end, xyz, alpha);
	}

private:
	virtual const TImageHandle doCreate(const std::string& filename, LevelMode levelMode, 
		const TResolution2D& resolution, const TRgbSpacePtr& rgbSpace,
		const std::string& options) const = 0;
	virtual const TImageHandle doOpen(const std::string& filename, const TRgbSpacePtr& rgbSpace,
		const std::string& options) const = 0;
	virtual void doClose(TImageHandle) const = 0;

	virtual const LevelMode doLevelMode(TImageHandle handle) const = 0;
	virtual const TLevel doLevels(TImageHandle handle) const = 0;
	virtual const TResolution2D doResolution(TImageHandle handle, const TLevel& level) const = 0;
	virtual const TRgbSpacePtr doRgbSpace(TImageHandle handle) const = 0;
	
	virtual void doRead(TImageHandle handle, const TLevel& level, const TResolution2D& begin, 
		const TResolution2D& end, XYZ* xyz, TScalar* alpha) const = 0;	
	virtual void doWrite(TImageHandle handle, const TLevel& level, const TResolution2D& begin, 
		const TResolution2D& end, const XYZ* xyz, const TScalar* alpha) const = 0;
};

typedef python::PyObjectPtr<ImageCodec>::Type TImageCodecPtr;
typedef std::map<std::string, TImageCodecPtr> TImageCodecMap;

LIAR_KERNEL_DLL TImageCodecMap& LASS_CALL imageCodecs();
LIAR_KERNEL_DLL const TImageCodecPtr& LASS_CALL imageCodec(const std::string& extension);



class LIAR_KERNEL_DLL ImageReader: util::NonCopyable
{
public:

	typedef ImageCodec::TLevel TLevel;

	ImageReader(const std::string& filename, const TRgbSpacePtr& rgbSpace, 
		const std::string& options);
	~ImageReader();

	const ImageCodec::LevelMode levelMode() const { return codec_->levelMode(handle_); }
	const TLevel levels() const { return codec_->levels(handle_); }
	const TResolution2D resolution(const TLevel& level = TLevel()) const 
	{ 
		return codec_->resolution(handle_, level); 
	}
	const TRgbSpacePtr rgbSpace() const { return codec_->rgbSpace(handle_); }

	void read(
			const TResolution2D& begin, const TResolution2D& end, 
			XYZ* xyz, TScalar* alpha) const
	{
		codec_->read(handle_, TLevel(), begin, end, xyz, alpha);
	}
	void read(
			const TLevel& level, const TResolution2D& begin, const TResolution2D& end, 
			XYZ* xyz, TScalar* alpha) const
	{
		codec_->read(handle_, level, begin, end, xyz, alpha);
	}
private:
	TImageCodecPtr codec_;
	ImageCodec::TImageHandle handle_;
};



class LIAR_KERNEL_DLL ImageWriter: util::NonCopyable
{
public:

	typedef ImageCodec::TLevel TLevel;

	ImageWriter(const std::string& filename, ImageCodec::LevelMode levelMode, 
		const TResolution2D& resolution, const TRgbSpacePtr& rgbSpace, const std::string& options);
	~ImageWriter();

	const ImageCodec::LevelMode levelMode() const { return codec_->levelMode(handle_); }
	const TLevel levels() const { return codec_->levels(handle_); }
	const TResolution2D resolution(const TLevel& level = TLevel()) const 
	{ 
		return codec_->resolution(handle_, level); 
	}
	const TRgbSpacePtr rgbSpace() const { return codec_->rgbSpace(handle_); }

	void write(
			const TResolution2D& level, const TResolution2D& begin, 
			const TResolution2D& end, const XYZ* xyz, const TScalar* alpha) const
	{
		codec_->write(handle_, level, begin, end, xyz, alpha);
	}
	void write(
			const TResolution2D& begin, const TResolution2D& end, 
			const XYZ* xyz, const TScalar* alpha) const
	{
		codec_->write(handle_, TLevel(), begin, end, xyz, alpha);
	}
private:
	TImageCodecPtr codec_;
	ImageCodec::TImageHandle handle_;
};


// built-in codecs

class LIAR_KERNEL_DLL ImageCodecLassCommon: public ImageCodec
{
private:
	virtual const TImageHandle doCreate(const std::string& filename, LevelMode levelMode, 
		const TResolution2D& resolution, const TRgbSpacePtr& rgbSpace, const std::string& options) const;
	virtual const TImageHandle doOpen(const std::string& filename, const TRgbSpacePtr& rgbSpace,
		const std::string& options) const;
	virtual void doClose(TImageHandle handle) const;

	virtual const ImageCodec::LevelMode doLevelMode(TImageHandle handle) const;
	virtual const TLevel doLevels(TImageHandle handle) const;
	virtual const TResolution2D doResolution(TImageHandle handle, const TLevel& level) const;
	virtual const TRgbSpacePtr doRgbSpace(TImageHandle handle) const;
};



class LIAR_KERNEL_DLL ImageCodecLassLDR: public ImageCodecLassCommon
{
	PY_HEADER(ImageCodec)
private:
	virtual void doRead(TImageHandle handle, const TResolution2D& level, const TResolution2D& begin, 
		const TResolution2D& end, XYZ* xyz, TScalar* alpha) const;	
	virtual void doWrite(TImageHandle handle, const TResolution2D& level, const TResolution2D& begin, 
		const TResolution2D& end, const XYZ* xyz, const TScalar* alpha) const;
};



class LIAR_KERNEL_DLL ImageCodecLassHDR: public ImageCodecLassCommon
{
	PY_HEADER(ImageCodec)
private:
	virtual void doRead(TImageHandle handle, const TResolution2D& level, const TResolution2D& begin, 
		const TResolution2D& end, XYZ* xyz, TScalar* alpha) const;	
	virtual void doWrite(TImageHandle handle, const TResolution2D& level, const TResolution2D& begin, 
		const TResolution2D& end, const XYZ* xyz, const TScalar* alpha) const;
};



}

}

#endif

// EOF
