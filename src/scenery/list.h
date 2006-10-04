/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2006  Bram de Greve
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

/** @class liar::scenery::List
 *  @brief compound of some child scenery in a linear list.
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SCENERY_LIST_H
#define LIAR_GUARDIAN_OF_INCLUSION_SCENERY_LIST_H

#include "scenery_common.h"
#include "../kernel/scene_object.h"

namespace liar
{
namespace scenery
{

class LIAR_SCENERY_DLL List: public SceneObject
{
    PY_HEADER(SceneObject)
public:

    typedef std::vector<TSceneObjectPtr> TChildren;

    List();
    List(const TChildren& children); // for python

    template <typename InputIterator> List(InputIterator first, InputIterator last):
        SceneComposite(&Type)
    {
        add(first, last);
    }

    void add(const TSceneObjectPtr& child);
	void add(const TChildren& children); // for python

	template <typename InputIterator> void add(InputIterator first, InputIterator last)
	{
        while (first != last)
		{
			this->add(*first++);
		}
	}

private:

    void doAccept(lass::util::VisitorBase& visitor);

	void doPreProcess(const TSceneObjectPtr& scene, const TimePeriod& period);
	void doIntersect(const Sample& sample, const BoundedRay& ray, 
		Intersection& result) const;
	const bool doIsIntersecting(const Sample& sample, const BoundedRay& ray) const;
	void doLocalContext(const Sample& sample, const BoundedRay& ray,
		const Intersection& intersection, IntersectionContext& result) const;
	const bool doContains(const Sample& sample, const TPoint3D& point) const;
	const TAabb3D doBoundingBox() const;
	virtual const TScalar doArea() const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

    TChildren children_;
};



}

}

#endif

// EOF
