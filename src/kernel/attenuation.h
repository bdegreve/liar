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

/** @class liar::Attenuation
 *  @brief Defines geometric attenuation of lights
 *  @author Bram de Greve [Bramz]
 *
 *  Defines a geometric attenuation of the form 1 / (constant + linear * r + quadratic * r * r)
 *  where r is the ray distance.
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_ATTENUATION_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_ATTENUATION_H

#include "kernel_common.h"

namespace liar
{
namespace kernel
{

class Attenuation;
typedef python::PyObjectPtr<Attenuation>::Type TAttenuationPtr;

class LIAR_KERNEL_DLL Attenuation: public python::PyObjectPlus
{
	PY_HEADER(python::PyObjectPlus)
public:

	Attenuation();
    Attenuation(TScalar constant, TScalar linear, TScalar quadratic);
    
	const TScalar constant() const;
    const TScalar linear() const;
    const TScalar quadratic() const;
    
	void setConstant(TScalar constant);
	void setLinear(TScalar linear);
	void setQuadratic(TScalar quadratic);

	TScalar attenuation(TScalar distance) const
	{
		return attenuation(distance, num::sqr(distance));
	}

	TScalar attenuation(TScalar distance, TScalar squaredDistance) const
	{
		return constant_ + linear_ * distance + quadratic_ * squaredDistance;
	}

	static TAttenuationPtr defaultAttenuation();

	const TPyObjectPtr reduce() const;
	const TPyObjectPtr getState() const;
	void setState(const TPyObjectPtr& state);

private:

	TScalar constant_;
	TScalar linear_;
	TScalar quadratic_;

	static TAttenuationPtr defaultAttenuation_;
};

}

}

#endif

// EOF
