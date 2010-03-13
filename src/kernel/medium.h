/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::Medium
 *  @brief base class of all media
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_MEDIUM_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_MEDIUM_H

#include "kernel_common.h"
#include "differential_ray.h"
#include "xyz.h"
#include "solid_event.h"

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL Medium: public python::PyObjectPlus
{
	PY_HEADER(python::PyObjectPlus)
public:

	virtual ~Medium();

	const XYZ& refractionIndex() const;
	void setRefractionIndex(const XYZ& refractionIndex);
	
	size_t priority() const;
	void setPriority(size_t priority);

	const XYZ transparency(const BoundedRay& ray) const
	{
		return doTransparency(ray); 
	}

protected:

	Medium();

private:

	virtual const XYZ doTransparency(const BoundedRay& ray) const = 0;

	XYZ refractionIndex_;
	size_t priority_;
};

typedef python::PyObjectPtr<Medium>::Type TMediumPtr;



class LIAR_KERNEL_DLL MediumStack
{
public:
	MediumStack(const TMediumPtr& defaultMedium = TMediumPtr());
	const XYZ transparency(const BoundedRay& ray) const;
	const XYZ transparency(const BoundedRay& ray, TScalar farLimit) const;
	const XYZ transparency(const DifferentialRay& ray, TScalar farLimit) const;
private:
	friend class MediumChanger;
	typedef std::vector<const Medium*> TStack;
	
	//void push(const Medium* medium);
	//void pop(const Medium* medium);
	const Medium* top() const;

	TStack stack_;
	TMediumPtr default_;
};

class LIAR_KERNEL_DLL MediumChanger
{
public:
	MediumChanger(MediumStack& stack, const Medium* medium, SolidEvent solidEvent);
	~MediumChanger();
private:
	MediumStack::TStack& stack_;
	const Medium* medium_;
	SolidEvent event_;
};

}

}

#endif

// EOF

