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

/** @class liar::scenery::Csg
 *  @brief Constructive Solid Geometry
 *
 *  Can perform all three CSG operations:
 *	@arg UNION: A + B
 *	@arg INTERSECTION: A * B
 *	@arg DIFFERENCE: A - B
 *
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_CSG_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_CSG_H

#include "scenery_common.h"
#include "../kernel/scene_object.h"
#include <lass/util/dictionary.h>

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL Csg: public SceneObject
{
    PY_HEADER(SceneObject)
public:

	Csg(const TSceneObjectPtr& iA, const TSceneObjectPtr& iB);
	Csg(const TSceneObjectPtr& iA, const TSceneObjectPtr& iB,
		const std::string& iOperation);

	const TSceneObjectPtr& childA() const;
	const TSceneObjectPtr& childB() const;
	const std::string operation() const;

	void setChildA(const TSceneObjectPtr& iChild);	
	void setChildB(const TSceneObjectPtr& iChild);
	void setOperation(const std::string& iOperation);

private:

	enum Operation
	{
		oUnion = 0,
		oIntersection,
		oDifference,
		numOperation
	};

	typedef util::Dictionary<std::string, Operation> TOperationDictionary;

    void doAccept(lass::util::VisitorBase& ioVisitor);

	void doPreProcess(const TimePeriod& iPeriod);
	void doIntersect(const Sample& iSample, const BoundedRay& iRay, 
		Intersection& oResult) const;
	const bool doIsIntersecting(const Sample& iSample, const BoundedRay& iRay) const;
	void doLocalContext(const Sample& iSample, const BoundedRay& iRay,
		const Intersection& iIntersection, IntersectionContext& oResult) const;
	void doLocalSpace(TTime iTime, TTransformation3D& ioLocalToWorld) const;
	const bool doContains(const Sample& iSample, const TPoint3D& iPoint) const;
	const TAabb3D doBoundingBox() const;
	const TScalar doArea() const;

	static TOperationDictionary makeOperationDictionary();

    TSceneObjectPtr childA_;
    TSceneObjectPtr childB_;
	Operation operation_;

	static TOperationDictionary operationDictionary_;
};



}

}

#endif

// EOF
