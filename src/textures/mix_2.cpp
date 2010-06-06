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

#include "textures_common.h"
#include "mix_2.h"
#include "constant.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(Mix2, "base class of textures mixing two input textures")
PY_CLASS_MEMBER_RW_DOC(Mix2, textureA, setTextureA, "first texture")
PY_CLASS_MEMBER_RW_DOC(Mix2, textureB, setTextureB, "second texture")

// --- public --------------------------------------------------------------------------------------

void Mix2::setTextureA(const TTexturePtr& a)
{
	a_ = a;
}



void Mix2::setTextureB(const TTexturePtr& b)
{
	b_ = b;
}



// --- protected -----------------------------------------------------------------------------------
/*
Mix2::Mix2():
	a_(new Constant(rgb(1, 0, 0))),
	b_(new Constant(rgb(0, 1, 0)))
{
}
*/


Mix2::Mix2(const TTexturePtr& a, const TTexturePtr& b):
	a_(a),
	b_(b)
{
}



// --- private -------------------------------------------------------------------------------------

const TPyObjectPtr Mix2::doGetState() const
{
	return python::makeTuple(a_, b_, doGetMixState());
}



void Mix2::doSetState(const TPyObjectPtr& state)
{
	TPyObjectPtr wrappedState;
	python::decodeTuple(state, a_, b_, wrappedState);
	doSetMixState(wrappedState);
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

