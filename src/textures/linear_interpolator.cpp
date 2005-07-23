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

#include "textures_common.h"
#include "linear_interpolator.h"
#include <lass/stde/extended_iterator.h>

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS(LinearInterpolator)
PY_CLASS_CONSTRUCTOR_0(LinearInterpolator)
PY_CLASS_CONSTRUCTOR_2(LinearInterpolator, const LinearInterpolator::TKeyTextures&, const kernel::TTexturePtr&)
PY_CLASS_MEMBER_RW(LinearInterpolator, "keys", keys, setKeys)
PY_CLASS_MEMBER_RW(LinearInterpolator, "control", control, setControl)
PY_CLASS_METHOD(LinearInterpolator, addKey)

// --- public --------------------------------------------------------------------------------------

LinearInterpolator::LinearInterpolator():
	Texture(&Type),
	keys_(),
	control_(kernel::Texture::black())
{
	keys_.push_back(TKeyTexture(TNumTraits::zero, kernel::Texture::white()));
}



LinearInterpolator::LinearInterpolator(const TKeyTextures& iKeyTextures, 
									   const kernel::TTexturePtr& iControlTexture):
	Texture(&Type),
	keys_(),
	control_(iControlTexture)
{
	setKeys(iKeyTextures);
}



/** return list of key textures 
 */
const LinearInterpolator::TKeyTextures& LinearInterpolator::keys() const
{
	return keys_;
}



/** return control texture
 */
const kernel::TTexturePtr& LinearInterpolator::control() const
{
	return control_;
}



/** set list of key textures
 */
void LinearInterpolator::setKeys(const TKeyTextures& iKeyTextures)
{
	keys_ = iKeyTextures;
	std::sort(keys_.begin(), keys_.end(), LesserKey());
}



/** set control texture
 */
void LinearInterpolator::setControl(const kernel::TTexturePtr& iContolTexture)
{
	control_ = iContolTexture;
}



/** add a key texture to the list
 */
void LinearInterpolator::addKey(const TScalar iKeyValue, 
								const kernel::TTexturePtr& iKeyTexture)
{
	TKeyTexture key(iKeyValue, iKeyTexture);
	TKeyTextures::iterator i = std::lower_bound(keys_.begin(), keys_.end(), key, LesserKey());
	keys_.insert(i, key);
}



// --- protected -----------------------------------------------------------------------------------




// --- private -------------------------------------------------------------------------------------

kernel::Spectrum LinearInterpolator::doLookUp(const kernel::Sample& iSample, 
										const kernel::IntersectionContext& iContext) const
{
	const TScalar controlValue = control_->lookUp(iSample, iContext).averagePower();

	TKeyTexture sentinel(controlValue, kernel::TTexturePtr());
	TKeyTextures::const_iterator i = std::lower_bound(keys_.begin(), keys_.end(), sentinel, LesserKey());
	
	if (i == keys_.begin())
	{
		return keys_.front().second->lookUp(iSample, iContext);
	}
	if (i == keys_.end())
	{
		return keys_.back().second->lookUp(iSample, iContext);
	}
	
	TKeyTextures::const_iterator prevI = stde::prior(i);
	LASS_ASSERT(prevI->first != i->first); // due to lower_bound

	const TTime blendFactor = (iSample.time() - prevI->first) / (i->first - prevI->first);
	
	return kernel::blend(
		prevI->second->lookUp(iSample, iContext),
		i->second->lookUp(iSample, iContext),
		static_cast<TScalar>(blendFactor));
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

