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

/** @class liar::textures::Image
 *  @brief texture using image file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_IMAGE_H
#define LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_IMAGE_H

#include "textures_common.h"
#include "../kernel/texture.h"
#include "../kernel/rgb_space.h"
#include <lass/prim/xy.h>
#include <lass/util/dictionary.h>
#include <lass/util/thread.h>

namespace liar
{
namespace textures
{

class LIAR_TEXTURES_DLL Image: public Texture
{
	PY_HEADER(Texture)
public:

	explicit Image(const std::string& filename);
	Image(const std::string& filename, const TRgbSpacePtr& rgbSpace);
	Image(const std::string& filename, const std::string& antiAliasing, 
		const std::string& mipMapping);
	Image(const std::string& filename, const std::string& antiAliasing, 
		const std::string& mipMapping, const TRgbSpacePtr& rgbSpace);

	void loadFile(const std::string& filename);
	void loadFile(const std::string& filename, const TRgbSpacePtr& rgbSpace);

	const TResolution2D& resolution() const;

	const std::string antiAliasing() const;
	const std::string mipMapping() const;
	void setAntiAliasing(const std::string& mode);
	void setMipMapping(const std::string& mode);

	static void setDefaultAntiAliasing(const std::string& mode);
	static void setDefaultMipMapping(const std::string& mode);

private:

	enum AntiAliasing
	{
		aaNone = 0,
		aaBilinear,
		aaTrilinear,
		numAntiAliasing
	};
	typedef util::Dictionary<std::string, AntiAliasing> TAntiAliasingDictionary;

	enum MipMapping
	{
		mmNone = 0,
		mmIsotropic,
		mmAnisotropic,
		numMipMapping,
		mmUninitialized = numMipMapping
	};
	typedef util::Dictionary<std::string, MipMapping> TMipMappingDictionary;

	typedef TVector3D TPixel;
	typedef util::SharedPtr<TPixel, util::ArrayStorage> TPixels;

	class MipMapLevel
	{
	public:
		MipMapLevel(const TPixels& pixels, const TResolution2D& resolution): 
			pixels_(pixels), 
			resolution_(resolution) 
		{
		}
		MipMapLevel(const TResolution2D& resolution):
			pixels_(new TPixel[resolution.x * resolution.y]),
			resolution_(resolution)
		{
		}
		TPixel& operator()(unsigned x, unsigned y)
		{ 
			return pixels_[y * resolution_.x + x]; 
		}
		const TPixel& operator()(unsigned x, unsigned y) const 
		{ 
			return pixels_[y * resolution_.x + x]; 
		}
		const TResolution2D& resolution() const { return resolution_; }
	private:
		TPixels pixels_;
		TResolution2D resolution_;
	};		
	typedef std::vector<MipMapLevel> TMipMaps;

	const Spectrum doLookUp(const Sample& sample, 
		const IntersectionContext& context) const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	void makeMipMaps(MipMapping mode) const;
	MipMapLevel makeMipMap(const MipMapLevel& parent, prim::XY compressionAxis, unsigned newSize) const;
	MipMapLevel makeMipMapEven(const MipMapLevel& parent, prim::XY compressionAxis, unsigned newSize) const;
	MipMapLevel makeMipMapOdd(const MipMapLevel& parent, prim::XY compressionAxis, unsigned newSize) const;

	void mipMapLevel(TScalar width, size_t numLevels, 
		unsigned& level0, unsigned& level1, TScalar& dLevel) const;

	const TPixel nearest(unsigned levelU, unsigned levelV, const TPoint2D& uv) const;
	const TPixel bilinear(unsigned levelU, unsigned levelV, const TPoint2D& uv) const;

	static TAntiAliasingDictionary makeAntiAliasingDictionary();
	static TMipMappingDictionary makeMipMappingDictionary();

	std::string filename_;
	TRgbSpacePtr rgbSpace_;
	TPixels image_;
	TResolution2D resolution_;
	AntiAliasing antiAliasing_;
	MipMapping mipMapping_;
	mutable MipMapping currentMipMapping_;
	mutable TMipMaps mipMaps_;
	mutable unsigned numLevelsU_;
	mutable unsigned numLevelsV_;
	mutable util::CriticalSection mutex_;

	static TAntiAliasingDictionary antiAliasingDictionary_;
	static TMipMappingDictionary mipMappingDictionary_;
	static AntiAliasing defaultAntiAliasing_;
	static MipMapping defaultMipMapping_;
};

}

}

#endif

// EOF
