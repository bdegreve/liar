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
#include "socket_host.h"
#include "socket_helpers.h"
#include <lass/util/thread_fun.h>

namespace liar
{
namespace output
{

PY_DECLARE_CLASS_DOC(SocketHost,
	"SocketHost()\n"
	"SocketHost(target)\n"
	"SocketHost(address [, port [, target]])\n"
	"\n"
	"Receives output samples from a socket and sends them to a RenderTarget");
PY_CLASS_CONSTRUCTOR_0(SocketHost);
PY_CLASS_CONSTRUCTOR_1(SocketHost, const std::string&);
PY_CLASS_CONSTRUCTOR_2(SocketHost, const std::string&, SocketHost::TPort);
PY_CLASS_CONSTRUCTOR_3(SocketHost, const std::string&, SocketHost::TPort, const TRenderTargetPtr&);
PY_CLASS_CONSTRUCTOR_1(SocketHost, const TRenderTargetPtr&);
PY_CLASS_MEMBER_RW(SocketHost, target, setTarget)
PY_CLASS_MEMBER_R(SocketHost, address)
PY_CLASS_MEMBER_R(SocketHost, port)

// --- public --------------------------------------------------------------------------------------

SocketHost::SocketHost():
	isQuiting_(false)
{
	init();
}



SocketHost::SocketHost(const TRenderTargetPtr& target):
	target_(target),
	isQuiting_(false)
{
	init();
}



SocketHost::SocketHost(const std::string& address, TPort port, const TRenderTargetPtr& target):
	target_(target),
	isQuiting_(false)
{
	init(address, port);
}



std::string SocketHost::address() const
{
	return socket_.address();
}



SocketHost::TPort SocketHost::port() const
{
	return socket_.port();
}



const TRenderTargetPtr& SocketHost::target() const
{
	return target_;
}



void SocketHost::setTarget(const TRenderTargetPtr& target)
{
	target_ = target;
}



SocketHost::~SocketHost()
{
	isQuiting_ = true;
	socket_.close();
	for (TThreads::iterator i = threads_.begin(), end = threads_.end(); i != end; ++i)
	{
		(*i)->join();
	}
}



void SocketHost::init(const std::string& address, TPort port)
{
	socket_.bind(address, port);
	socket_.listen();
	TThreadPtr thread(util::threadMemFun(this, &SocketHost::acceptThread, util::threadJoinable));
	thread->run();
	threads_.push_back(thread);
}



void SocketHost::acceptThread()
{
	while (!isQuiting_)
	{
		io::Socket connection;
		try
		{
			socket_.accept(connection);
		}
		catch (const io::SocketError&)
		{
			util::Thread::sleep(100);
			continue;
		}
		TThreadPtr thread(new ConnectionThread(*this, connection));
		thread->run();
		threads_.push_back(thread);
	}
}


SocketHost::ConnectionThread::ConnectionThread(SocketHost& receiver, io::Socket& connection):
	util::Thread(util::threadJoinable),
	receiver_(receiver),
	sampleBuffer_(128)
{
	connection_.swap(connection);
	istream_.setSocket(&connection_);
	ostream_.setSocket(&connection_);
	sampleIterator_ = sampleBuffer_.begin();
}


void SocketHost::ConnectionThread::doRun()
{
	TRenderTargetPtr target = receiver_.target();
	OutputSample sample;
	bool isCanceling = false;
	while (true)
	{
		SocketCommand command;
		if (!readCommand(istream_, command))
		{
			break;
		}
		switch (command)
		{
		case scSample:
			if (readSample(istream_, sample))
			{
				*sampleIterator_++ = sample;
				if (sampleIterator_ == sampleBuffer_.end())
				{
					flush();
				}
			}
			break;

		case scBeginRender:
			target->beginRender();
			break;

		case scEndRender:
			flush();
			//target->endRender();
			break;

		case scResolution:
			writeResolution(ostream_, target->resolution());
			break;

		case scIsCanceling:
			isCanceling |= target->isCanceling();
			ostream_ << isCanceling;
			ostream_.flush();
			break;
		}
	}
	flush();
}



void SocketHost::ConnectionThread::flush()
{
	if (sampleIterator_ == sampleBuffer_.begin())
	{
		return;
	}
	OutputSample* first = &sampleBuffer_[0];
	OutputSample* last = first + (sampleIterator_ - sampleBuffer_.begin());
	receiver_.target()->writeRender(first, last);
	sampleIterator_ = sampleBuffer_.begin();
}


}
}
