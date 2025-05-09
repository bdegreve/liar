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

#ifndef LIAR_GUARDIAN_OF_INCLUSION_OUTPUT_SOCKET_HELPERS_H
#define LIAR_GUARDIAN_OF_INCLUSION_OUTPUT_SOCKET_HELPERS_H

#include "output_common.h"
#include "../kernel/output_sample.h"
#include <lass/io/binary_i_stream.h>
#include <lass/io/binary_o_stream.h>

namespace liar
{
namespace output
{

enum SocketCommand
{
	scSample,
	scBeginRender,
	scEndRender,
	scResolution,
	scIsCanceling
};

void writeCommand(io::BinaryOStream& stream, SocketCommand command);
bool readCommand(io::BinaryIStream& stream, SocketCommand& command);

void writeResolution(io::BinaryOStream& stream, const TResolution2D& resolution);
bool readResolution(io::BinaryIStream& stream, TResolution2D& resolution);

void writeSample(io::BinaryOStream& stream, const OutputSample& sample);
bool readSample(io::BinaryIStream& stream, OutputSample& sample);

}
}

#endif

// EOF
