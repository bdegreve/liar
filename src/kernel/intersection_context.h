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

/** @class liar::kernel::IntersectionContext
 *  @brief contains local geometry context of intersection point
 *  @author Bram de Greve [BdG]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_INTERSECTION_CONTEXT_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_INTERSECTION_CONTEXT_H

#pragma once
#include "kernel_common.h"
#include "shader.h"

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL IntersectionContext
{
public:

    const TPoint3D& point() const { return point_; }
    const TVector3D& normal() const { return normal_; }
    const TVector3D& tangentU() const { return tangentU_; }
    const TVector3D& tangentV() const { return tangentV_; }
    const TScalar t() const { return t_; }
    const TScalar u() const { return u_; }
    const TScalar v() const { return v_; }
    const TShaderPtr& shader() const { return shader_; }

    void setPoint(const TPoint3D& iPoint) { point_ = iPoint; }
    void setNormal(const TVector3D& iNormal) { normal_ = iNormal; }
    void setTangentU(const TVector3D& iTangentU) { tangentU_ = iTangentU; }
    void setTangentV(const TVector3D& iTangentV) { tangentV_ = iTangentV; }
    void setT(TScalar iT) { t_ = iT; }
    void setU(TScalar iU) { u_ = iU; }
    void setV(TScalar iV) { v_ = iV; }
    void setShader(const TShaderPtr& iShader) { shader_ = iShader; }

	void transform(const TTransformation3D& iTransformation)
	{
		point_ = prim::transform(point_, iTransformation);
		normal_ = prim::normalTransform(normal_, iTransformation);
		tangentU_ = prim::transform(tangentU_, iTransformation);
		tangentV_ = prim::transform(tangentV_, iTransformation);
	}

	void translate(const TVector3D& iOffset)
	{
		point_ += iOffset;
	}

private:

    TPoint3D point_;
    TVector3D normal_;
    TVector3D tangentU_;
    TVector3D tangentV_;
    TScalar t_;
    TScalar u_;
    TScalar v_;
    TShaderPtr shader_;
};

}

}

#endif

// EOF
