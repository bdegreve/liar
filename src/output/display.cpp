/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2023  Bram de Greve (bramz@users.sourceforge.net)
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

#include "output_common.h"

#if LIAR_OUTPUT_HAVE_PIXELTOASTER_H

#include "display.h"
#include <lass/util/callback_0.h>
#include <lass/util/thread_fun.h>
#include <lass/num/num_cast.h>

namespace liar
{
namespace output
{

PY_DECLARE_CLASS_DOC(Display, "Render target in a window"
	"\n"
	"---\n"
	"\n"
	"This software is based in part on the PixelToaster library\n"
	"(http://code.google.com/p/pixeltoaster/):\n"
	"\n"
	"PixelToaster Framebuffer Library.\n"
	"\n"
	"Copyright (c) 2004-2007 Glenn Fiedler\n"
	"\n"
	"This software is provided 'as-is', without any express or implied\n"
	"warranty. In no event will the authors be held liable for any damages\n"
	"arising from the use of this software.\n"
	"\n"
	"Permission is granted to anyone to use this software for any purpose,\n"
	"including commercial applications, and to alter it and redistribute it\n"
	"freely, subject to the following restrictions:\n"
	"\n"
	"1. The origin of this software must not be misrepresented; you must not\n"
	"   claim that you wrote the original software. If you use this software\n"
	"   in a product, an acknowledgment in the product documentation would be\n"
	"   appreciated but is not required.\n"
	"\n"
	"2. Altered source versions must be plainly marked as such, and must not be\n"
	"   misrepresented as being the original software.\n"
	"\n"
	"3. This notice may not be removed or altered from any source distribution.\n"
	"\n"
	"Glenn Fiedler\n"
	"gaffer@gaffer.org\n"
	);
PY_CLASS_CONSTRUCTOR_2(Display, const std::string&, const TResolution2D&)
PY_CLASS_MEMBER_R(Display, title)
PY_CLASS_MEMBER_RW(Display, autoUpdate, setAutoUpdate)
PY_CLASS_MEMBER_RW(Display, showHistogram, setShowHistogram);


// --- public --------------------------------------------------------------------------------------

Display::Display(const std::string& title, const TResolution2D& resolution):
	Raster(resolution),
	title_(title),
	currentResolution_(0, 0),
	showHistogram_(false),
	wasShowingHistogram_(false),
	autoUpdate_(true),
	isCanceling_(false),
	isClosing_(false)
{
}



Display::~Display()
{
	if (displayLoop_)
	{
		displayLoop_->join();
	}
}



const std::string& Display::title() const
{
	return title_;
}



bool Display::autoUpdate() const
{
	return autoUpdate_;
}



bool Display::showHistogram() const
{
	return showHistogram_;
}



void Display::setAutoUpdate(bool enable)
{
	autoUpdate_ = enable;
}



void Display::setShowHistogram(bool enable)
{
	showHistogram_ = enable;
}



// --- private -------------------------------------------------------------------------------------

void Display::doBeginRender()
{
	if (displayLoop_ && currentResolution_ != resolution())
	{
		close();
	}

	beginRaster();

	if (!displayLoop_)
	{
		const size_t n = resolution().x * resolution().y;
		displayBuffer_.assign(n, PixelToaster::FloatingPointPixel());
		displayLoop_.reset(
			util::threadFun(util::makeCallback(this, &Display::displayLoop), util::threadJoinable));
		displayLoop_->run();
		currentResolution_ = resolution();
	}
}



void Display::doEndRender()
{
	signal_.signal();
}



bool Display::doIsCanceling() const
{
	return isCanceling_;
}



void Display::onKeyDown(PixelToaster::DisplayInterface& display, PixelToaster::Key key)
{
	const TValue stopDivisions = 3.f; // third of stop
	TValue exposureStep = num::inv(stopDivisions);

	using namespace PixelToaster;
	switch (key)
	{
	case Key::Escape:
		onClose(display);
		return;

	case Key::A:
		setAutoExposure(!autoExposure());
		break;

	case Key::E:
		nextToneMapping();
		break;

	case Key::H:
		setShowHistogram(!showHistogram_);
		break;

	case Key::Separator:
	case Key::Subtract:
		exposureStep = -exposureStep;
		[[fallthrough]];
	case Key::Equals:
	case Key::Add:
		{
			TValue stops = autoExposure() ? exposureCorrectionStops() : exposureStops();
			stops += exposureStep;
			stops = num::round(stopDivisions * stops) / stopDivisions; // round to nearest stop step
			if (autoExposure())
			{
				setExposureCorrectionStops(stops);
			}
			else
			{
				setExposureStops(stops);
			}
		}
		break;

	default:
		break;
	}

	//isAnyKeyed_ = true;
	signal_.signal();
}



void Display::onMouseButtonDown(PixelToaster::DisplayInterface&, PixelToaster::Mouse mouse)
{
    if ( mouse.buttons.left )
    {
        beginDrag_.x = mouse.x / static_cast<TScalar>(resolution().x);
        beginDrag_.y = mouse.y / static_cast<TScalar>(resolution().y);
    }
    else
    {
        beginDrag_.x = TNumTraits::qNaN;
    }
}



void Display::onMouseButtonUp(PixelToaster::DisplayInterface&, PixelToaster::Mouse mouse)
{
    if ( !num::isNaN( beginDrag_.x ) )
    {
        TPoint2D endDrag(
            (mouse.x + 1) / static_cast<TScalar>(resolution().x),
            (mouse.y + 1) / static_cast<TScalar>(resolution().y)
        );
        LASS_CERR << "\nselected area: (" << beginDrag_ << ", " << endDrag << ")\n";
    }
}



bool Display::onClose(PixelToaster::DisplayInterface&)
{
	cancel();
	return false;
}


void Display::cancel()
{
	isCanceling_ = true;
	isClosing_ = true;
}


void Display::close()
{
	isClosing_ = true;
	signal_.signal();
	displayLoop_->join();
}

namespace
{

typedef Display::TValue TValue;

void printThirdStops(std::ostringstream& stream, TValue stops)
{
	const char sign = stops >= 0 ? '+' : '-';
	const int thirds = static_cast<int>(num::round(num::abs(3 * stops)));
	const int i = thirds / 3;
	const int d = thirds % 3;
	if (i)
	{
		stream << " " << sign << i;
		if (d)
		{
			stream << " " << d << "/3";
		}
	}
	else if (d)
	{
		stream << " " << sign << d << "/3";
	}
	else
	{
		stream << " +0";
	}
}

std::string toString(Display::ToneMapping toneMapping)
{
	auto& enumDef = lass::python::PyExportTraits<Display::ToneMapping>::enumDefinition;
	auto val = enumDef.getValue(toneMapping);
	return val ? *val : "unknown";
}

}

const std::string Display::makeTitle() const
{
	std::ostringstream buffer;
	buffer << title_ << " [" << toString(toneMapping());
	if (autoExposure())
	{
		printThirdStops(buffer, exposureStops());
		buffer << " A";
		printThirdStops(buffer, exposureCorrectionStops());
	}
	else
	{
		printThirdStops(buffer, exposureStops() + exposureCorrectionStops());
	}
	buffer << "]";
	return buffer.str();
}



void Display::displayLoop()
{
	isCanceling_ = false;
	isClosing_ = false;

	PixelToaster::Display display;
	const int width = num::numCast<int>(resolution().x);
	const int height = num::numCast<int>(resolution().y);
 	LASS_ENFORCE(display.open(makeTitle().c_str(), width, height, PixelToaster::Output::Windowed, PixelToaster::Mode::TrueColor));
	display.listener(this);

	LASS_ENFORCE(display.update(displayBuffer_)); // full update of initial buffer

	while (!isClosing_)
	{
		if (autoUpdate_)
		{
			displayDirtyBox_ = tonemap(rgbSpace());
			copyToDisplayBuffer(displayDirtyBox_);
			histogram();
			display.title(makeTitle().c_str());
		}

		PixelToaster::Rectangle box;
		if (!displayDirtyBox_.isEmpty())
		{
			box.xBegin = num::numCast<int>(displayDirtyBox_.min().x);
			box.yBegin = num::numCast<int>(displayDirtyBox_.min().y);
			box.xEnd = num::numCast<int>(displayDirtyBox_.max().x + 1);
			box.yEnd = num::numCast<int>(displayDirtyBox_.max().y + 1);
		}
		LASS_ENFORCE(display.update(displayBuffer_, &box));

		displayDirtyBox_.clear();
		signal_.wait(2000);
	}

	display.close();
}


/** @internal
 */
void Display::copyToDisplayBuffer(const TDirtyBox& box)
{
    const TTonemapBuffer& tonemapped = tonemapBuffer();
    const TDirtyBox::TPoint min = box.min();
    const TDirtyBox::TPoint max = box.max();
    const size_t nx = max.x - min.x + 1;
    for (size_t j = min.y; j <= max.y; ++j)
    {
        const size_t kBegin = j * resolution().x + min.x;
        const size_t kEnd = kBegin + nx;
        for (size_t k = kBegin; k < kEnd; ++k)
        {
            const prim::ColorRGBA& pIn = tonemapped[k];
            PixelToaster::FloatingPointPixel& pOut = displayBuffer_[k];
            pOut.r = pIn.r;
            pOut.g = pIn.g;
            pOut.b = pIn.b;
            pOut.a = pIn.a;
        }
    }
}


namespace
{
    size_t bucket( TScalar x, size_t maxBucket )
    {
        return std::min(static_cast<size_t>( std::max(num::round(x * static_cast<TScalar>(maxBucket)), TScalar(0)) ), maxBucket );
    }
}


void Display::histogram()
{
    const size_t displayWidth = resolution().x;
    const size_t displayHeight = resolution().y;

    const size_t numBuckets = displayWidth / 4;
    const size_t histoHeight = displayHeight / 5;

    // overwrite area of previous histo with real image, so we can recompute
    const TDirtyBox histoBox(TDirtyBox::TPoint( displayWidth - numBuckets - 1, 0 ), TDirtyBox::TPoint( displayWidth - 1, histoHeight ));

    if (wasShowingHistogram_)
    {
        copyToDisplayBuffer(histoBox);
        wasShowingHistogram_ = false;
    }

    if (!showHistogram_)
    {
        return;
    }

    typedef prim::Vector3D<size_t> THistoSample;
    std::vector<THistoSample> histo(numBuckets);
    const size_t maxBucket = numBuckets - 1;
    const TValueBuffer& w = totalWeight();
    const TTonemapBuffer& tonemapped = tonemapBuffer();
    for (size_t k = 0, n = tonemapped.size(); k < n; ++k)
    {
        if (w[k] <= 0)
        {
            continue;
        }
        const prim::ColorRGBA& p = tonemapped[k];
        histo[bucket(p.r, maxBucket)].x += 1;
        histo[bucket(p.g, maxBucket)].y += 1;
        histo[bucket(p.b, maxBucket)].z += 1;
    }
    size_t max = 0;
    for (size_t k = 1; k < numBuckets; ++k)
    {
        max = std::max(histo[k].x, max);
        max = std::max(histo[k].y, max);
        max = std::max(histo[k].z, max);
    }
    const TScalar scale = max > 0 ? TScalar(histoHeight) / TScalar(max) : TScalar(0);
    for (size_t k = 0; k < numBuckets; ++k)
    {
        histo[k].x = static_cast<size_t>(static_cast<TScalar>(histo[k].x) * scale);
        histo[k].y = static_cast<size_t>(static_cast<TScalar>(histo[k].y) * scale);
        histo[k].z = static_cast<size_t>(static_cast<TScalar>(histo[k].z) * scale);
    }
    for (size_t j = 0; j < histoHeight; ++j)
    {
        const size_t kBegin = j * displayWidth + histoBox.min().x;
        const size_t threshold = histoHeight - j;
        displayBuffer_[kBegin] = PixelToaster::FloatingPointPixel(.25f, .25f, .25f, 1);
        for (size_t k = 0; k < numBuckets; ++k)
        {
            PixelToaster::FloatingPointPixel& p = displayBuffer_[kBegin + k + 1];
            p.a = 1;
            p.b = histo[k].z >= threshold ? 1.f : p.b * .25f;
            p.g = histo[k].y >= threshold ? 1.f : p.g * .25f;
            p.r = histo[k].x >= threshold ? 1.f : p.r * .25f;
        }
    }
    const size_t kBegin = histoHeight * displayWidth + histoBox.min().x;
    for (size_t k = 0; k <= numBuckets; ++k)
    {
        displayBuffer_[kBegin + k] = PixelToaster::FloatingPointPixel(.25f, .25f, .25f, 1);
    }

    displayDirtyBox_ += histoBox;
    wasShowingHistogram_ = true;
}


// --- free ----------------------------------------------------------------------------------------



}

}

#endif

// EOF
