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

#ifndef LIAR_GUARDIAN_OF_INCLUSION_OUTPUT_SOCKET_HOST_H
#define LIAR_GUARDIAN_OF_INCLUSION_OUTPUT_SOCKET_HOST_H

#include "output_common.h"
#include "../kernel/render_target.h"
#include <lass/io/socket.h>
#include <lass/io/binary_o_socket.h>
#include <lass/io/binary_i_socket.h>

namespace liar
{
namespace output
{


class LIAR_OUTPUT_DLL SocketHost: public python::PyObjectPlus, util::NonCopyable
{
	PY_HEADER(python::PyObjectPlus)
public:

	typedef io::Socket::TPort TPort;

	SocketHost();
	explicit SocketHost(const std::string& address, TPort port=0, const TRenderTargetPtr& target=TRenderTargetPtr());
	explicit SocketHost(const TRenderTargetPtr& target);
	~SocketHost();

	std::string address() const;
	TPort port() const;
	const TRenderTargetPtr& target() const;
	void setTarget(const TRenderTargetPtr& target);

private:

	typedef util::SharedPtr<util::Thread> TThreadPtr;
	typedef std::vector<TThreadPtr> TThreads;

	class ConnectionThread: public util::Thread
	{
	public:
		ConnectionThread(SocketHost& receiver, io::Socket& connection);
	private:
		typedef std::vector<OutputSample> TSamples;
		void doRun();
		void flush();
		const SocketHost& receiver_;
		io::Socket connection_;
		io::BinaryISocket istream_;
		io::BinaryOSocket ostream_;
		TSamples sampleBuffer_;
		TSamples::iterator sampleIterator_;
	};

	void init(const std::string& address = "", TPort port = 0);
	void acceptThread();

	io::Socket socket_;
	TRenderTargetPtr target_;
	TThreads threads_;
	std::atomic<bool> isQuiting_;
};



}

}

#endif

// EOF
