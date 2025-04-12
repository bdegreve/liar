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

/** @class liar::Texture
 *  @brief abstract base class of all textures
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_TEXTURE_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_TEXTURE_H

#include "kernel_common.h"
#include "intersection_context.h"
#include "sample.h"
#include "spectral.h"
#include "xyz.h"

namespace liar
{
namespace kernel
{

class Texture;
typedef python::PyObjectPtr<Texture>::Type TTexturePtr;
typedef PyObjectRef<Texture> TTextureRef;

class LIAR_KERNEL_DLL Texture: public python::PyObjectPlus
{
	PY_HEADER(python::PyObjectPlus)
public:
	typedef Spectral::TValue TValue;

	virtual ~Texture();

	const Spectral lookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const
	{
		return doLookUp(sample, context, type);
	}

	TValue scalarLookUp(const Sample& sample, const IntersectionContext& context) const
	{
		return doScalarLookUp(sample, context);
	}

	bool isChromatic() const;

	static const TTextureRef& black();
	static const TTextureRef& white();

	const TPyObjectPtr reduce() const;
	const TPyObjectPtr getState() const;
	void setState(const TPyObjectPtr& state);

protected:

	Texture();

	virtual const TPyObjectPtr doGetState() const = 0;
	virtual void doSetState(const TPyObjectPtr& state) = 0;

private:

	virtual const Spectral doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const = 0;
	virtual TValue doScalarLookUp(const Sample& sample, const IntersectionContext& context) const = 0;
	virtual bool doIsChromatic() const = 0;

	static TTextureRef black_;
	static TTextureRef white_;
};

// --- implementation ------------------------------------------------------------------------------

namespace impl
{
	class LIAR_KERNEL_DLL TextureBlack: public Texture
	{
		PY_HEADER(Texture);
	public:
		TextureBlack();
	private:
		const Spectral doLookUp(const Sample&, const IntersectionContext&, SpectralType) const override;
		TValue doScalarLookUp(const Sample&, const IntersectionContext&) const override;
		bool doIsChromatic() const override;
		const TPyObjectPtr doGetState() const override;
		void doSetState(const TPyObjectPtr&) override;
	};

	class LIAR_KERNEL_DLL TextureWhite: public Texture
	{
		PY_HEADER(Texture);
	public:
		TextureWhite();
	private:
		const Spectral doLookUp(const Sample&, const IntersectionContext&, SpectralType) const override;
		TValue doScalarLookUp(const Sample&, const IntersectionContext&) const override;
		bool doIsChromatic() const override;
		const TPyObjectPtr doGetState() const override;
		void doSetState(const TPyObjectPtr&) override;
	};
}

}

}

#endif

// EOF
