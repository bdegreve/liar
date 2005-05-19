/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LIAR isn't a raytracer
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

/** @class liar::kernel::Texture
 *  @brief abstract base class of all textures
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_TEXTURE_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_TEXTURE_H

#include "kernel_common.h"
#include "intersection_context.h"

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL Texture: public python::PyObjectPlus
{
    PY_HEADER(python::PyObjectPlus)
public:

	typedef util::CallTraits<TSpectrum>::TValue TValue;
	typedef util::CallTraits<TSpectrum>::TParam TParam;
	typedef util::CallTraits<TSpectrum>::TReference TReference;
	typedef util::CallTraits<TSpectrum>::TConstReference TConstReference;

    virtual ~Texture();

    TValue operator()(const IntersectionContext& iContext) const { return doLookUp(iContext); }

protected:

    Texture(PyTypeObject* iType);

private:

    virtual TValue doLookUp(const IntersectionContext& iContext) const = 0;
};

typedef python::PyObjectPtr<Texture>::Type TTexturePtr;

}

}

#endif

// EOF
