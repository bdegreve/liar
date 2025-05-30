/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2025  Bram de Greve (bramz@users.sourceforge.net)
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

/** @namespace liar
 *  @brief LiAR isn't a raytracer
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

/** @namespace liar::kernel
 *  @brief namespace with core elements of LiAR
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_KERNEL_COMMON_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_KERNEL_COMMON_H

// maybe Bill Gates likes to global lock iterator operations, but I don't.
#ifdef _MSC_VER
//#	define _SECURE_SCL 0
//#	define _HAS_ITERATOR_DEBUGGING 0
#endif

#include <lass/num/num_traits.h>
#include <lass/num/basic_ops.h>
#include <lass/num/floating_point_comparison.h>
#include <lass/num/random.h>
#include <lass/num/distribution.h>
#include <lass/prim/vector_2d.h>
#include <lass/prim/vector_3d.h>
#include <lass/prim/point_2d.h>
#include <lass/prim/point_3d.h>
#include <lass/prim/aabb_2d.h>
#include <lass/prim/aabb_3d.h>
#include <lass/prim/sphere_3d.h>
#include <lass/prim/ray_3d.h>
#include <lass/prim/color_rgba.h>
#include <lass/prim/transformation_2d.h>
#include <lass/prim/transformation_3d.h>
#include <lass/python/python_api.h>
#include <lass/python/streams.h>
#include <lass/prim/impl/plane_3d_impl_detail.h>

#include "config.h"
#include "pyobject_ref.h"

#ifdef libkernel_EXPORTS
#	define LIAR_KERNEL_DLL LASS_DLL_EXPORT
#else
#	define LIAR_KERNEL_DLL LASS_DLL_IMPORT
#endif

#define LIAR_VERSION_FULL LASS_STRINGIFY(LIAR_VERSION_MAJOR) "."\
	LASS_STRINGIFY(LIAR_VERSION_MINOR) "." LASS_STRINGIFY(LIAR_VERSION_REVISION)

#ifdef LIAR_DEBUG
#	define LIAR_ASSERT( predicate, message ) \
		do\
		{\
			if (!(predicate))\
			{\
				LASS_CERR << "\nASSERT FAILURE: " LASS_HERE ": " LASS_PRETTY_FUNCTION ": " LASS_STRINGIFY(predicate) << ": " << message << "\n";\
				LASS_BREAK_HERE;\
			}\
		}\
		while (false)
#else
#	define LIAR_ASSERT( predicate, message )
#endif
#define LIAR_ASSERT_FINITE( x ) LIAR_ASSERT( ::liar::kernel::isFinite(x), (x) )
#define LIAR_ASSERT_POSITIVE_FINITE( x ) LIAR_ASSERT( ::liar::kernel::isPositiveAndFinite(x), (x) )
#define LIAR_ASSERT_NORMALIZED( x ) LIAR_ASSERT( ::liar::kernel::isNormalized(x), LASS_STRINGIFY(x) << "=" << x << " norm=" << x.norm() )

#if LASS_COMPILER_TYPE == LASS_COMPILER_TYPE_MSVC
#	define LIAR_FORCE_INLINE __forceinline
#else
#	define LIAR_FORCE_INLINE inline __attribute__((always_inline))
#endif
namespace liar
{

using namespace lass;

typedef LIAR_SCALAR TScalar;
typedef LIAR_TIME TTime;
typedef LIAR_TIME_DELTA TTimeDelta;
typedef LIAR_WAVELENGTH TWavelength;

typedef num::NumTraits<TScalar> TNumTraits;

inline TScalar operator""_s(long double x) { return static_cast<TScalar>(x); }

typedef prim::Vector2D<TScalar> TVector2D;
typedef prim::Point2D<TScalar> TPoint2D;
typedef prim::Vector3D<TScalar> TVector3D;
typedef prim::Point3D<TScalar> TPoint3D;
typedef prim::Ray3D<TScalar, prim::Normalized, prim::Unbounded> TRay3D;
typedef prim::Aabb3D<TScalar> TAabb3D;
typedef prim::Sphere3D<TScalar> TSphere3D;
typedef prim::Transformation2D<TScalar> TTransformation2D;
typedef prim::Transformation3D<TScalar> TTransformation3D;

typedef prim::Vector2D<size_t> TResolution2D;
typedef prim::Aabb2D<size_t> TRange2D;

typedef python::PyObjectPtr<PyObject>::Type TPyObjectPtr;

LIAR_KERNEL_DLL extern TScalar tolerance;

const std::string name = LIAR_NAME_FULL;
const std::string version = LIAR_VERSION_FULL;
const std::string authors = LIAR_AUTHORS;
const std::string website = LIAR_WEBSITE;

const std::string license =
	LIAR_NAME_FULL "\n"
    "Copyright (C) 2004-2023 " LIAR_AUTHORS "\n"
	"\n"
	"This is free software; you can redistribute it and/or modify\n"
	"it under the terms of the GNU General Public License as published by\n"
	"the Free Software Foundation; either version 2 of the License, or\n"
	"(at your option) any later version.\n"
	"\n"
	"This program is distributed in the hope that it will be useful,\n"
	"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	"GNU General Public License for more details.\n"
	"\n"
	"You should have received a copy of the GNU General Public License\n"
	"along with this program; if not, write to the Free Software\n"
	"Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n"
    "\n"
    LIAR_WEBSITE "\n";



namespace kernel
{

template <typename T>
bool russianRoulette(T& subject, TScalar pdf, TScalar sample)
{
	if (pdf <= 0 || sample > pdf)
	{
		return false;
	}
	subject /= static_cast<LIAR_VALUE>(std::min(pdf, TNumTraits::one));
	return true;
}

/** generate an orthonormal basis <i,j,k>, given k.
 */
inline void generateOrthonormal(const TVector3D& k, TVector3D& i, TVector3D& j)
{
	prim::impl::Plane3DImplDetail::generateDirections(k, i, j);
	i.normalize();
	j.normalize();
}


/** Uniform distribution over real right-open interval [a, b)
 *
 * This is a wrapper around std::uniform_real_distribution that ensures that the result is always less than b.
 * Although std::uniform_real_distribution claims to be right-open, most implementations can still return b.
 * https://github.com/microsoft/STL/issues/1074
 */
template <typename T>
class UniformRealDistribution: public std::uniform_real_distribution<T>
{
public:
	using result_type = typename std::uniform_real_distribution<T>::result_type;
	UniformRealDistribution(result_type a = 0, result_type b = 1): std::uniform_real_distribution<T>(a, b) {}
	template< class Generator >
	result_type operator()(Generator& generator)
	{
		const auto b = this->b();
		while (true)
		{
			const auto x = std::uniform_real_distribution<T>::operator()(generator);
			if (x < b)
			{
				return x;
			}
		}
	}
};


template <typename RandomAccessIterator, typename Generator>
void stratifier1D(RandomAccessIterator first, RandomAccessIterator last, Generator& generator)
{
	UniformRealDistribution<TNumTraits::baseType> uniform;
	const std::ptrdiff_t n = last - first;
	const TScalar scale = num::inv(static_cast<TScalar>(n));
	for (std::ptrdiff_t k = 0; k < n; ++k)
	{
		first[k] = scale * (static_cast<TScalar>(k) + uniform(generator));
	};
	std::shuffle(first, last, generator);
}

template <typename RandomAccessRange, typename Generator>
void stratifier1D(RandomAccessRange& range, Generator& generator)
{
	stratifier1D(range.begin(), range.end(), generator);
}

template <typename RandomAccessIterator, typename Generator>
void latinHypercube2D(RandomAccessIterator first, RandomAccessIterator last, Generator& generator)
{
	UniformRealDistribution<TNumTraits::baseType> uniform;
	const std::ptrdiff_t n = last - first;
	const TPoint2D::TValue scale = num::inv(static_cast<TPoint2D::TValue>(n));
	for (std::ptrdiff_t k = 0; k < n; ++k)
	{
		first[k].x = scale * (static_cast<TScalar>(k) + uniform(generator));
		first[k].y = scale * (static_cast<TScalar>(k) + uniform(generator));
	};
	std::shuffle(stde::member_iterator(first, &TPoint2D::x), stde::member_iterator(last, &TPoint2D::x), generator);
	std::shuffle(stde::member_iterator(first, &TPoint2D::y), stde::member_iterator(last, &TPoint2D::y), generator);
}

template <typename RandomAccessRange, typename Generator>
void latinHypercube2D(RandomAccessRange& range, Generator& generator)
{
	latinHypercube2D(range.begin(), range.end(), generator);
}

inline TScalar sphericalPhi(const TVector3D& v)
{
	if (v.x == 0 && v.y == 0)
	{
		return 0;
	}
	const TScalar phi = num::atan2(v.y, v.x);
	if (phi < 0)
	{
		return phi + 2 * TNumTraits::pi;
	}
	return phi;
}

inline TScalar sphericalTheta(const TVector3D& v)
{
	LASS_ASSERT(v.z >= -1 && v.z <= 1);
	return num::acos(v.z);
}

template <typename T>
bool isFinite(T x)
{
	return !num::isInf(x) && !num::isNaN(x);
}
template <typename T>
inline bool isFinite(const prim::Vector2D<T>& x)
{
	return isFinite(x.squaredNorm());
}
template <typename T>
inline bool isFinite(const prim::Vector3D<T>& x)
{
	return isFinite(x.squaredNorm());
}

template <typename T>
bool isPositiveAndFinite(T x)
{
	return x >= 0 && !num::isInf(x) && !num::isNaN(x);
}

template <typename T>
bool isNormalized(const T &x)
{
	typedef typename T::TValue TValue;
	return num::abs(x.squaredNorm() - static_cast<TValue>(1)) < static_cast<TValue>(1e-6f);
}

}

}

#include "lass_exports.h"

#endif

// EOF
