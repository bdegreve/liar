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

/** @class liar::Camera
 *  @brief Abstract base class of render viewports
 *  @author Bram de Greve [Bramz]
 *
 *  Concrete cameras that to be used by LiAR should derive from this class.
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_CAMERA_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_CAMERA_H

#include "kernel_common.h"
#include "differential_ray.h"
#include "sample.h"
#include "time_period.h"

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL Camera: public python::PyObjectPlus
{
	PY_HEADER(python::PyObjectPlus)
public:

	virtual ~Camera();

	const DifferentialRay primaryRay(const Sample& sample, const TVector2D& screenSpaceDelta) const;
	const TimePeriod shutterDelta() const { return doShutterDelta(); }
	
	TScalar weight(const TRay3D& ray) { return doWeight(ray); }
	TScalar weight(const DifferentialRay& ray) { return doWeight(ray.centralRay().unboundedRay()); }

	TScalar asDepth(const TRay3D& ray, TScalar t) { return doAsDepth(ray, t); }
	TScalar asDepth(const DifferentialRay& ray, TScalar t) { return doAsDepth(ray.centralRay().unboundedRay(), t); }

	const TPyObjectPtr reduce() const;
	const TPyObjectPtr getState() const { return doGetState(); }
	void setState(const TPyObjectPtr& state) { doSetState(state); }

protected:

	Camera();

private:

	virtual const BoundedRay doGenerateRay(const Sample& sample, const TVector2D& screenSpaceDelta) const = 0;
	virtual const TimePeriod doShutterDelta() const = 0;
	virtual TScalar doWeight(const TRay3D& ray) const;
	virtual TScalar doAsDepth(const TRay3D& ray, TScalar t) const = 0;

	virtual const TPyObjectPtr doGetState() const = 0;
	virtual void doSetState(const TPyObjectPtr& state) = 0;
};

typedef python::PyObjectPtr<Camera>::Type TCameraPtr;

}

}

#endif

// EOF
