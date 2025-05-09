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

/** @class liar::scenery::Csg
 *  @brief Constructive Solid Geometry
 *
 *  Can perform all three CSG operations:
 *	@arg UNION: A + B
 *	@arg INTERSECTION: A * B
 *	@arg DIFFERENCE: A - B
 *
 *  @author Bram de Greve [Bramz]
 */

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

	Csg(const TSceneObjectRef& a, const TSceneObjectRef& b);
	Csg(const TSceneObjectRef& a, const TSceneObjectRef& b,
		const std::string& iOperation);

	const TSceneObjectRef& childA() const;
	const TSceneObjectRef& childB() const;
	const std::string operation() const;

	void setChildA(const TSceneObjectRef& child);
	void setChildB(const TSceneObjectRef& child);
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

	void doAccept(lass::util::VisitorBase& visitor) override;

	void doPreProcess(const TimePeriod& period) override;
	void doIntersect(const Sample& sample, const BoundedRay& ray,
		Intersection& result) const override;
	bool doIsIntersecting(const Sample& sample, const BoundedRay& ray) const override;
	void doLocalContext(const Sample& sample, const BoundedRay& ray,
		const Intersection& intersection, IntersectionContext& result) const override;
	void doLocalSpace(TTime time, TTransformation3D& localToWorld) const override;
	bool doContains(const Sample& sample, const TPoint3D& point) const override;
	const TAabb3D doBoundingBox() const override;
	TScalar doArea() const override;
	TScalar doArea(const TVector3D& normal) const override;

	const TPyObjectPtr doGetState() const override;
	void doSetState(const TPyObjectPtr& state) override;

	static TOperationDictionary makeOperationDictionary();

	TSceneObjectRef childA_;
	TSceneObjectRef childB_;
	Operation operation_;

	static TOperationDictionary operationDictionary_;
};



}

}

#endif

// EOF
