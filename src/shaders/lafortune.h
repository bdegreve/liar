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

/** @class liar::shaders::Lafortune
 *  @brief Implements Lafortune model.
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_SHADERS_LAFORTUNE_H
#define LIAR_GUARDIAN_OF_INCLUSION_SHADERS_LAFORTUNE_H

#include "shaders_common.h"
#include "../kernel/shader.h"
#include "../kernel/texture.h"
#include <lass/stde/static_vector.h>

namespace liar
{
namespace shaders
{

class LIAR_SHADERS_DLL Lafortune: public Shader
{
	PY_HEADER(Shader)
public:

	enum { capacity = 8 };

	class Lobe: public python::PyObjectPlus
	{
		PY_HEADER(python::PyObjectPlus)
	public:
		TTexturePtr x;
		TTexturePtr y;
		TTexturePtr z;
		TTexturePtr power;
		Lobe(TTexturePtr x, TTexturePtr y, TTexturePtr z, TTexturePtr power);

		const TPyObjectPtr reduce() const;
		const TPyObjectPtr getState() const;
		void setState(const TPyObjectPtr& state);
	};
	typedef python::PyObjectPtr<Lobe>::Type TLobePtr;
	typedef std::vector<TLobePtr> TLobes;

	Lafortune();
	Lafortune(const TTexturePtr& diffuse);
	Lafortune(const TTexturePtr& diffuse, const TLobes& lobes);

	const TTexturePtr& diffuse() const;
	void setDiffuse(const TTexturePtr& diffuse);

	const TLobes& lobes() const;
	void setLobes(const TLobes& lobes);
	void addLobe(TTexturePtr x, TTexturePtr y, TTexturePtr z, TTexturePtr power);

private:

	class LafortuneBsdf : public Bsdf
	{
	public:
		LafortuneBsdf(const Sample& sample, const IntersectionContext& context, TBsdfCaps caps, const Spectral& diffuse);
		void addLobe(const Spectral& x, const Spectral& y, const Spectral& z, const Spectral& power);
	private:
		typedef Spectral::TValue TValue;
		struct Lobe
		{
			Spectral x;
			Spectral y;
			Spectral z;
			Spectral power;
			Lobe(const Spectral& x, const Spectral& y, const Spectral& z, const Spectral& power) : x(x), y(y), z(z), power(power) {}
		};
		typedef stde::static_vector<Lobe, capacity> TLobes;
		BsdfOut doEvaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, TBsdfCaps allowedCaps) const;
		SampleBsdfOut doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, TBsdfCaps allowedCaps) const;
		Spectral eval(const TVector3D& omegaIn, const TVector3D& omegaOut) const;
		Spectral diffuseOverPi_;
		TLobes lobes_;
	};

	TBsdfPtr doBsdf(const Sample& sample, const IntersectionContext& context) const;

	const TPyObjectPtr doGetState() const;
	void doSetState(const TPyObjectPtr& state);

	TTexturePtr diffuse_;
	TLobes lobes_;
};


}

}

#endif

// EOF