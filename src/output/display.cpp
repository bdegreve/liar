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
PY_CLASS_MEMBER_RW(Display, rgbSpace, setRgbSpace)
PY_CLASS_MEMBER_RW(Display, exposure, setExposure)
PY_CLASS_MEMBER_RW(Display, exposureCorrection, setExposureCorrection)
PY_CLASS_MEMBER_RW(Display, autoExposure, setAutoExposure)
PY_CLASS_MEMBER_RW(Display, middleGrey, setMiddleGrey);
PY_CLASS_MEMBER_RW(Display, showHistogram, setShowHistogram);
PY_CLASS_METHOD(Display, testGammut)
PY_CLASS_STATIC_CONST(Display, "TM_LINEAR", "linear");
PY_CLASS_STATIC_CONST(Display, "TM_COMPRESS_Y", "compress_y");
PY_CLASS_STATIC_CONST(Display, "TM_COMPRESS_XYZ", "compress_xyz");
PY_CLASS_STATIC_CONST(Display, "TM_COMPRESS_RGB", "compress_rgb");
PY_CLASS_STATIC_CONST(Display, "TM_REINHARD2002_Y", "reinhard2002_y");
PY_CLASS_STATIC_CONST(Display, "TM_EXPONENTIAL_Y", "exponential_y");
PY_CLASS_STATIC_CONST(Display, "TM_EXPONENTIAL_XYZ", "exponential_xyz");
PY_CLASS_STATIC_CONST(Display, "TM_EXPONENTIAL_RGB", "exponential_rgb");
PY_CLASS_STATIC_CONST(Display, "TM_DUIKER_Y", "duiker_y");
PY_CLASS_STATIC_CONST(Display, "TM_DUIKER_XYZ", "duiker_xyz");
PY_CLASS_STATIC_CONST(Display, "TM_DUIKER_RGB", "duiker_rgb");

Display::TToneMappingDictionary Display::toneMappingDictionary_ = Display::makeToneMappingDictionary();

// --- public --------------------------------------------------------------------------------------

Display::Display(const std::string& title, const TResolution2D& resolution):
	title_(title),
	resolution_(resolution),
	totalLogSceneLuminance_(0),
	maxSceneLuminance_(0),
	sceneLuminanceCoverage_(0),
	middleGrey_(.184f),
	toneMapping_(tmExponentialRGB),
	autoExposure_(true),
	showHistogram_(false),
	wasShowingHistogram_(false),
	refreshTitle_(false),
	isCanceling_(false)
{
	setRgbSpace(RgbSpace::defaultSpace());
	setExposure(0);
	setExposureCorrection(0);
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



const TRgbSpacePtr& Display::rgbSpace() const
{
	return rgbSpace_;
}



const std::string Display::toneMapping() const
{
	return toneMappingDictionary_.key(toneMapping_);
}



TScalar Display::exposure() const
{
	return exposure_ / 3.f;
}



TScalar Display::exposureCorrection() const
{
	return exposureCorrection_ / 3.f;
}



bool Display::autoExposure() const
{
	return autoExposure_;
}



TScalar Display::middleGrey() const
{
	return middleGrey_;
}



bool Display::showHistogram() const
{
	return showHistogram_;
}


void Display::setRgbSpace(const TRgbSpacePtr& rgbSpace)
{
	rgbSpace_ = rgbSpace;
	linearSpace_ = rgbSpace_->withGamma(1);
	gammaSpace_ = CIEXYZ->withGamma(rgbSpace_->gamma());
}



void Display::setToneMapping(const std::string& toneMapping)
{
	LASS_LOCK(renderBufferLock_)
	{
		toneMapping_ = toneMappingDictionary_[stde::tolower(toneMapping)];
		renderDirtyBox_ += allTimeDirtyBox_;
		refreshTitle_ = true;
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
			renderDirtyBox_ += allTimeDirtyBox_;
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
			renderDirtyBox_ += allTimeDirtyBox_;
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
			const size_t nx = max.x - min.x + 1;
			const TScalar minY = 1e-10f;
			totalLogSceneLuminance_ = 0;
			sceneLuminanceCoverage_ = 0;
			for (size_t j = min.y; j <= max.y; ++j)
			{
				const size_t kBegin = j * resolution_.x + min.x;
				const size_t kEnd = kBegin + nx;
				for (size_t k = kBegin; k < kEnd; ++k)
				{
					const TScalar w = totalWeight_[k];
					const XYZ xyz = renderBuffer_[k];
					if (w > 0 && xyz.y > 0)
					{
						totalLogSceneLuminance_ += num::log(std::max(xyz.y / w, minY));
						maxSceneLuminance_ = std::max(xyz.y / w, maxSceneLuminance_);
						++sceneLuminanceCoverage_;
					}
				}			
			}
		}
	}
	autoExposure_ = enable;
	refreshTitle_ = true;
}



void Display::setShowHistogram(bool enable)
{
	showHistogram_ = enable;
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
				XYZ xyz;
				xyz.y = 1.f - static_cast<TScalar>(j) / (m - 1);
				xyz.x = static_cast<TScalar>(i) / (n - 1) - (xyz.y / 2);
				xyz.z = 1.f - xyz.x - xyz.y;
				if (xyz.x < 0 || xyz.z < 0)
				{
					xyz = 0;
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
				renderBuffer_[j * resolution_.x + i] = static_cast<TScalar>(i) / (n - 1);
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
				renderBuffer_[j * resolution_.x + i] = static_cast<TScalar>((i + j) % 2);
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
				renderBuffer_[j * resolution_.x + i] = .5f;
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
	// TODO: don't need to reset display loop if it already has the right resolution.

	if (displayLoop_)
	{
		cancel();
		displayLoop_->join();
	}	
	
	const size_t n = resolution_.x * resolution_.y;
	renderBuffer_.clear();
	renderBuffer_.resize(n);
	displayBuffer_.clear();
	displayBuffer_.resize(n);
	totalWeight_.clear();
	totalWeight_.resize(n, 0);
	renderDirtyBox_.clear();
	displayDirtyBox_.clear();
	
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
				XYZ& xyz = renderBuffer_[j * resolution_.x + i];
				TScalar& w = totalWeight_[j * resolution_.x + i];
				const TScalar oldY = xyz.y;
				const TScalar oldW = w;
				xyz += first->radiance() * first->alpha() * first->weight();
				w += first->weight();
				renderDirtyBox_ += TDirtyBox::TPoint(i, j);

				if (autoExposure_ && w > 0 && xyz.y > 0)
				{
					maxSceneLuminance_ = std::max(xyz.y / w, maxSceneLuminance_);
					if (oldW > 0 && oldY > 0)
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
}



bool Display::doIsCanceling() const
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

	case Key::H:
		setShowHistogram(!showHistogram_);
		break;

	case Key::Equals:
	case Key::Add:
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
	case Key::Subtract:
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



bool Display::onClose(PixelToaster::DisplayInterface&)
{
	cancel();
	return false;
}


void Display::cancel()
{
	isCanceling_ = true;
	signal_.signal();
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
	if (autoExposure_)
	{
		printThirdStops(buffer, exposure_);
		buffer << " A";
		printThirdStops(buffer, exposureCorrection_);
	}
	else
	{
		printThirdStops(buffer, exposure_ + exposureCorrection_);
	}
	buffer << "]";
	return buffer.str();
}



void Display::displayLoop()
{
	isCanceling_ = false;

	PixelToaster::Display display;
	const int width = num::numCast<int>(resolution_.x);
	const int height = num::numCast<int>(resolution_.y);
 	LASS_ENFORCE(display.open(makeTitle().c_str(), width, height, PixelToaster::Output::Windowed,	PixelToaster::Mode::TrueColor));
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
			box.xBegin = num::numCast<int>(displayDirtyBox_.min().x);
			box.yBegin = num::numCast<int>(displayDirtyBox_.min().y);
			box.xEnd = num::numCast<int>(displayDirtyBox_.max().x + 1);
			box.yEnd = num::numCast<int>(displayDirtyBox_.max().y + 1);
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

namespace
{
	inline double filmic(double x)
	{
		x = std::max(0., x - 0.004);
		const double y = (x * (6.2 * x + .5)) / (x * (6.2 * x + 1.7) + 0.06); 
		return num::pow(y, 2.2); // undo gamma
	}
	inline double invFilmic(double y)
	{
		y = std::max(0., std::min(y, 0.99999));
		y = num::pow(y, 1. / 2.2); // apply gamma
		// a*x*x + b*y + c == 0
		const double a = 6.2f* y - 6.2;
		const double b = 1.7 * y - .5;
		const double c = .06 * y;
		const double D = b * b - 4 * a * c;
		const double x = (-b - num::sqrt(D)) / (2 * a);
		return x + 0.004;
	}
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
				case tmCompressRGB: 
				case tmReinhard2002Y:
				case tmReinhard2002RGB:
					g = a / (y * (1 - a));
					break;
				case tmExponentialY: // a = 1 - exp(-g * y)  ->  g * y = -ln(1 - a)
				case tmExponentialRGB:
					g = -num::log(1 - a) / y;
					break;
				case tmDuikerY:
				case tmDuikerRGB:
					g = invFilmic(a) / y;
					break;
				default:
					LASS_ENFORCE_UNREACHABLE;
				};
				const TScalar e = num::log2(g);
				setExposure(e); // reentrant lock
			}
		}

		tonemap( renderDirtyBox_ );
		histogram();

		renderDirtyBox_.clear();
	}
}


void Display::tonemap(const TDirtyBox& box)
{
	const TDirtyBox::TPoint min = box.min();
	const TDirtyBox::TPoint max = box.max();
	const size_t nx = max.x - min.x + 1;

	const TScalar Lw = gain_ * maxSceneLuminance_;
	const TScalar invLwSquared = num::inv(Lw * Lw);

	switch (toneMapping_)
	{
	case tmLinear:
		for (size_t j = min.y; j <= max.y; ++j)
		{
			const size_t kBegin = j * resolution_.x + min.x;
			const size_t kEnd = kBegin + nx;
			for (size_t k = kBegin; k < kEnd; ++k)
			{
				const TScalar w = totalWeight_[k];
				XYZ xyz = w > 0 ? renderBuffer_[k] * (gain_ / w) : 0;
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
		for (size_t j = min.y; j <= max.y; ++j)
		{
			const size_t kBegin = j * resolution_.x + min.x;
			const size_t kEnd = kBegin + nx;
			for (size_t k = kBegin; k < kEnd; ++k)
			{
				const TScalar w = totalWeight_[k];
				XYZ xyz = w > 0 ? renderBuffer_[k] * (gain_ / w) : 0;
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

	case tmCompressRGB:
		for (size_t j = min.y; j <= max.y; ++j)
		{
			const size_t kBegin = j * resolution_.x + min.x;
			const size_t kEnd = kBegin + nx;
			for (size_t k = kBegin; k < kEnd; ++k)
			{
				const TScalar w = totalWeight_[k];
				const XYZ xyz = w > 0 ? renderBuffer_[k] * (gain_ / w) : 0;
				const prim::ColorRGBA linear = linearSpace_->convert(xyz);
				const XYZ tonemapped(linear.r / (1 + linear.r), linear.g / (1 + linear.g), linear.b / (1 + linear.b));
				const prim::ColorRGBA rgb = gammaSpace_->convert(tonemapped);
				PixelToaster::FloatingPointPixel& p = displayBuffer_[k];
				p.a = 1;
				p.b = rgb.b;
				p.g = rgb.g;
				p.r = rgb.r;
			}			
		}
		break;

	case tmReinhard2002Y:
		for (size_t j = min.y; j <= max.y; ++j)
		{
			const size_t kBegin = j * resolution_.x + min.x;
			const size_t kEnd = kBegin + nx;
			for (size_t k = kBegin; k < kEnd; ++k)
			{
				const TScalar w = totalWeight_[k];
				XYZ xyz = w > 0 ? renderBuffer_[k] * (gain_ / w) : 0;
				const TScalar L = xyz.y;
				xyz *= (1 + L * invLwSquared) / (1 + L);
				const prim::ColorRGBA rgb = rgbSpace_->convert(xyz);
				PixelToaster::FloatingPointPixel& p = displayBuffer_[k];
				p.a = 1;
				p.b = rgb.b;
				p.g = rgb.g;
				p.r = rgb.r;
			}
		}
		break;

	case tmReinhard2002RGB:
		for (size_t j = min.y; j <= max.y; ++j)
		{
			const size_t kBegin = j * resolution_.x + min.x;
			const size_t kEnd = kBegin + nx;
			for (size_t k = kBegin; k < kEnd; ++k)
			{
				const TScalar w = totalWeight_[k];
				const XYZ xyz = w > 0 ? renderBuffer_[k] * (gain_ / w) : 0;
				const prim::ColorRGBA linear = linearSpace_->convert(xyz);
				const XYZ tonemapped(
					linear.r * (1 + linear.r * invLwSquared) / (1 + linear.r), 
					linear.g * (1 + linear.g * invLwSquared) / (1 + linear.g), 
					linear.b * (1 + linear.b * invLwSquared) / (1 + linear.b));
				const prim::ColorRGBA rgb = gammaSpace_->convert(tonemapped);
				PixelToaster::FloatingPointPixel& p = displayBuffer_[k];
				p.a = 1;
				p.b = rgb.b;
				p.g = rgb.g;
				p.r = rgb.r;
			}			
		}
		break;

	case tmExponentialY:
		for (size_t j = min.y; j <= max.y; ++j)
		{
			const size_t kBegin = j * resolution_.x + min.x;
			const size_t kEnd = kBegin + nx;
			for (size_t k = kBegin; k < kEnd; ++k)
			{
				const TScalar w = totalWeight_[k];
				XYZ xyz = w > 0 ? renderBuffer_[k] * (gain_ / w) : 0;
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

	case tmExponentialRGB:
		for (size_t j = min.y; j <= max.y; ++j)
		{
			const size_t kBegin = j * resolution_.x + min.x;
			const size_t kEnd = kBegin + nx;
			for (size_t k = kBegin; k < kEnd; ++k)
			{
				const TScalar w = totalWeight_[k];
				const XYZ xyz = w > 0 ? renderBuffer_[k] * (gain_ / w) : 0;
				const prim::ColorRGBA linear = linearSpace_->convert(xyz);
				const XYZ tonemapped(1 - num::exp(-linear.r), 1 - num::exp(-linear.g), 1 - num::exp(-linear.b));
				const prim::ColorRGBA rgb = gammaSpace_->convert(tonemapped);
				PixelToaster::FloatingPointPixel& p = displayBuffer_[k];
				p.a = 1;
				p.b = rgb.b;
				p.g = rgb.g;
				p.r = rgb.r;
			}			
		}
		break;

	case tmDuikerY:
		for (size_t j = min.y; j <= max.y; ++j)
		{
			const size_t kBegin = j * resolution_.x + min.x;
			const size_t kEnd = kBegin + nx;
			for (size_t k = kBegin; k < kEnd; ++k)
			{
				const TScalar w = totalWeight_[k];
				XYZ xyz = w > 0 ? renderBuffer_[k] * (gain_ / w) : 0;
				xyz *= filmic(xyz.y) / std::max(xyz.y, 0.004);
				const prim::ColorRGBA rgb = rgbSpace_->convert(xyz);
				PixelToaster::FloatingPointPixel& p = displayBuffer_[k];
				p.a = 1;
				p.b = rgb.b;
				p.g = rgb.g;
				p.r = rgb.r;
			}			
		}
		break;

	case tmDuikerRGB:
		for (size_t j = min.y; j <= max.y; ++j)
		{
			const size_t kBegin = j * resolution_.x + min.x;
			const size_t kEnd = kBegin + nx;
			for (size_t k = kBegin; k < kEnd; ++k)
			{
				const TScalar w = totalWeight_[k];
				const XYZ xyz = w > 0 ? renderBuffer_[k] * (gain_ / w) : 0;
				const prim::ColorRGBA linear = linearSpace_->convert(xyz);
				const XYZ tonemapped(filmic(linear.r), filmic(linear.g), filmic(linear.b));
				const prim::ColorRGBA rgb = gammaSpace_->convert(tonemapped);
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

	displayDirtyBox_ += box;
}

namespace 
{
    size_t bucket( TScalar x, size_t maxBucket )
    {
        return std::min(static_cast<size_t>( std::max(num::round(x * maxBucket), TScalar(0)) ), maxBucket );
    }
}

void Display::histogram()
{
    const size_t numBuckets = resolution_.x / 4;
    const size_t height = resolution_.y / 5;

    // overwrite area of previous histo with real image, so we can recompute
    const TDirtyBox histoBox(TDirtyBox::TPoint( resolution_.x - numBuckets - 1, 0 ), TDirtyBox::TPoint( resolution_.x - 1, height ));

    if (wasShowingHistogram_)
    {
        tonemap( histoBox );
        wasShowingHistogram_ = false;
    }

    if (!showHistogram_)
    {
        return;
    }

    typedef prim::Vector3D<size_t> THistoSample;
    std::vector<THistoSample> histo(numBuckets);
    const size_t maxBucket = numBuckets - 1;
    for (size_t k = 0, n = displayBuffer_.size(); k < n; ++k)
    {
        if (totalWeight_[k] <= 0)
        {
            continue;
        }
        const PixelToaster::FloatingPointPixel& p = displayBuffer_[k];
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
    const TScalar scale = max > 0 ? TScalar(height) / max : TScalar(0);
    for (size_t k = 0; k < numBuckets; ++k)
    {
        histo[k].x = static_cast<size_t>(histo[k].x * scale);
        histo[k].y = static_cast<size_t>(histo[k].y * scale);
        histo[k].z = static_cast<size_t>(histo[k].z * scale);
    }
    for (size_t j = 0; j < height; ++j)
    {
        const size_t kBegin = j * resolution_.x + histoBox.min().x;
        const size_t threshold = height - j;
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
    const size_t kBegin = height * resolution_.x + histoBox.min().x;
    for (size_t k = 0; k <= numBuckets; ++k)
    {
        displayBuffer_[kBegin + k] = PixelToaster::FloatingPointPixel(.25f, .25f, .25f, 1);
    }

	displayDirtyBox_ += histoBox;
    wasShowingHistogram_ = true;
}

Display::TToneMappingDictionary Display::makeToneMappingDictionary()
{
	TToneMappingDictionary result;
	result.enableSuggestions();
	result.add("linear", tmLinear);
	result.add("compress_y", tmCompressY);
	result.add("compress_rgb", tmCompressRGB);
	result.add("reinhard2002_y", tmReinhard2002Y);
	result.add("reinhard2002_rgb", tmReinhard2002RGB);
	result.add("exponential_y", tmExponentialY);
	result.add("exponential_rgb", tmExponentialRGB);
	result.add("duiker_y", tmDuikerY);
	result.add("duiker_rgb", tmDuikerRGB);
	return result;
}



// --- free ----------------------------------------------------------------------------------------



}

}

#endif

// EOF
