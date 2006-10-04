/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2006  Bram de Greve
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

	typedef prim::Vector2D<size_t> TSize;

	Display(const std::string& title, const TSize& size);
    ~Display();

	const std::string& title() const;
	const TSize& size() const;
	const TRgbSpacePtr& rgbSpace() const;
    const TScalar gamma() const;
	const TScalar exposureTime() const;

	void setRgbSpace(const TRgbSpacePtr& rgbSpace);
    void setGamma(TScalar gammaExponent);
	void setExposureTime(TScalar time);

private:

	typedef std::vector<PixelToaster::FloatingPointPixel> TBuffer;
	typedef std::vector<unsigned> TCounter;

    void doBeginRender();
	void doWriteRender(const OutputSample* first, const OutputSample* last);
    void doEndRender();
	const bool doIsCanceling() const;
		
	void onKeyPressed(PixelToaster::Key key);
	void onClose();

	void displayLoop();
	void updateDisplayBuffer();
	void waitForAnyKey();

	PixelToaster::Display display_;
	Listener listener_;
	TBuffer renderBuffer_;
	TBuffer displayBuffer_;
	TCounter numberOfSamples_;
	util::ScopedPtr<util::Thread> displayLoop_;
	util::CriticalSection mutex_;
	util::Condition signal_;
	std::string title_;
    TSize size_;
	TRgbSpacePtr rgbSpace_;
    TScalar gamma_;
	TScalar exposureTime_;
	bool isQuiting_;
	bool isDirty_;
	bool isCanceling_;
	bool isAnyKeyed_;
};



}

}

#if LASS_COMPILER_TYPE == LASS_COMPILER_TYPE_MSVC
#	pragma warning(pop)
#endif

#endif

// EOF
