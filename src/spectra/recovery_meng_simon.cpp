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

#include "spectra_common.h"
#include "recovery_meng_simon.h"

#include <lass/spat/mesh_interpolator.h>

namespace liar
{
namespace spectra
{

PY_DECLARE_CLASS_DOC(RecoveryMengSimon, "spectral recovery using Meng-Simon (2015)")
	PY_CLASS_CONSTRUCTOR_2(RecoveryMengSimon, const RecoveryMengSimon::TWavelengths&, const RecoveryMengSimon::TSamples&)
	PY_CLASS_METHOD_QUALIFIED_1(RecoveryMengSimon, recover, RecoveryMengSimon::TValues, const XYZ&)
	PY_CLASS_METHOD(RecoveryMengSimon, meshEdges)

struct RecoveryMengSimon::Impl
{
	struct PointInfo
	{
		TValues values;
		TValue max;
	};

	typedef spat::PlanarMesh<TValue, const PointInfo*, meta::EmptyType, meta::EmptyType> TMesh;
	typedef std::vector<PointInfo> TPointInfos;
	typedef prim::Aabb2D<TValue> TAabb2D;

	struct EdgeCollector
	{
		mutable RecoveryMengSimon::TEdges edges;
		bool call(TMesh::TEdge* e) const
		{
			const XY tail = TMesh::fastOrg(e);
			const XY head = TMesh::fastDest(e);
			edges.push_back(std::make_pair(tail, head));
			return true;
		}
	};

	Impl(const TWavelengths& wavelengths, const TSamples& samples);

	bool recover(const XYZ& xyz, const Impl::PointInfo* pi[3], TValue w[3]) const;
	
	const TAabb2D aabb{ XY{ -0.3f, -0.3f }, XY{ 1.0f, 1.2f } };
	const TWavelengths wavelengths;
	const TSamples samples;
	TMesh mesh;
	TPointInfos pointInfos;
	PointInfo zero;
};


// --- public --------------------------------------------------------------------------------------

RecoveryMengSimon::RecoveryMengSimon(const TWavelengths& wavelengths, const TSamples& samples) :
	pimpl_(new Impl{ wavelengths, samples })
{
}


RecoveryMengSimon::~RecoveryMengSimon()
{
}


RecoveryMengSimon::TValues RecoveryMengSimon::recover(const XYZ& xyz) const
{
	const size_t n = pimpl_->wavelengths.size();
	TValues r(n, 0);

	const Impl::PointInfo* pi[3];
	TValue w[3];
	if (!pimpl_->recover(xyz, pi, w))
	{
		return r;
	}

	const TValues& a = pi[0]->values;
	const TValues& b = pi[1]->values;
	const TValues& c = pi[2]->values;
	for (size_t k = 0; k < n; ++k)
	{
		r[k] = w[0] * a[k] + w[1] * b[k] + w[2] * c[k];
	}

	return r;
}


RecoveryMengSimon::TEdges RecoveryMengSimon::meshEdges() const
{
	Impl::EdgeCollector collector;
	if (!pimpl_->mesh.forAllPrimaryUndirectedEdges(Impl::TMesh::TEdgeCallback(&collector, &Impl::EdgeCollector::call)))
	{
		LASS_THROW("internal error.");
	}
	return collector.edges;
}


// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

Spectral RecoveryMengSimon::doRecover(const XYZ& xyz, const Sample& sample, SpectralType type) const
{
	const Impl::PointInfo* pi[3];
	TValue w[3];
	if (!pimpl_->recover(xyz, pi, w))
	{
		return Spectral(0);
	}
	const TValues& a = pi[0]->values;
	const TValues& b = pi[1]->values;
	const TValues& c = pi[2]->values;

#if LIAR_SPECTRAL_MODE_SINGLE
	const TWavelengths& ws = pimpl_->wavelengths;
	const size_t k = std::upper_bound(ws.begin(), ws.end(), sample.wavelength()) - ws.begin();
	if (k == 0 || k == ws.size())
	{
		return Spectral(0);
	}

	LASS_ASSERT(ws[k] > ws[k - 1]);
	const TWavelength t = (sample.wavelength() - ws[k - 1]) / (ws[k] - ws[k - 1]);
	
	const TValue v0 = w[0] * a[k - 1] + w[1] * b[k - 1] + w[2] * c[k - 1];
	const TValue v1 = w[0] * a[k]     + w[1] * b[k]     + w[2] * c[k];
	const TValue v = num::lerp(v0, v1, static_cast<TValue>(t));

	if (type == Reflectant)
	{
		// maximum of interpolated spectrum is not necessarely interpolated maximum
		// but interpolated maximum should be an overestimation, so it should be still valid.
		const TValue max = w[0] * pi[0]->max + w[1] * pi[1]->max + w[2] * pi[2]->max;
		LASS_ASSERT(max >= v);
		if (max > 1)
		{
			return Spectral(v / max);
		}
	}
	return Spectral(v);
#else
	const size_t n = pimpl_->wavelengths.size();
	TValues r(n, 0);
	for (size_t k = 0; k < n; ++k)
	{
		r[k] += w[0] * a[k] + w[1] * b[k] + w[2] * c[k];
	}
	return Spectral::fromSampled(pimpl_->wavelengths, r, sample, type);
#endif
}


RecoveryMengSimon::Impl::Impl(const TWavelengths& ws, const TSamples& ss):
	wavelengths(ws),
	samples(ss),
	mesh(aabb)
{
	// validate inputs.
	const size_t n = wavelengths.size();
	if (n < 2)
	{
		LASS_THROW("Requires at least two wavelengths.");
	}
	for (size_t k = 1; k < n; ++k)
	{
		if (wavelengths[k] <= wavelengths[k - 1])
		{
			LASS_THROW("Requires strict increasing wavelengths.");
		}
	}

	pointInfos.resize(samples.size()); // resize only once!

	size_t k = 0;
	for (const auto& s : samples)
	{
		const XY xy = s.first;
		const TValues& values = s.second;
		if (!aabb.contains(xy))
		{
			LASS_THROW("chromaticities must be in box [0, 1]x[0, 1].");
		}
		if (values.size() != n)
		{
			LASS_THROW("Requires as many values as wavelengths.");
		}

		LASS_ASSERT(k < samples.size());
		PointInfo* pointInfo = &pointInfos[k++];
		pointInfo->values = values;
		pointInfo->max = *std::max_element(values.begin(), values.end());

		auto edge = mesh.insertSite(xy);
		TMesh::setPointHandle(edge, pointInfo);
	}
	
	zero.values.resize(n, 0);
	zero.max = 0;
}


bool RecoveryMengSimon::Impl::recover(const XYZ& xyz, const Impl::PointInfo* pi[3], TValue w[3]) const
{
	const TValue s = xyz.x + xyz.y + xyz.z;
	if (s == 0)
	{
		return false;
	}
	const XY xy(xyz.x / s, xyz.y / s);
	if (!aabb.contains(xy))
	{
		return false;
	}

	auto e = mesh.locate(xy);
	if (mesh.isBoundingPoint(TMesh::fastOrg(e)) && !TMesh::hasLeftFace(e))
	{
		e = e->sym();
	}

	const XY a = TMesh::fastOrg(e);
	const XY b = TMesh::fastDest(e);
	const XY c = TMesh::fastDest(e->lNext());
	spat::barycenters(xy, a, b, c, w[0], w[1], w[2]);

	pi[0] = TMesh::pointHandle(e);
	pi[1] = TMesh::pointHandle(e->lNext());
	pi[2] = TMesh::pointHandle(e->lNext()->lNext());

	for (size_t k = 0; k < 3; ++k)
	{
		if (!pi[k])
		{
			pi[k] = &zero;
			w[k] = 0;
		}
	}

	const TValue wtot = w[0] + w[1] + w[2];
	if (wtot == 0)
	{
		return false;
	}

	w[0] *= s / wtot;
	w[1] *= s / wtot;
	w[2] *= s / wtot;

	return true;
}


}
}

// EOF
