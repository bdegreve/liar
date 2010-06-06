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

/** @class liar::output::Image
 *  @brief render target to a PixelToaster display
 *  @author Bram de Greve [Bramz]
 *
 *  http://www.pixeltoaster.com
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_OUTPUT_DISPLAY_H
#define LIAR_GUARDIAN_OF_INCLUSION_OUTPUT_DISPLAY_H

#include "output_common.h"
#include "../kernel/render_target.h"
#include "../kernel/rgb_space.h"
#include <lass/prim/aabb_2d.h>
#include <lass/stde/extended_string.h>
#include <lass/util/dictionary.h>
#include <lass/util/scoped_ptr.h>
#include <lass/util/thread.h>

#if LIAR_OUTPUT_HAVE_PIXELTOASTER_H

#include <PixelToaster.h>

#if LASS_COMPILER_TYPE == LASS_COMPILER_TYPE_MSVC
#	pragma warning(push)
#	pragma warning(disable: 4275) // non dll-interface class used as base for dll-interface class
#endif

namespace liar
{
namespace output
{


class LIAR_OUTPUT_DLL Display: public RenderTarget, PixelToaster::Listener
{
    PY_HEADER(RenderTarget)
public:

	Display(const std::string& title, const TResolution2D& resolution);
	~Display();

	const std::string& title() const;
	const TRgbSpacePtr& rgbSpace() const;
	const std::string toneMapping() const;
	TScalar exposure() const;
	TScalar exposureCorrection() const;
	bool autoExposure() const;
	TScalar middleGrey() const;

	void setRgbSpace(const TRgbSpacePtr& rgbSpace);
	void setToneMapping(const std::string& mode);
	void setExposure(TScalar fStops);
	void setExposureCorrection(TScalar stops);
	void setAutoExposure(bool enable = true);
	void setMiddleGrey(TScalar grey);

	void testGammut();

private:

	typedef std::vector<XYZ> TRenderBuffer;
	typedef std::vector<TScalar> TWeightBuffer;
	typedef std::vector<PixelToaster::FloatingPointPixel> TDisplayBuffer;
	typedef prim::Aabb2D<size_t> TDirtyBox;

	enum ToneMapping
	{
		tmLinear = 0,
		tmCompressY,
		tmCompressXYZ,
		tmReinhard2002Y,
		tmExponentialY,
		tmExponentialXYZ,
		numToneMapping
	};
	typedef util::Dictionary<std::string, ToneMapping> TToneMappingDictionary;

	const TResolution2D doResolution() const;
	void doBeginRender();
	void doWriteRender(const OutputSample* first, const OutputSample* last);
	void doEndRender();
	bool doIsCanceling() const;
		
	void onKeyDown(PixelToaster::DisplayInterface& display, PixelToaster::Key key);
	bool onClose(PixelToaster::DisplayInterface& display);

	void displayLoop();
	void copyToDisplayBuffer();
	const std::string makeTitle() const;

	static TToneMappingDictionary makeToneMappingDictionary();

	Listener listener_;
	TRenderBuffer renderBuffer_;
	TWeightBuffer totalWeight_;
	TDisplayBuffer displayBuffer_;
	util::ScopedPtr<util::Thread> displayLoop_;
	util::CriticalSection renderBufferLock_;
	util::Condition signal_;
	std::string title_;
	TDirtyBox renderDirtyBox_;
	TDirtyBox displayDirtyBox_;
	TDirtyBox allTimeDirtyBox_;
	TResolution2D resolution_;
	TRgbSpacePtr rgbSpace_;
	TScalar totalLogSceneLuminance_;
	TScalar maxSceneLuminance_;
	int sceneLuminanceCoverage_;
	TScalar gain_;
	TScalar middleGrey_;
	int exposure_;
	int exposureCorrection_;
	ToneMapping toneMapping_;
	bool autoExposure_;
	bool refreshTitle_;
	volatile bool isQuiting_;
	volatile bool isCanceling_;

	static TToneMappingDictionary toneMappingDictionary_;
};



}

}

#if LASS_COMPILER_TYPE == LASS_COMPILER_TYPE_MSVC
#	pragma warning(pop)
#endif

#endif

#endif

// EOF
