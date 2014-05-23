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
#include "raster.h"
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


class LIAR_OUTPUT_DLL Display: public Raster, PixelToaster::Listener
{
    PY_HEADER(Raster)
public:

	Display(const std::string& title, const TResolution2D& resolution);
	~Display();

	const std::string& title() const;
	bool showHistogram() const;
	bool autoUpdate() const;

	void setShowHistogram(bool enable = true);
	void setAutoUpdate(bool enable = true);

private:

	typedef std::vector<PixelToaster::FloatingPointPixel> TDisplayBuffer;

	void doBeginRender();
	void doEndRender();
	bool doIsCanceling() const;
	
	void onKeyDown(PixelToaster::DisplayInterface& display, PixelToaster::Key key);
	void onMouseButtonDown(PixelToaster::DisplayInterface& display, PixelToaster::Mouse );
	void onMouseButtonUp(PixelToaster::DisplayInterface& display, PixelToaster::Mouse );
	bool onClose(PixelToaster::DisplayInterface& display);

	void displayLoop();
	void copyToDisplayBuffer(const TDirtyBox& box);
	void histogram();
	void cancel();
	void close();
	const std::string makeTitle() const;

	Listener listener_;
	TDisplayBuffer displayBuffer_;
	util::ScopedPtr<util::Thread> displayLoop_;
	util::Condition signal_;
	std::string title_;
	TDirtyBox displayDirtyBox_;
	TResolution2D currentResolution_;
	bool showHistogram_;
	bool wasShowingHistogram_;
	bool autoUpdate_;
	volatile bool isCanceling_;
	volatile bool isClosing_;
	TPoint2D beginDrag_;
};



}

}

#if LASS_COMPILER_TYPE == LASS_COMPILER_TYPE_MSVC
#	pragma warning(pop)
#endif

#endif

#endif

// EOF
