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
#include "socket_helpers.h"

namespace liar
{
namespace output
{

typedef num::Tuint8 TToken;

void writeCommand(io::BinaryOStream& stream, SocketCommand command)
{
	stream << static_cast<TToken>(command);
	stream.flush();
}

bool readCommand(io::BinaryIStream& stream, SocketCommand& command)
{
	TToken temp;
	stream >> temp;
	if (!stream.good())
	{
		return false;
	}
	command = static_cast<SocketCommand>(temp);
	return true;
}

void writeResolution(io::BinaryOStream& stream, const TResolution2D& resolution)
{
	const num::Tuint32 width = num::numCast<num::Tuint32>(resolution.x);
	const num::Tuint32 height = num::numCast<num::Tuint32>(resolution.y);
	stream << width << height;
	stream.flush();
}

bool readResolution(io::BinaryIStream& stream, TResolution2D& resolution)
{
	num::Tuint32 width = 0;
	num::Tuint32 height = 0;
	stream >> width >> height;
	if (!stream.good())
	{
		return false;
	}
	resolution = TResolution2D(width, height);
	return true;
}

void writeSample(io::BinaryOStream& stream, const OutputSample& sample)
{
	const TToken command = scSample;
	const TPoint2D& screenCoordinate = sample.screenCoordinate();
	const XYZ& radiance = sample.radiance();
	stream << command
		<< static_cast<num::Tfloat32>(screenCoordinate.x) << static_cast<num::Tfloat32>(screenCoordinate.y)
		<< static_cast<num::Tfloat32>(radiance.x) << static_cast<num::Tfloat32>(radiance.y) << static_cast<num::Tfloat32>(radiance.z)
		<< static_cast<num::Tfloat32>(sample.depth()) << static_cast<num::Tfloat32>(sample.alpha()) << static_cast<num::Tfloat32>(sample.weight());
}

bool readSample(io::BinaryIStream& stream, OutputSample& sample)
{
	num::Tfloat32 s, t;
	num::Tfloat32 x, y, z;
	num::Tfloat32 depth, alpha, weight;
	stream >> s >> t >> x >> y >> z >> depth >> alpha >> weight;
	if (!stream.good())
	{
		return false;
	}
	sample = OutputSample(TPoint2D(s, t), XYZ(x, y, z), depth, alpha, weight);
	return true;
}

}
}

// EOF
