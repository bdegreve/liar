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

#include "kernel_common.h"
#include "medium.h"

namespace liar
{
namespace kernel
{

PY_DECLARE_CLASS(Medium)

// --- public --------------------------------------------------------------------------------------

Medium::~Medium()
{
}



const Spectrum& Medium::refractionIndex() const
{
	return refractionIndex_;
}



void Medium::setRefractionIndex(const Spectrum& refractionIndex)
{
	refractionIndex_ = refractionIndex;
}



const unsigned Medium::priority() const
{
	return priority_;
}



void Medium::setPriority(unsigned priority)
{
	priority_ = priority;
}



// --- protected -----------------------------------------------------------------------------------

Medium::Medium():
	refractionIndex_(1),
	priority_(0)
{
}



// --- private -------------------------------------------------------------------------------------



// --- free ----------------------------------------------------------------------------------------



// -------------------------------------------------------------------------------------------------

MediumStack::MediumStack(const TMediumPtr& defaultMedium):
	stack_(),
	default_(defaultMedium)
{
}



const Spectrum MediumStack::transparency(const BoundedRay& ray) const
{
	return top() ? top()->transparency(ray) : Spectrum(1);
}



void MediumStack::push(const Medium* medium)
{
	struct LessPriority
	{
		bool operator()(const Medium* a, const Medium* b) const
		{
			return a->priority() < b->priority();
		}
	};
	TStack::iterator i = std::upper_bound(stack_.begin(), stack_.end(), medium);
	stack_.insert(i, medium);
}


void MediumStack::pop(const Medium* medium)
{
	
}



const Medium* const MediumStack::top() const
{
	return stack_.empty() ? default_.get() : stack_.back();
}



// -------------------------------------------------------------------------------------------------

MediumChanger::MediumChanger(MediumStack& stack, const Medium* medium, SolidEvent solidEvent):
	stack_(stack.stack_),
	medium_(medium),
	event_(solidEvent)
{
	if (medium_)
	{
		switch (event_)
		{
		case seEntering:
			stack_.push_back(medium_);
			break;
		case seLeaving:
			if (stack_.empty())
			{
				// our first generation view ray is leaving an object.  Regard this as a nop.
				event_ = seNoEvent;
			}
			else
			{
				LASS_ASSERT(stack_.back() == medium_);
				stack_.pop_back();
			}
			break;
		default:
			break;
		}
	}
}



MediumChanger::~MediumChanger()
{
	if (medium_)
	{
		switch (event_)
		{
		case seEntering:
			LASS_ASSERT(!stack_.empty() && stack_.back() == medium_);
			stack_.pop_back();
			break;
		case seLeaving:
			stack_.push_back(medium_);
			break;
		default:
			break;
		};
	}
}

}

}

// EOF