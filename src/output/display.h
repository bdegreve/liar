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
#include <lass/util/scoped_ptr.h>
#include <lass/util/thread.h>

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

	Display(const std::string& title, const TResolution& resolution);
	~Display();

	const std::string& title() const;
	const TRgbSpacePtr& rgbSpace() const;
	const TScalar exposure() const;
	const TScalar fStops() const;
	const TScalar gamma() const;
	const TScalar gain() const;

	void setRgbSpace(const TRgbSpacePtr& rgbSpace);
	void setExposure(TScalar exposure);
	void setFStops(TScalar fStops);
	void setGamma(TScalar gammaExponent);
	void setGain(TScalar gain);

private:

	typedef std::vector<PixelToaster::FloatingPointPixel> TBuffer;
	typedef prim::Aabb2D<unsigned> TDirtyBox;

	const TResolution doResolution() const;
	void doBeginRender();
	void doWriteRender(const OutputSample* first, const OutputSample* last);
	void doEndRender();
	const bool doIsCanceling() const;
		
	void onKeyDown(PixelToaster::DisplayInterface& display, PixelToaster::Key key);
	bool onClose(PixelToaster::DisplayInterface& display);

	void displayLoop();
	void copyToDisplayBuffer();
	void shadeDisplayBuffer();
	void waitForAnyKey();

	PixelToaster::Display display_;
	Listener listener_;
	TBuffer renderBuffer_;
	TBuffer displayBuffer_;
	std::vector<TScalar> totalWeight_;
	util::ScopedPtr<util::Thread> displayLoop_;
	util::CriticalSection renderBufferLock_;
	util::Condition signal_;
	std::string title_;
	TDirtyBox renderDirtyBox_;
	TDirtyBox displayDirtyBox_;
	TResolution resolution_;
	TRgbSpacePtr rgbSpace_;
	TScalar exposure_;
	TScalar gamma_;
	TScalar gain_;
	volatile bool isQuiting_;
	volatile bool isCanceling_;
	volatile bool isAnyKeyed_;
};



}

}

#if LASS_COMPILER_TYPE == LASS_COMPILER_TYPE_MSVC
#	pragma warning(pop)
#endif

#endif

// EOF
