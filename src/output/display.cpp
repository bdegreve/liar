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
 */

#include "output_common.h"
#include "display.h"
#include <lass/util/callback_0.h>
#include <lass/util/thread_fun.h>

namespace liar
{
namespace output
{

PY_DECLARE_CLASS(Display);
PY_CLASS_CONSTRUCTOR_2(Display, const std::string&, const Display::TResolution&)
PY_CLASS_MEMBER_R(Display, "title", title)
PY_CLASS_MEMBER_RW(Display, "rgbSpace", rgbSpace, setRgbSpace)
PY_CLASS_MEMBER_RW(Display, "exposure", exposure, setExposure)
PY_CLASS_MEMBER_RW(Display, "fStops", fStops, setFStops)
PY_CLASS_MEMBER_RW(Display, "gamma", gamma, setGamma)
PY_CLASS_MEMBER_RW(Display, "gain", gain, setGain)



// --- public --------------------------------------------------------------------------------------

Display::Display(const std::string& title, const TResolution& resolution):
	display_(),
	title_(title),
    resolution_(resolution),
	rgbSpace_(RgbSpace::defaultSpace()),
	exposure_(1.f),
    gamma_(2.2f),
	gain_(1.f),
    isQuiting_(false),
	isCanceling_(false),
	isAnyKeyed_(false)
{
}



Display::~Display()
{
    if (!isQuiting_)
    {
        endRender();
    }
}



const std::string& Display::title() const
{
	return title_;
}



const TRgbSpacePtr& Display::rgbSpace() const
{
	return rgbSpace_;
}



const TScalar Display::exposure() const
{
    return exposure_;
}



const TScalar Display::fStops() const
{
	return num::log2(exposure_);
}



const TScalar Display::gamma() const
{
    return gamma_;
}



const TScalar Display::gain() const
{
    return gain_;
}



void Display::setRgbSpace(const TRgbSpacePtr& rgbSpace)
{
	rgbSpace_ = rgbSpace;
}



void Display::setExposure(TScalar exposure)
{
    exposure_ = exposure;
}



void Display::setFStops(TScalar fStops)
{
	exposure_ = num::pow(TScalar(2), fStops);
}



void Display::setGamma(TScalar gammaExponent)
{
    gamma_ = gammaExponent;
}



void Display::setGain(TScalar gain)
{
    gain_ = gain;
}



// --- private -------------------------------------------------------------------------------------

const Display::TResolution Display::doResolution() const
{
	return resolution_;
}



void Display::doBeginRender()
{
	LASS_LOCK(renderBufferLock_)
	{
		const unsigned n = resolution_.x * resolution_.y;
		renderBuffer_.clear();
		renderBuffer_.resize(n);
		displayBuffer_.clear();
		displayBuffer_.resize(n);
		totalWeight_.clear();
		totalWeight_.resize(n, 0);
		renderDirtyBox_.clear();
		displayDirtyBox_.clear();
		isQuiting_ = false;
		isCanceling_ = false;
		isAnyKeyed_ = false;
	}
	
	displayLoop_.reset(
		util::threadFun(util::makeCallback(this, &Display::displayLoop), util::threadJoinable));
	displayLoop_->run();
}



void Display::doWriteRender(const OutputSample* first, const OutputSample* last)
{
    LASS_ASSERT(resolution_.x > 0 && resolution_.y > 0);

	LASS_LOCK(renderBufferLock_)
	{
		while (first != last)
		{
			const TPoint2D& position = first->screenCoordinate();
			const int i = static_cast<int>(num::floor(position.x * resolution_.x));
			const int j = static_cast<int>(num::floor(position.y * resolution_.y));
			if (i >= 0 && i < resolution_.x && j >= 0 && j < resolution_.y)
			{
				const TVector3D xyz = first->radiance().xyz();
				prim::ColorRGBA color = rgbSpace_->convert(xyz);
				color *= first->alpha() * first->weight();
				
				PixelToaster::FloatingPointPixel& pixel = renderBuffer_[j * resolution_.x + i];
				pixel.a += color.a;
				pixel.b += color.b;
				pixel.g += color.g;
				pixel.r += color.r;
				
				totalWeight_[j * resolution_.x + i] += first->weight();
				renderDirtyBox_ += TDirtyBox::TPoint(i, j);
			}
			++first;
		}
	}
}



void Display::doEndRender()
{
	isQuiting_ = true;
	displayLoop_->join();
}



const bool Display::doIsCanceling() const
{
	return isCanceling_;
}



void Display::onKeyDown(PixelToaster::DisplayInterface& display, PixelToaster::Key key)
{
	if (key == PixelToaster::Key::Escape)
	{
		onClose(display);
		return;
	}

	isAnyKeyed_ = true;
	signal_.signal();
}



bool Display::onClose(PixelToaster::DisplayInterface& display)
{
	isCanceling_ = true;
	signal_.signal();
	return false;
}



void Display::displayLoop()
{
	LASS_ENFORCE(display_.open(
		title_.c_str(), resolution_.x, resolution_.y, PixelToaster::Output::Windowed,
		PixelToaster::Mode::FloatingPoint));
	display_.listener(this);

	bool isRunning = true;
	while (isRunning)
	{
		LASS_LOCK(renderBufferLock_)
		{
			if (isQuiting_ || isCanceling_)
			{
				isRunning = false;
			}
			copyToDisplayBuffer();
		}

		if (isRunning)
		{
			shadeDisplayBuffer();
			LASS_ENFORCE(display_.update(displayBuffer_));
			signal_.wait(250);
		}
	}

	waitForAnyKey();

	display_.close();
}


/** @internal
 */
void Display::copyToDisplayBuffer()
{
	if (renderDirtyBox_.isEmpty())
	{
		return;
	}

	const TDirtyBox::TPoint min = renderDirtyBox_.min();
	const TDirtyBox::TPoint max = renderDirtyBox_.max();
	const unsigned nx = max.x - min.x + 1;
	for (unsigned j = min.y; j <= max.y; ++j)
	{
		const unsigned kBegin = j * resolution_.x + min.x;
		const unsigned kEnd = kBegin + nx;
		for (unsigned k = kBegin; k < kEnd; ++k)
		{
			displayBuffer_[k] = renderBuffer_[k];
			const TScalar w = totalWeight_[k];
			if (w > 0)
			{
				const TScalar invW = num::inv(w);
				PixelToaster::FloatingPointPixel& p = displayBuffer_[k];
				p.a *= invW;
				p.r *= invW;
				p.g *= invW;
				p.b *= invW;
			}
		}
	}

	displayDirtyBox_ = renderDirtyBox_;
	renderDirtyBox_.clear();
}	



/** @internal
 */
void Display::shadeDisplayBuffer()
{
	if (displayDirtyBox_.isEmpty())
	{
		return;
	}

	const TDirtyBox::TPoint min = displayDirtyBox_.min();
	const TDirtyBox::TPoint max = displayDirtyBox_.max();
	const unsigned nx = max.x - min.x + 1;
	const TScalar invGamma = num::inv(gamma_);

	if (exposure_ > 0)
	{
		for (unsigned j = min.y; j <= max.y; ++j)
		{
			const unsigned kBegin = j * resolution_.x + min.x;
			const unsigned kEnd = kBegin + nx;
			for (unsigned k = kBegin; k < kEnd; ++k)
			{
				PixelToaster::FloatingPointPixel& p = displayBuffer_[k];
				p.a = num::clamp(p.a, 0.f, 1.f);
				p.b = num::pow(num::clamp(gain_ * (1.f - num::exp(-exposure_ * p.b)), 0.f, 1.f), invGamma);
				p.g = num::pow(num::clamp(gain_ * (1.f - num::exp(-exposure_ * p.g)), 0.f, 1.f), invGamma);
				p.r = num::pow(num::clamp(gain_ * (1.f - num::exp(-exposure_ * p.r)), 0.f, 1.f), invGamma);
			}
		}
	}
	else
	{
		for (unsigned j = min.y; j <= max.y; ++j)
		{
			const unsigned kBegin = j * resolution_.x + min.x;
			const unsigned kEnd = kBegin + nx;
			for (unsigned k = kBegin; k < kEnd; ++k)
			{
				PixelToaster::FloatingPointPixel& p = displayBuffer_[k];
				p.a = num::clamp(p.a, 0.f, 1.f);
				p.b = num::pow(num::clamp(gain_ * p.b, 0.f, 1.f), invGamma);
				p.g = num::pow(num::clamp(gain_ * p.g, 0.f, 1.f), invGamma);
				p.r = num::pow(num::clamp(gain_ * p.r, 0.f, 1.f), invGamma);
			}
		}
	}

	displayDirtyBox_.clear();
}



void Display::waitForAnyKey()
{
	isAnyKeyed_ = false;
	while (true)
	{
		if (isAnyKeyed_ || isCanceling_)
		{
			return;
		}
		shadeDisplayBuffer();
		LASS_ENFORCE(display_.update(displayBuffer_));
		signal_.wait(250);
	}
}



// --- Display::Listener ---------------------------------------------------------------------------




// --- free ----------------------------------------------------------------------------------------



}

}

// EOF