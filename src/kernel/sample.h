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

/** @class liar::Sample
 *  @brief representation of a single sample that must be rendered
 *  @author Bram de Greve [BdG]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SAMPLE_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SAMPLE_H

#include "kernel_common.h"
#include <lass/prim/point_2d.h>
#include <lass/stde/iterator_range.h>

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL Sample
{
public:

	//typedef  TSampleSequence1D;
	//typedef  TSampleSequence2D;

	typedef stde::iterator_range<std::vector<TScalar>::const_iterator> TSubSequence1D;
	typedef stde::iterator_range<std::vector<TVector2D>::const_iterator> TSubSequence2D;

    Sample();
    
	const TPoint2D& screenCoordinate() const;
	const TPoint2D& lensCoordinate() const;
	const TTime time() const;

    const TScalar weight() const;
    void setWeight(TScalar iWeight);

    const TSubSequence1D subSequence1D(int iId) const;
    const TSubSequence2D subSequence2D(int iId) const;

private:

	friend class Sampler;

    TPoint2D screenCoordinate_;
	TPoint2D lensCoordinate_;
	TTime time_;
    TScalar weight_;
    std::vector<TScalar> subSequences1D_;
    std::vector<TVector2D> subSequences2D_;
	Sampler* sampler_;
};

}

}

#endif

// EOF
