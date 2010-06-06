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

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_LASS_EXPORTS_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_LASS_EXPORTS_H

#include "kernel_common.h"

namespace liar
{
namespace kernel
{
namespace impl
{

template 
<
	typename T,
	template <typename, typename> class PrimExportTraits,
	typename ExportTraits
>
struct PySpecialExportTraits: private PrimExportTraits<T, ExportTraits>
{
	typedef python::impl::ShadowTraits<typename python::ShadoweeTraits<T>::TShadow> TShadowTraits; 
	static PyObject* build(const liar::TTransformation2D& v)
	{
		return python::fromSharedPtrToNakedCast(TShadowTraits::buildObject(v));
	}
	static int get(PyObject* obj, liar::TTransformation2D& v)
	{
		if (PrimExportTraits<T, ExportTraits>::get(obj, v) == 0)
		{
			return 0;
		}
		return TShadowTraits::getObject(obj, v);
	}
};

PY_SHADOW_CLASS(LIAR_KERNEL_DLL, ShadowTransformation2D, liar::TTransformation2D)

}
}
}

PY_SHADOW_CASTERS(liar::kernel::impl::ShadowTransformation2D);

namespace lass
{
namespace python
{

template <>
struct PyExportTraits<liar::TTransformation2D>: 
	public ::liar::kernel::impl::PySpecialExportTraits< liar::TTransformation2D, impl::PyExportTraitsPrimTransformation, PyExportTraits<liar::TTransformation2D> >
{
	static const char* className() { return "Transformation2D"; }
};

}
}

#endif

// EOF
