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

#include "textures_common.h"
#include "linear_interpolator.h"
#include <lass/stde/extended_iterator.h>

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(LinearInterpolator, "interpolates textures using gray value of control texture as parameter")
PY_CLASS_CONSTRUCTOR_0(LinearInterpolator)
PY_CLASS_CONSTRUCTOR_2(LinearInterpolator, const LinearInterpolator::TKeyTextures&, const TTexturePtr&)
PY_CLASS_MEMBER_RW(LinearInterpolator, keys, setKeys)
PY_CLASS_MEMBER_RW(LinearInterpolator, control, setControl)
PY_CLASS_METHOD(LinearInterpolator, addKey)

// --- public --------------------------------------------------------------------------------------

LinearInterpolator::LinearInterpolator():
	keys_(),
	control_(Texture::black())
{
	keys_.push_back(TKeyTexture(TNumTraits::zero, Texture::white()));
}



LinearInterpolator::LinearInterpolator(const TKeyTextures& keyTextures, 
									   const TTexturePtr& controlTexture):
	keys_(),
	control_(controlTexture)
{
	setKeys(keyTextures);
}



/** return list of key textures 
 */
const LinearInterpolator::TKeyTextures& LinearInterpolator::keys() const
{
	return keys_;
}



/** return control texture
 */
const TTexturePtr& LinearInterpolator::control() const
{
	return control_;
}



/** set list of key textures
 */
void LinearInterpolator::setKeys(const TKeyTextures& keyTextures)
{
	keys_ = keyTextures;
	std::sort(keys_.begin(), keys_.end(), LesserKey());
}



/** set control texture
 */
void LinearInterpolator::setControl(const TTexturePtr& iContolTexture)
{
	control_ = iContolTexture;
}



/** add a key texture to the list
 */
void LinearInterpolator::addKey(const TScalar keyValue, 
								const TTexturePtr& keyTexture)
{
	TKeyTexture key(keyValue, keyTexture);
	TKeyTextures::iterator i = std::lower_bound(keys_.begin(), keys_.end(), key, LesserKey());
	keys_.insert(i, key);
}



// --- protected -----------------------------------------------------------------------------------




// --- private -------------------------------------------------------------------------------------

const Spectrum 
LinearInterpolator::doLookUp(const Sample& sample, const IntersectionContext& context) const
{
	const TScalar keyValue = control_->lookUp(sample, context).average();

	TKeyTexture sentinel(keyValue, TTexturePtr());
	TKeyTextures::const_iterator i = std::lower_bound(keys_.begin(), keys_.end(), sentinel, LesserKey());
	
	if (i == keys_.begin())
	{
		return keys_.front().second->lookUp(sample, context);
	}
	if (i == keys_.end())
	{
		return keys_.back().second->lookUp(sample, context);
	}
	
	TKeyTextures::const_iterator prevI = stde::prev(i);
	LASS_ASSERT(prevI->first != i->first); // due to lower_bound

	const TScalar t = (keyValue - prevI->first) / (i->first - prevI->first);
	
	return lerp(
		prevI->second->lookUp(sample, context),
		i->second->lookUp(sample, context),
		t);
}



const TPyObjectPtr LinearInterpolator::doGetState() const
{
	return python::makeTuple(keys_, control_);
}



void LinearInterpolator::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, keys_, control_);
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

