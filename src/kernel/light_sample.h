/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2005  Bram de Greve
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

/** @class liar::LightSample
 *  @brief a radiance sample from a light shader
 *  @author Bram de Greve [BdG]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_LIGHT_SAMPLE_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_LIGHT_SAMPLE_H

#pragma once
#include "kernel_common.h"
#include "spectrum.h"

#include <lass/stde/iterator_range.h>

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL LightSample
{
public:

	LightSample(const Spectrum& iRadiance, const TVector3D& iDirection/*, TScalar iPdf*/);

	const Spectrum& radiance() const { return radiance_; }
	const TVector3D& direction() const { return direction_; }
	//const TScalar pdf() const { return pdf_; }

private:

	Spectrum radiance_;
	TVector3D direction_;
	//TScalar pdf_;
};

typedef std::vector<LightSample> TLightSamples;
typedef stde::iterator_range<TLightSamples::const_iterator> TLightSamplesRange;

}

}

#endif

// EOF
