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

/** @class liar::Recovery
*  @brief Spectral recovery based on work of Meng, J., Simon, F., Hanika, J. and Dachsbacher, C. (2015)
*  @author Bram de Greve [Bramz]
*
*  Meng, J., Simon, F., Hanika, J. and Dachsbacher, C. (2015), 
*  Physically Meaningful Rendering using Tristimulus Colours. 
*  Computer Graphics Forum, 34: 31–40. doi: 10.1111/cgf.12676
*/

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SPECTRA_RECOVERY_MENG_SIMON_H
#define LIAR_GUARDIAN_OF_INCLUSION_SPECTRA_RECOVERY_MENG_SIMON_H

#include "spectra_common.h"
#include "../kernel/recovery.h"

#include <lass/spat/planar_mesh.h>
#include <lass/meta/empty_type.h>

namespace liar
{
namespace spectra
{

class LIAR_SPECTRA_DLL RecoveryMengSimon : public Recovery
{
	PY_HEADER(Recovery)
public:

	typedef TPoint2D TXY;

	struct LessXY
	{
		bool operator()(TXY a, TXY b) const
		{
			return a.x < b.x || (a.x == b.x && a.y < b.y);
		}
	};

	typedef Spectral::TValue TValue;
	typedef std::vector<TValue> TValues;
	typedef std::vector<TWavelength> TWavelengths;
	typedef std::map<TXY, TValues, LessXY> TSamples;
	typedef std::pair<TXY, TXY> TEdge;
	typedef std::vector<TEdge> TEdges;

	RecoveryMengSimon(const TWavelengths& wavelengths, const TSamples& samples);
	~RecoveryMengSimon();

	TValues recover(const XYZ& xyz) const;

	TEdges meshEdges() const;

private:

	Spectral doRecover(const XYZ& xyz, const Sample& sample, SpectralType type) const override;

	struct Impl;
	std::unique_ptr<Impl> pimpl_;
};

}

}

#endif

// EOF
