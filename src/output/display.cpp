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
PY_CLASS_CONSTRUCTOR_2(Display, const std::string&, Display::TSize)
PY_CLASS_MEMBER_R(Display, "size", size)
PY_CLASS_MEMBER_R(Display, "title", title)
PY_CLASS_MEMBER_RW(Display, "rgbSpace", rgbSpace, setRgbSpace)
PY_CLASS_MEMBER_RW(Display, "gamma", gamma, setGamma)
PY_CLASS_MEMBER_RW(Display, "exposureTime", exposureTime, setExposureTime)



// --- public --------------------------------------------------------------------------------------

Display::Display(const std::string& title, const TSize& size):
	display_(),
	title_(title),
    size_(size),
	rgbSpace_(RgbSpace::defaultSpace()),
    gamma_(1.f),
	exposureTime_(-1.f),
    isQuiting_(false),
	isDirty_(false),
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



const Display::TSize& Display::size() const
{
	return size_;
}



const TRgbSpacePtr& Display::rgbSpace() const
{
	return rgbSpace_;
}



const TScalar Display::gamma() const
{
    return gamma_;
}



const TScalar Display::exposureTime() const
{
    return exposureTime_;
}



void Display::setRgbSpace(const TRgbSpacePtr& rgbSpace)
{
	rgbSpace_ = rgbSpace;
}



void Display::setGamma(TScalar gammaExponent)
{
    gamma_ = gammaExponent;
}



void Display::setExposureTime(TScalar time)
{
    exposureTime_ = time;
}



// --- private -------------------------------------------------------------------------------------

void Display::doBeginRender()
{
	LASS_LOCK(mutex_)
	{
		const std::size_t n = size_.x * size_.y;
		renderBuffer_.clear();
		renderBuffer_.resize(n);
		displayBuffer_.clear();
		displayBuffer_.resize(n);
		numberOfSamples_.clear();
		numberOfSamples_.resize(n, 0);
		isQuiting_ = false;
		isDirty_ = false;
		isCanceling_ = false;
		isAnyKeyed_ = false;
	}
	
	displayLoop_.reset(
		util::threadFun(util::makeCallback(this, displayLoop), util::threadJoinable));
	displayLoop_->run();
}



void Display::doWriteRender(const OutputSample* first, const OutputSample* last)
{
    LASS_ASSERT(size_.x > 0 && size_.y > 0);

	LASS_LOCK(mutex_)
	{
		while (first != last)
		{
			const TPoint2D& position = first->screenCoordinate();
			LASS_ASSERT(position.x >= 0 && position.y >= 0 && position.x < 1 && position.y < 1);
			const unsigned i = static_cast<unsigned>(num::floor(position.x * size_.x));
			const unsigned j = static_cast<unsigned>(num::floor(position.y * size_.y));
			if (i < 0 || i >= size_.x || j < 0 || j >= size_.y)
			{
				return;
			}
			LASS_ASSERT(i < size_.x && j < size_.y);

			TVector3D xyz = first->radiance().xyz();
			xyz *= first->weight();
			
			const prim::ColorRGBA color = rgbSpace_->convert(xyz);
			PixelToaster::FloatingPointPixel& pixel = renderBuffer_[j * size_.x + i];
			pixel.a = color.a;
			pixel.b = color.b;
			pixel.g = color.g;
			pixel.r = color.r;
			
			++numberOfSamples_[j * size_.x + i];

			++first;
			isDirty_ = true;
		}
	}
}



void Display::doEndRender()
{
	LASS_LOCK(mutex_)
	{
		isQuiting_ = true;
	}
	displayLoop_->join();
}



const bool Display::doIsCanceling() const
{
	return isCanceling_;
}



void Display::onKeyPressed(PixelToaster::Key key)
{
	if (key == PixelToaster::Key::Escape)
	{
		onClose();
		return;
	}

	LASS_LOCK(mutex_)
	{
		isAnyKeyed_ = true;
	}
	signal_.signal();
}



void Display::onClose()
{
	LASS_LOCK(mutex_)
	{
		isCanceling_ = true;
	}
	signal_.signal();
}



void Display::displayLoop()
{
	LASS_ENFORCE(display_.open(
		title_.c_str(), size_.x, size_.y, PixelToaster::Output::Windowed,
		PixelToaster::Mode::FloatingPoint));
	display_.listener(this);

	bool isRunning = true;
	while (isRunning)
	{
		LASS_LOCK(mutex_)
		{
			if (isQuiting_ || isCanceling_)
			{
				isRunning = false;
			}

			if (isDirty_)
			{
				updateDisplayBuffer();
				isDirty_ = false;
			}
		}

		if (isRunning)
		{
			LASS_ENFORCE(display_.update(displayBuffer_));
			signal_.wait(250);
		}
	}

	waitForAnyKey();

	display_.close();
}



/** @internal
 *  @warning NOT THREAD SAFE, but that's ok since we lock it externaly.
 */
void Display::updateDisplayBuffer()
{
	std::copy(renderBuffer_.begin(), renderBuffer_.end(), displayBuffer_.begin());

	const std::size_t n = size_.x * size_.y;
	const TScalar invGamma = num::inv(gamma_);

	if (exposureTime_ > 0)
	{
		for (std::size_t i = 0; i < n; ++i)
		{
			PixelToaster::FloatingPointPixel& p = displayBuffer_[i];
			const unsigned nSamples = std::max<unsigned>(numberOfSamples_[i], 1);
			const TScalar scaler = -exposureTime_ / nSamples;

			p.a = num::clamp(p.a, 0.f, 1.f);
			p.b = num::pow(num::clamp(1.f - num::exp(scaler * p.b), 0.f, 1.f), invGamma);
			p.g = num::pow(num::clamp(1.f - num::exp(scaler * p.g), 0.f, 1.f), invGamma);
			p.r = num::pow(num::clamp(1.f - num::exp(scaler * p.r), 0.f, 1.f), invGamma);
		}
	}
	else
	{
		for (std::size_t i = 0; i < n; ++i)
		{
			PixelToaster::FloatingPointPixel& p = displayBuffer_[i];
			const unsigned nSamples = std::max<unsigned>(numberOfSamples_[i], 1);
			const TScalar scaler = 1.f / nSamples;

			p.a = num::clamp(p.a, 0.f, 1.f);
			p.b = num::pow(num::clamp(scaler * p.b, 0.f, 1.f), invGamma);
			p.g = num::pow(num::clamp(scaler * p.g, 0.f, 1.f), invGamma);
			p.r = num::pow(num::clamp(scaler * p.r, 0.f, 1.f), invGamma);
		}
	}
}



void Display::waitForAnyKey()
{
	LASS_LOCK(mutex_);
	{
		isAnyKeyed_ = false;
	}

	while (true)
	{
		LASS_LOCK(mutex_)
		{
			if (isAnyKeyed_ || isCanceling_)
			{
				return;
			}
		}
		LASS_ENFORCE(display_.update(displayBuffer_));
		signal_.wait(250);
	}
}



// --- Display::Listener ---------------------------------------------------------------------------




// --- free ----------------------------------------------------------------------------------------



}

}

// EOF