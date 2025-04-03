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

#include "output_common.h"
#include "splitter.h"

namespace liar
{
namespace output
{

PY_DECLARE_CLASS_DOC(Splitter, "splits output stream to several render targets");
PY_CLASS_CONSTRUCTOR_0(Splitter)
PY_CLASS_CONSTRUCTOR_1(Splitter, const Splitter::TChildren&)
PY_CLASS_MEMBER_RW(Splitter, children, setChildren)
PY_CLASS_METHOD_QUALIFIED_1(Splitter, add, void, const TRenderTargetPtr&)
PY_CLASS_METHOD_QUALIFIED_1(Splitter, add, void, const Splitter::TChildren&)

// --- public --------------------------------------------------------------------------------------

Splitter::Splitter():
	children_()
{
}



Splitter::Splitter(const TChildren& children):
	children_(children)
{
}



const Splitter::TChildren& Splitter::children() const
{
	return children_;
}



void Splitter::setChildren(const TChildren& children)
{
	children_ = children;
}



void Splitter::add(const TRenderTargetPtr& child)
{
	children_.push_back(child);
}



void Splitter::add(const TChildren& children)
{
	children_.insert(children_.end(), children.begin(), children.end());
}



// --- private -------------------------------------------------------------------------------------

const TResolution2D Splitter::doResolution() const
{
	TResolution2D maxResolution(1, 1);
	for (TChildren::const_iterator i = children_.begin(); i != children_.end(); ++i)
	{
		maxResolution = pointwiseMax(maxResolution, (*i)->resolution());
	}
	return maxResolution;
}



void Splitter::doBeginRender()
{
	for (TChildren::const_iterator i = children_.begin(); i != children_.end(); ++i)
	{
		(*i)->beginRender();
	}
}



void Splitter::doWriteRender(const OutputSample* first, const OutputSample* last)
{
	for (TChildren::const_iterator i = children_.begin(); i != children_.end(); ++i)
	{
		(*i)->writeRender(first, last);
	}
}



void Splitter::doEndRender()
{
	for (TChildren::const_iterator i = children_.begin(); i != children_.end(); ++i)
	{
		(*i)->endRender();
	}
}



bool Splitter::doIsCanceling() const
{
	for (TChildren::const_iterator i = children_.begin(); i != children_.end(); ++i)
	{
		if ((*i)->isCanceling())
		{
			return true;
		}
	}
	return false;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
