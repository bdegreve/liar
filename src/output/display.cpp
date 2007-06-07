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

#include "output_common.h"

#if HAVE_PIXELTOASTER_H

#include "display.h"
#include <lass/util/callback_0.h>
#include <lass/util/thread_fun.h>

namespace liar
{
namespace output
{

PY_DECLARE_CLASS(Display);
PY_CLASS_CONSTRUCTOR_2(Display, const std::string&, const TResolution2D&)
PY_CLASS_MEMBER_R(Display, title)
PY_CLASS_MEMBER_RW(Display, rgbSpace, setRgbSpace)
PY_CLASS_MEMBER_RW(Display, exposure, setExposure)
PY_CLASS_MEMBER_RW(Display, fStops, setFStops)
PY_CLASS_MEMBER_RW(Display, gain, setGain)
PY_CLASS_METHOD(Display, testGammut)



// --- public --------------------------------------------------------------------------------------

Display::Display(const std::string& title, const TResolution2D& resolution):
	display_(),
	title_(title),
	resolution_(resolution),
	rgbSpace_(RgbSpace::defaultSpace()),
	exposure_(1.f),
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



void Display::setGain(TScalar gain)
{
	gain_ = gain;
}



void Display::testGammut()
{
	if (isRendering())
	{
		endRender();
	}
	exposure_ = 0;
	const size_t n = 500;
	const size_t m = n * num::sqrt(3.) / 2;
	resolution_ = TResolution2D(n, m + 200);
	beginRender();

	std::fill(totalWeight_.begin(), totalWeight_.end(), 1.f);

	for (size_t j = 0; j < m; ++j)
	{
		LASS_LOCK(renderBufferLock_)
		{
			for (size_t i = 0; i < n; ++i)
			{
				TVector3D xyz;
				xyz.y = 1.f - static_cast<TScalar>(j) / (m - 1);
				xyz.x = static_cast<TScalar>(i) / (n - 1) - (xyz.y / 2);
				xyz.z = 1.f - xyz.x - xyz.y;
				if (xyz.x < 0 || xyz.z < 0)
				{
					xyz = TVector3D();
				}
				renderBuffer_[j * resolution_.x + i] = xyz;
				renderDirtyBox_ += TDirtyBox::TPoint(i, j);
			}
		}
	}

	for (size_t j = m + 100; j < m + 200; ++j)
	{
		LASS_LOCK(renderBufferLock_)
		{
			for (size_t i = 0; i < n; ++i)
			{
				const TScalar f = static_cast<TScalar>(i) / (n - 1);
				renderBuffer_[j * resolution_.x + i] = TVector3D(f, f, f);
				renderDirtyBox_ += TDirtyBox::TPoint(i, j);
			}
		}
	}

	for (size_t j = 0; j < 150; ++j)
	{
		LASS_LOCK(renderBufferLock_)
		{
			for (size_t i = n - 150; i < n; ++i)
			{
				const TScalar f = (i + j) % 2;
				renderBuffer_[j * resolution_.x + i] = TVector3D(f, f, f);
				renderDirtyBox_ += TDirtyBox::TPoint(i, j);
			}
		}
	}
	for (size_t j = 50; j < 100; ++j)
	{
		LASS_LOCK(renderBufferLock_)
		{
			for (size_t i = n - 100; i < n - 50; ++i)
			{
				const TScalar f = .5f;
				renderBuffer_[j * resolution_.x + i] = TVector3D(f, f, f);
				renderDirtyBox_ += TDirtyBox::TPoint(i, j);
			}
		}
	}		
}



// --- private -------------------------------------------------------------------------------------

const TResolution2D Display::doResolution() const
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
	LASS_ASSERT(static_cast<int>(resolution_.x) > 0 && static_cast<int>(resolution_.y) > 0);

	LASS_LOCK(renderBufferLock_)
	{
		while (first != last)
		{
			const TPoint2D& position = first->screenCoordinate();
			const int i = static_cast<int>(num::floor(position.x * resolution_.x));
			const int j = static_cast<int>(num::floor(position.y * resolution_.y));
			if (i >= 0 && i < static_cast<int>(resolution_.x) &&
				j >= 0 && j < static_cast<int>(resolution_.y))
			{
				renderBuffer_[j * resolution_.x + i] += first->radiance().xyz() * first->alpha() * first->weight();
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

	while (!(isQuiting_ || isCanceling_))
	{
		copyToDisplayBuffer();
		LASS_ENFORCE(display_.update(displayBuffer_));
		signal_.wait(250);
	}

	waitForAnyKey();
	display_.close();
}


/** @internal
 */
void Display::copyToDisplayBuffer()
{
	LASS_LOCK(renderBufferLock_)
	{
		if (renderDirtyBox_.isEmpty())
		{
			return;
		}

		const TDirtyBox::TPoint min = renderDirtyBox_.min();
		const TDirtyBox::TPoint max = renderDirtyBox_.max();
		const unsigned nx = max.x - min.x + 1;

		if (exposure_ > 0)
		{
			for (unsigned j = min.y; j <= max.y; ++j)
			{
				const unsigned kBegin = j * resolution_.x + min.x;
				const unsigned kEnd = kBegin + nx;
				for (unsigned k = kBegin; k < kEnd; ++k)
				{
					const TScalar w = totalWeight_[k];
					TVector3D xyz = w > 0 ? renderBuffer_[k] * (gain_ / w) : TVector3D();
					xyz.x = 1 - num::exp(-exposure_ * xyz.x);
					xyz.y = 1 - num::exp(-exposure_ * xyz.y);
					xyz.z = 1 - num::exp(-exposure_ * xyz.z);
					const prim::ColorRGBA rgb = rgbSpace_->convert(xyz);
					PixelToaster::FloatingPointPixel& p = displayBuffer_[k];
					p.a = 1;
					p.b = rgb.b;
					p.g = rgb.g;
					p.r = rgb.r;
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
					const TScalar w = totalWeight_[k];
					TVector3D xyz = w > 0 ? renderBuffer_[k] * (gain_ / w) : TVector3D();
					const prim::ColorRGBA rgb = rgbSpace_->convert(xyz);
					PixelToaster::FloatingPointPixel& p = displayBuffer_[k];
					p.a = 1;
					p.b = rgb.b;
					p.g = rgb.g;
					p.r = rgb.r;
				}			
			}
		}

		renderDirtyBox_.clear();
	}
}	



void Display::waitForAnyKey()
{
	isAnyKeyed_ = false;
	while (!(isAnyKeyed_ || isCanceling_))
	{
		copyToDisplayBuffer();
		LASS_ENFORCE(display_.update(displayBuffer_));
		signal_.wait(250);
	}
}



// --- Display::Listener ---------------------------------------------------------------------------




// --- free ----------------------------------------------------------------------------------------



}

}

#endif

// EOF
