/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
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

namespace liar
{
namespace output
{

PY_DECLARE_CLASS(Display);
PY_CLASS_CONSTRUCTOR_2(Display, const std::string&, const TResolution2D&)
PY_CLASS_MEMBER_R(Display, title)
PY_CLASS_MEMBER_RW(Display, rgbSpace, setRgbSpace)
PY_CLASS_MEMBER_RW(Display, exposure, setExposure)
PY_CLASS_MEMBER_RW(Display, exposureCorrection, setExposureCorrection)
PY_CLASS_MEMBER_RW(Display, autoExposure, setAutoExposure)
PY_CLASS_MEMBER_RW(Display, middleGrey, setMiddleGrey);
PY_CLASS_METHOD(Display, testGammut)
PY_CLASS_STATIC_CONST(Display, "TM_LINEAR", "linear");
PY_CLASS_STATIC_CONST(Display, "TM_COMPRESS_Y", "compress_y");
PY_CLASS_STATIC_CONST(Display, "TM_EXPONENTIAL_Y", "exponential_y");
PY_CLASS_STATIC_CONST(Display, "TM_EXPONENTIAL_XYZ", "exponential_xyz");

Display::TToneMappingDictionary Display::toneMappingDictionary_ = Display::makeToneMappingDictionary();

// --- public --------------------------------------------------------------------------------------

Display::Display(const std::string& title, const TResolution2D& resolution):
	title_(title),
	resolution_(resolution),
	rgbSpace_(RgbSpace::defaultSpace()),
	totalLogSceneLuminance_(0),
	sceneLuminanceCoverage_(0),
	middleGrey_(.13f),
	toneMapping_(tmExponentialY),
	autoExposure_(true),
	refreshTitle_(false),
	isQuiting_(false),
	isCanceling_(false)
{
	setExposure(0);
	setExposureCorrection(0);
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



const std::string Display::toneMapping() const
{
	return toneMappingDictionary_.key(toneMapping_);
}



const TScalar Display::exposure() const
{
	return exposure_ / 3.f;
}



const TScalar Display::exposureCorrection() const
{
	return exposureCorrection_ / 3.f;
}



const bool Display::autoExposure() const
{
	return autoExposure_;
}



const TScalar Display::middleGrey() const
{
	return middleGrey_;
}



void Display::setRgbSpace(const TRgbSpacePtr& rgbSpace)
{
	rgbSpace_ = rgbSpace;
}



void Display::setToneMapping(const std::string& toneMapping)
{
	LASS_LOCK(renderBufferLock_)
	{
		toneMapping_ = toneMappingDictionary_[stde::tolower(toneMapping)];
		renderDirtyBox_ = allTimeDirtyBox_;
	}
}



void Display::setExposure(TScalar fStops)
{
	const int thirdStops = static_cast<int>(num::round(fStops * 3));
	if (thirdStops != exposure_)
	{
		LASS_LOCK(renderBufferLock_)
		{
			exposure_ = thirdStops;
			gain_ = num::pow(TScalar(2), this->exposure() + this->exposureCorrection());
			renderDirtyBox_ = allTimeDirtyBox_;
			refreshTitle_ = true;
		}
	}
}



void Display::setExposureCorrection(TScalar fStops)
{
	const int thirdStops = static_cast<int>(num::round(fStops * 3));
	if (thirdStops != exposureCorrection_)
	{
		LASS_LOCK(renderBufferLock_)
		{
			exposureCorrection_ = thirdStops;
			gain_ = num::pow(TScalar(2), this->exposure() + this->exposureCorrection());
			renderDirtyBox_ = allTimeDirtyBox_;
			refreshTitle_ = true;
		}
	}
}



void Display::setAutoExposure(bool enable)
{
	if (enable && !autoExposure_)
	{
		LASS_LOCK(renderBufferLock_)
		{
			const TDirtyBox::TPoint min = allTimeDirtyBox_.min();
			const TDirtyBox::TPoint max = allTimeDirtyBox_.max();
			const unsigned nx = max.x - min.x + 1;
			const TScalar minY = 1e-10f;
			totalLogSceneLuminance_ = 0;
			sceneLuminanceCoverage_ = 0;
			for (unsigned j = min.y; j <= max.y; ++j)
			{
				const unsigned kBegin = j * resolution_.x + min.x;
				const unsigned kEnd = kBegin + nx;
				for (unsigned k = kBegin; k < kEnd; ++k)
				{
					const TScalar w = totalWeight_[k];
					if (w > 0)
					{
						TVector3D xyz = renderBuffer_[k] / w;
						totalLogSceneLuminance_ += num::log(std::max(xyz.y, minY));
						++sceneLuminanceCoverage_;
					}
				}			
			}
		}
	}
	autoExposure_ = enable;
	refreshTitle_ = true;
}



void Display::setMiddleGrey(TScalar level)
{
	middleGrey_ = level;
}



void Display::testGammut()
{
	if (isRendering())
	{
		endRender();
	}
	setToneMapping("linear");
	setExposure(0);
	const size_t n = 500;
	const size_t m = static_cast<size_t>(n * num::sqrt(3.) / 2);
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
				const TScalar f = static_cast<TScalar>((i + j) % 2);
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
	}
	
	displayLoop_.reset(
		util::threadFun(util::makeCallback(this, &Display::displayLoop), util::threadJoinable));
	displayLoop_->run();
}



void Display::doWriteRender(const OutputSample* first, const OutputSample* last)
{
	LASS_ASSERT(static_cast<int>(resolution_.x) > 0 && static_cast<int>(resolution_.y) > 0);

	const TScalar minY = 1e-10f;

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
				TVector3D& xyz = renderBuffer_[j * resolution_.x + i];
				TScalar& w = totalWeight_[j * resolution_.x + i];
				const TScalar oldY = xyz.y;
				const TScalar oldW = w;
				xyz += first->radiance().xyz() * first->alpha() * first->weight();
				w += first->weight();
				renderDirtyBox_ += TDirtyBox::TPoint(i, j);

				if (autoExposure_ && w > 0)
				{
					if (oldW > 0)
					{
						totalLogSceneLuminance_ -= num::log(std::max(oldY / oldW, minY));
						--sceneLuminanceCoverage_;
					}
					totalLogSceneLuminance_ += num::log(std::max(xyz.y / w, minY));
					++sceneLuminanceCoverage_;
				}
			}
			++first;
		}
		allTimeDirtyBox_ += renderDirtyBox_;
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
	using namespace PixelToaster;
	switch (key)
	{
	case Key::Escape:
		onClose(display);
		return;

	case Key::A:
		setAutoExposure(!autoExposure_);
		break;

	case Key::E:
		setToneMapping(toneMappingDictionary_.key(static_cast<ToneMapping>((toneMapping_ + 1) % numToneMapping)));
		break;

	case Key::Equals:
		if (autoExposure_)
		{
			setExposureCorrection(exposureCorrection() + .33f);
		}
		else
		{
			setExposure(exposure() + .33f);
		}
		break;

	case Key::Separator:
		if (autoExposure_)
		{
			setExposureCorrection(exposureCorrection() - .33f);
		}
		else
		{
			setExposure(exposure() - .33f);
		}
		break;

	default:
		break;
	}

	//isAnyKeyed_ = true;
	signal_.signal();
}



bool Display::onClose(PixelToaster::DisplayInterface& display)
{
	isCanceling_ = true;
	signal_.signal();
	return false;
}



void printThirdStops(std::ostringstream& stream, int thirdStops)
{
	const int i = thirdStops / 3;
	const int d = thirdStops % 3;
	stream << std::showpos;
	if (i || !d)
	{
		stream << " " << i << std::noshowpos;
	}
	if (d)
	{
		stream << " " << (i ? num::abs(d) : d) << "/3";
	}
}



const std::string Display::makeTitle() const
{
	std::ostringstream buffer;
	buffer << title_ << " [" << toneMappingDictionary_.key(toneMapping_);
	printThirdStops(buffer, exposure_);
	if (autoExposure_)
	{
		buffer << " A";
	}
	if (exposureCorrection_)
	{
		printThirdStops(buffer, exposureCorrection_);
	}
	buffer << "]";
	return buffer.str();
}



void Display::displayLoop()
{
	PixelToaster::Display display;
 	LASS_ENFORCE(display.open(
		makeTitle().c_str(), resolution_.x, resolution_.y, 
		PixelToaster::Output::Windowed,	PixelToaster::Mode::FloatingPoint));
	display.listener(this);

	LASS_ENFORCE(display.update(displayBuffer_)); // full update of initial buffer

	while (!isCanceling_)
	{
		copyToDisplayBuffer();

		if (refreshTitle_)
		{
			refreshTitle_ = false;
			display.title(makeTitle().c_str());
		}

#if LIAR_OUTPUT_HAVE_PIXELTOASTER_DIRTYBOX
		PixelToaster::Rectangle box;
		if (!displayDirtyBox_.isEmpty())
		{
			box.xBegin = displayDirtyBox_.min().x;
			box.yBegin = displayDirtyBox_.min().y;
			box.xEnd = displayDirtyBox_.max().x + 1;
			box.yEnd = displayDirtyBox_.max().y + 1;
		}
		LASS_ENFORCE(display.update(displayBuffer_, &box));
		displayDirtyBox_.clear();
#else
		LASS_ENFORCE(display_.update(displayBuffer_));
#endif
		signal_.wait(250);
	}

	display.close();
}


/** @internal
 */
void Display::copyToDisplayBuffer()
{
	LASS_LOCK(renderBufferLock_)
	{
		if (autoExposure_ && sceneLuminanceCoverage_ > 0)
		{
			const TScalar a = middleGrey_;
			const TScalar y = num::exp(totalLogSceneLuminance_ / sceneLuminanceCoverage_);
			if (y > 0)
			{
				TScalar g = 1;
				switch (toneMapping_)
				{
				case tmLinear: // a = g * y
					g = a / y;
					break;
				case tmCompressY: // a = g * y / (1 + g * y)  ->  g * y = a / (1 - a)
					g = a / (y * (1 - a));
					break;
				case tmExponentialY: // a = 1 - exp(-g * y)  ->  g * y = -ln(1 - a)
				case tmExponentialXYZ:
					g = -num::log(1 - a) / y;
					break;
				default:
					LASS_ENFORCE_UNREACHABLE;
				};
				const TScalar e = num::log2(g);
				setExposure(e); // reentrant lock
			}
		}

		if (renderDirtyBox_.isEmpty())
		{
			return;
		}

		const TDirtyBox::TPoint min = renderDirtyBox_.min();
		const TDirtyBox::TPoint max = renderDirtyBox_.max();
		const unsigned nx = max.x - min.x + 1;

		switch (toneMapping_)
		{
		case tmLinear:
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
			break;

		case tmCompressY:
			for (unsigned j = min.y; j <= max.y; ++j)
			{
				const unsigned kBegin = j * resolution_.x + min.x;
				const unsigned kEnd = kBegin + nx;
				for (unsigned k = kBegin; k < kEnd; ++k)
				{
					const TScalar w = totalWeight_[k];
					TVector3D xyz = w > 0 ? renderBuffer_[k] * (gain_ / w) : TVector3D();
					xyz /= 1 + xyz.y;
					const prim::ColorRGBA rgb = rgbSpace_->convert(xyz);
					PixelToaster::FloatingPointPixel& p = displayBuffer_[k];
					p.a = 1;
					p.b = rgb.b;
					p.g = rgb.g;
					p.r = rgb.r;
				}			
			}
			break;

		case tmExponentialY:
			for (unsigned j = min.y; j <= max.y; ++j)
			{
				const unsigned kBegin = j * resolution_.x + min.x;
				const unsigned kEnd = kBegin + nx;
				for (unsigned k = kBegin; k < kEnd; ++k)
				{
					const TScalar w = totalWeight_[k];
					TVector3D xyz = w > 0 ? renderBuffer_[k] * (gain_ / w) : TVector3D();
					xyz *= (1 - num::exp(-xyz.y)) / xyz.y;
					const prim::ColorRGBA rgb = rgbSpace_->convert(xyz);
					PixelToaster::FloatingPointPixel& p = displayBuffer_[k];
					p.a = 1;
					p.b = rgb.b;
					p.g = rgb.g;
					p.r = rgb.r;
				}			
			}
			break;

		case tmExponentialXYZ:
			for (unsigned j = min.y; j <= max.y; ++j)
			{
				const unsigned kBegin = j * resolution_.x + min.x;
				const unsigned kEnd = kBegin + nx;
				for (unsigned k = kBegin; k < kEnd; ++k)
				{
					const TScalar w = totalWeight_[k];
					TVector3D xyz = w > 0 ? renderBuffer_[k] * (gain_ / w) : TVector3D();
					xyz.x = 1 - num::exp(-xyz.x);
					xyz.y = 1 - num::exp(-xyz.y);
					xyz.z = 1 - num::exp(-xyz.z);
					const prim::ColorRGBA rgb = rgbSpace_->convert(xyz);
					PixelToaster::FloatingPointPixel& p = displayBuffer_[k];
					p.a = 1;
					p.b = rgb.b;
					p.g = rgb.g;
					p.r = rgb.r;
				}			
			}
			break;

		default:
			LASS_ENFORCE_UNREACHABLE;
		}

		displayDirtyBox_ = renderDirtyBox_;
		renderDirtyBox_.clear();
	}
}	



Display::TToneMappingDictionary Display::makeToneMappingDictionary()
{
	TToneMappingDictionary result;
	result.enableSuggestions();
	result.add("linear", tmLinear);
	result.add("compress_y", tmCompressY);
	result.add("exponential_y", tmExponentialY);
	result.add("exponential_xyz", tmExponentialXYZ);
	return result;
}



// --- free ----------------------------------------------------------------------------------------



}

}

#endif

// EOF
