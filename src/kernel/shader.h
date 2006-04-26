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

/** @class liar::Shader
 *  @brief base class of all shaders
 *  @author Bram de Greve [BdG]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SHADER_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_SHADER_H

#include "kernel_common.h"
#include "spectrum.h"

namespace liar
{
namespace kernel
{

class DifferentialRay;
class Intersection;
class IntersectionContext;
class RayTracer;
class Sample;
class Sampler;
class SceneObject;
class LightContext;

typedef python::PyObjectPtr<Sampler>::Type TSamplerPtr;
typedef python::PyObjectPtr<SceneObject>::Type TSceneObjectPtr;

class LIAR_KERNEL_DLL Shader: public python::PyObjectPlus
{
    PY_HEADER(python::PyObjectPlus)
public:

    virtual ~Shader();

	const Spectrum shade(const Sample& iSample, const DifferentialRay& iRay, 
		const Intersection& iIntersection, const IntersectionContext& iContext) const
	{
		return doShade(iSample, iRay, iIntersection, iContext);
	}

	void requestSamples(const TSamplerPtr& iSampler);

	const TPyObjectPtr reduce() const;
	const TPyObjectPtr getState() const;
	void setState(const TPyObjectPtr& iState);

protected:

    Shader(PyTypeObject* iType);

private:

	virtual const Spectrum doShade(const Sample& iSample, const DifferentialRay& iRay, 
		const Intersection& iIntersection, const IntersectionContext& iContext) const = 0;

	virtual void doRequestSamples(const TSamplerPtr& iSampler);

	virtual const TPyObjectPtr doGetState() const = 0;
	virtual void doSetState(const TPyObjectPtr& iState) = 0;
};

typedef python::PyObjectPtr<Shader>::Type TShaderPtr;

}

}

#endif

// EOF

