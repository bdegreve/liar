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

/** @class liar::Projection
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_PROJECTION_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_PROJECTION_H

#include "kernel_common.h"

namespace liar
{
namespace kernel
{

class Projection;
typedef PyObjectRef<Projection> TProjectionRef;

class LIAR_KERNEL_DLL Projection: public python::PyObjectPlus
{
	PY_HEADER(python::PyObjectPlus)
public:

	typedef TPoint2D TUv;

	virtual ~Projection();

	const TRay3D ray(const TUv& uv, TScalar& pdf) const { return doRay(uv, pdf); }
	const TRay3D ray(TScalar u, TScalar v, TScalar& pdf) const { return doRay(TUv(u, v), pdf); }
	const TRay3D ray(TScalar u, TScalar v) const { TScalar pdf; return doRay(TUv(u, v), pdf); }

	const TUv uv(const TPoint3D& point, TRay3D& ray, TScalar& t) const { return doUv(point, ray, t); }

	const TPyObjectPtr reduce() const;
	const TPyObjectPtr getState() const { return doGetState(); }
	void setState(const TPyObjectPtr& state) { doSetState(state); }

protected:

	Projection();

private:

	virtual const TRay3D doRay(const TUv& uv, TScalar& pdf) const = 0;
	virtual const TUv doUv(const TPoint3D& point, TRay3D& ray, TScalar& t) const = 0;

	virtual const TPyObjectPtr doGetState() const = 0;
	virtual void doSetState(const TPyObjectPtr& state) = 0;
};

}

}

#endif

// EOF
