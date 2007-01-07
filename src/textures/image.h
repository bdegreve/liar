/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
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
 *  http://liar.sourceforge.net
 */

/** @class liar::textures::Image
 *  @brief texture using image file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_IMAGE_H
#define LIAR_GUARDIAN_OF_INCLUSION_TEXTURES_IMAGE_H

#include "textures_common.h"
#include "../kernel/texture.h"
#include <lass/io/image.h>
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
	Image(const std::string& filename, const std::string& antiAliasing, 
		const std::string& mipMapping);

	const std::string antiAliasing() const;
	const std::string mipMapping() const;
	void setAntiAliasing(const std::string& mode);
	void setMipMapping(const std::string& mode);

	void loadFile(const std::string& filename);

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

	enum MipMapping
	{
		mmNone = 0,
		mmIsotropic,
		mmAnisotropic,
		numMipMapping,
		mmUninitialized = numMipMapping
	};

	typedef util::SharedPtr<io::Image> TImagePtr;
	typedef std::vector<TImagePtr> THorizontalMipMaps;
	typedef std::vector<THorizontalMipMaps> TMipMaps;
	typedef util::Dictionary<std::string, AntiAliasing> TAntiAliasingDictionary;
	typedef util::Dictionary<std::string, MipMapping> TMipMappingDictionary;

	const Spectrum doLookUp(const Sample& sample, 
		const IntersectionContext& context) const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	void makeMipMaps(MipMapping mode) const;
	TImagePtr makeMipMap(const TImagePtr iOldImagePtr, prim::XY iCompressionAxis) const;
	TImagePtr makeMipMapEven(const TImagePtr iOldImagePtr, prim::XY iCompressionAxis) const;
	TImagePtr makeMipMapOdd(const TImagePtr iOldImagePtr, prim::XY iCompressionAxis) const;

	void mipMapLevel(TScalar iWidth, size_t iNumLevels, 
		size_t& oLevel0, size_t& oLevel1, TScalar& oDLevel) const;

	io::Image::TPixel nearest(size_t iLevelU, size_t iLevelV, const TPoint2D& uv) const;
	io::Image::TPixel bilinear(size_t iLevelU, size_t iLevelV, const TPoint2D& uv) const;

	static TAntiAliasingDictionary makeAntiAliasingDictionary();
	static TMipMappingDictionary makeMipMappingDictionary();

	TImagePtr image_;
	AntiAliasing antiAliasing_;
	MipMapping mipMapping_;
	mutable MipMapping currentMipMapping_;
	mutable TMipMaps mipMaps_;
	mutable size_t numLevelsU_;
	mutable size_t numLevelsV_;
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
