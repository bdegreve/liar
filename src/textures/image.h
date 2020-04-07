/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2020  Bram de Greve (bramz@users.sourceforge.net)
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

	explicit Image(const std::wstring& filename);
	Image(const std::wstring& filename, const TRgbSpacePtr& rgbSpace);
	Image(const std::wstring& filename, const std::string& antiAliasing, 
		const std::string& mipMapping);
	Image(const std::wstring& filename, const std::string& antiAliasing, 
		const std::string& mipMapping, const TRgbSpacePtr& rgbSpace);

	void loadFile(const std::wstring& filename);
	void loadFile(const std::wstring& filename, const TRgbSpacePtr& rgbSpace);

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

	typedef XYZ TPixel;
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
		TPixel& operator()(size_t x, size_t y)
		{ 
			return pixels_[y * resolution_.x + x]; 
		}
		const TPixel& operator()(size_t x, size_t y) const 
		{ 
			return pixels_[y * resolution_.x + x]; 
		}
		const TResolution2D& resolution() const { return resolution_; }
	private:
		TPixels pixels_;
		TResolution2D resolution_;
	};		
	typedef std::vector<MipMapLevel> TMipMaps;

	const Spectral doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const override;
	TValue doScalarLookUp(const Sample& sample, const IntersectionContext& context) const override;
	bool doIsChromatic() const override;

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

	void makeMipMaps(MipMapping mode) const;
	MipMapLevel makeMipMap(const MipMapLevel& parent, prim::XY compressionAxis, size_t newSize) const;
	MipMapLevel makeMipMapEven(const MipMapLevel& parent, prim::XY compressionAxis, size_t newSize) const;
	MipMapLevel makeMipMapOdd(const MipMapLevel& parent, prim::XY compressionAxis, size_t newSize) const;

	void mipMapLevel(TScalar width, size_t numLevels, 
		size_t& level0, size_t& level1, TPixel::TValue& dLevel) const;

	const TPixel lookUp(const IntersectionContext& context) const;
	const TPixel nearest(size_t levelU, size_t levelV, const TPoint2D& uv) const;
	const TPixel bilinear(size_t levelU, size_t levelV, const TPoint2D& uv) const;

	static TAntiAliasingDictionary makeAntiAliasingDictionary();
	static TMipMappingDictionary makeMipMappingDictionary();

	std::wstring filename_;
	TRgbSpacePtr rgbSpace_;
	TPixels image_;
	TResolution2D resolution_;
	AntiAliasing antiAliasing_;
	MipMapping mipMapping_;
	mutable MipMapping currentMipMapping_;
	mutable TMipMaps mipMaps_;
	mutable size_t numLevelsU_;
	mutable size_t numLevelsV_;
	mutable util::Semaphore mutex_;

	static TAntiAliasingDictionary antiAliasingDictionary_;
	static TMipMappingDictionary mipMappingDictionary_;
	static AntiAliasing defaultAntiAliasing_;
	static MipMapping defaultMipMapping_;
};

}

}

#endif

// EOF
