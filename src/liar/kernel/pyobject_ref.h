/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2025  Bram de Greve (bramz@users.sourceforge.net)
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

#pragma once

#include "kernel_common.h"

namespace liar::kernel
{

template <typename T>
class PyObjectRef
{
public:
    using TValue = T;
    using TValuePtr = typename lass::python::PyObjectPtr<T>::Type;

    PyObjectRef():
        ptr_(nullptr)
    {
    }
    explicit PyObjectRef(const TValuePtr& ptr):
        ptr_(PY_ENFORCE_POINTER(ptr))
    {
    }

    explicit PyObjectRef(TValue* ptr):
        ptr_(PY_ENFORCE_POINTER(ptr))
    {
    }

    const TValuePtr& ptr() const
    {
        return PY_ENFORCE_POINTER(ptr_);
    }
    operator TValuePtr() const
    {
        return ptr();
    }

    T* get() const
    {
        return ptr().get();
    }
    T* operator->() const
    {
        return get();
    }
    T& operator*() const
    {
        return *get();
    }
    explicit operator bool() const
    {
        return true;
    }

    template <typename U>
    bool operator==(const PyObjectRef<U>& other) const
    {
        return ptr_ == other.ptr_;
    }
    template <typename U>
    bool operator!=(const PyObjectRef<U>& other) const
    {
        return ptr_ != other.ptr_;
    }
private:
    TValuePtr ptr_;
};

}

namespace lass::python
{

template <typename T>
struct PyExportTraits<liar::kernel::PyObjectRef<T>>
{
    using TRef = liar::kernel::PyObjectRef<T>;
    constexpr static const char* py_typing = "T";

    static PyObject* build(const TRef& v)
    {
        return fromSharedPtrToNakedCast(v.ptr());
    }
	static int get(PyObject* obj, TRef& v)
	{
		if (obj == Py_None)
		{
			PyErr_SetString(PyExc_TypeError, "argument must be not be None");
			return 1;
		}
        typename TRef::TValuePtr p;
        if (pyGetSimpleObject(obj, p) != 0)
        {
            return 1;
        }
        v = TRef(p);
		return 0;
	}
};

}

namespace lass::python::impl
{

template <typename T>
struct ArgumentTraitsBuiltin< liar::kernel::PyObjectRef<T> >
{
    using TRef = liar::kernel::PyObjectRef<T>;
    using TStorage = NoNone<typename TRef::TValuePtr>;
    static TRef arg(const TStorage& storage) { return TRef(storage.reference()); }
};

}
