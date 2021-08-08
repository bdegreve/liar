/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2021  Bram de Greve (bramz@users.sourceforge.net)
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

/** @class liar::output::SocketClient
 *  @brief sends samples over TCP to a SocketHost
 *  @author Bram de Greve [Bramz]
 */

#ifndef LIAR_GUARDIAN_OF_INCLUSION_OUTPUT_SOCKET_CLIENT_H
#define LIAR_GUARDIAN_OF_INCLUSION_OUTPUT_SOCKET_CLIENT_H

#include "output_common.h"
#include "../kernel/render_target.h"
#include <lass/io/socket.h>
#include <lass/io/binary_o_socket.h>
#include <lass/io/binary_i_socket.h>
#include <lass/util/thread.h>

namespace liar
{
namespace output
{

class LIAR_OUTPUT_DLL SocketClient: public RenderTarget
{
	PY_HEADER(RenderTarget)
public:

	typedef io::Socket::TPort TPort;

	SocketClient(const std::string& address, TPort port);
	~SocketClient();

private:

	const TResolution2D doResolution() const;
	void doBeginRender();
	void doWriteRender(const OutputSample* first, const OutputSample* last);
	void doEndRender();
	bool doIsCanceling() const;
	void isCancelingLoop();

	io::Socket socket_;
	mutable util::CriticalSection socketLock_;
	mutable io::BinaryOSocket ostream_;
	mutable io::BinaryISocket istream_;

	std::unique_ptr<util::Thread> isCancelingLoop_;
	volatile bool isQuiting_;
	volatile bool isCanceling_;
};

}

}

#endif

// EOF
