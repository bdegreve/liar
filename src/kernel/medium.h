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

/** @class liar::Medium
 *  @brief base class of all media
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_KERNEL_MEDIUM_H
#define LIAR_GUARDIAN_OF_INCLUSION_KERNEL_MEDIUM_H

#include "kernel_common.h"
#include "differential_ray.h"
#include "spectral.h"
#include "solid_event.h"
#include "sampler.h"

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL Medium: public python::PyObjectPlus
{
	PY_HEADER(python::PyObjectPlus)
public:

	virtual ~Medium();

	const Spectral& refractionIndex() const;
	void setRefractionIndex(const Spectral& refractionIndex);

	size_t priority() const;
	void setPriority(size_t priority);

	void requestSamples(const TSamplerPtr& sampler);
	size_t numScatterSamples() const
	{
		return doNumScatterSamples();
	}
	Sampler::TSubSequenceId idStepSamples() const
	{
		return idStepSamples_;
	}
	Sampler::TSubSequenceId idLightSamples() const
	{
		return idLightSamples_;
	}
	Sampler::TSubSequenceId idSurfaceSamples() const
	{
		return idSurfaceSamples_;
	}

	const Spectral transmittance(const BoundedRay& ray) const
	{
		return doTransmittance(ray);
	}

	const Spectral emission(const BoundedRay& ray) const
	{
		return doEmission(ray);
	}

	const Spectral scatterOut(const BoundedRay& ray) const
	{
		return doScatterOut(ray);
	}

	/** @param tScatter [out] position along ray where photon hits particle
	 *  @returns attenuation due to transmition only, does not include scattering at tScatter.
	 */
	const Spectral sampleScatterOut(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const
	{
		return doSampleScatterOut(sample, ray, tScatter, pdf);
	}

	/** @param tScatter [out] position along ray where photon hits particle
	 *  @returns attenuation due to transmition only, does not include scattering at tScatter.
	 */
	const Spectral sampleScatterOutOrTransmittance(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const
	{
		return doSampleScatterOutOrTransmittance(sample, ray, tScatter, pdf);
	}

	/** @param dirIn: incoming direction @e towards @a position.
	 *  @param dirOut: incoming direction away from @a position.
	 */
	const Spectral phase(const TPoint3D& position, const TVector3D& dirIn, const TVector3D& dirOut) const
	{
		TScalar pdf;
		return doPhase(position, dirIn, dirOut, pdf);
	}
	const Spectral phase(const TPoint3D& position, const TVector3D& dirIn, const TVector3D& dirOut, TScalar& pdf) const
	{
		return doPhase(position, dirIn, dirOut, pdf);
	}

	const Spectral samplePhase(const TPoint2D& sample, const TPoint3D& position, const TVector3D& dirIn, TVector3D& dirOut, TScalar& pdf) const
	{
		return doSamplePhase(sample, position, dirIn, dirOut, pdf);
	}

protected:

	Medium();

private:

	virtual void doRequestSamples(const TSamplerPtr& sampler);
	virtual size_t doNumScatterSamples() const;
	virtual const Spectral doTransmittance(const BoundedRay& ray) const = 0;
	virtual const Spectral doEmission(const BoundedRay& ray) const = 0;
	virtual const Spectral doScatterOut(const BoundedRay& ray) const = 0;
	virtual const Spectral doSampleScatterOut(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const = 0;
	virtual const Spectral doSampleScatterOutOrTransmittance(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const = 0;
	virtual const Spectral doPhase(const TPoint3D& position, const TVector3D& dirIn, const TVector3D& dirOut, TScalar& pdf) const = 0;
	virtual const Spectral doSamplePhase(const TPoint2D& sample, const TPoint3D& position, const TVector3D& dirIn, TVector3D& dirOut, TScalar& pdf) const = 0;

	Spectral refractionIndex_;
	size_t priority_;
	Sampler::TSubSequenceId idStepSamples_;
	Sampler::TSubSequenceId idLightSamples_;
	Sampler::TSubSequenceId idSurfaceSamples_;
};

typedef python::PyObjectPtr<Medium>::Type TMediumPtr;



class LIAR_KERNEL_DLL MediumStack
{
public:
	MediumStack(const TMediumPtr& defaultMedium = TMediumPtr());
	const Medium* medium() const;
	const Spectral transmittance(const BoundedRay& ray) const;
	const Spectral transmittance(const BoundedRay& ray, TScalar farLimit) const;
	const Spectral transmittance(const DifferentialRay& ray, TScalar farLimit) const;
	const Spectral emission(const BoundedRay& ray) const;
	const Spectral sampleScatterOut(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const;
	const Spectral sampleScatterOutOrTransmittance(TScalar sample, const BoundedRay& ray, TScalar& tScatter, TScalar& pdf) const;
	const Spectral samplePhase(const TPoint2D& sample, const TPoint3D& position, const TVector3D& dirIn, TVector3D& dir, TScalar& pdf) const;

private:
	friend class MediumChanger;
	typedef std::vector<const Medium*> TStack;

	//void push(const Medium* medium);
	//void pop(const Medium* medium);

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
