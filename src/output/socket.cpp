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
#include "socket.h"
#include <lass/util/thread_fun.h>

namespace liar
{
namespace output
{

PY_DECLARE_CLASS_DOC(Socket, "sends output samples over socket");
PY_CLASS_CONSTRUCTOR_2(Socket, const std::string&, Socket::TPort)

// --- public --------------------------------------------------------------------------------------

Socket::Socket(const std::string& address, TPort port):
	socket_(),
	ostream_(socket_),
	istream_(socket_),
	specialToken_(-1),
	isCanceling_(false),
	isQuiting_(false)
{
	socket_.connect(address, port);
}



Socket::~Socket()
{
	if (!isQuiting_)
	{
		endRender();
	}
}



// --- private -------------------------------------------------------------------------------------

const TResolution2D Socket::doResolution() const
{
	LASS_LOCK(socketLock_)
	{
		ostream_ << specialToken_ << "?resolution";
		num::Tuint32 width, height;
		istream_ >> width >> height;
		return TResolution2D(width, height);
	}
}



void Socket::doBeginRender()
{
	LASS_LOCK(socketLock_)
	{
		ostream_ << specialToken_ << "begin";
	}
	isCancelingLoop_.reset(util::threadFun(util::makeCallback(this, &Socket::isCancelingLoop), util::threadJoinable));
	isCancelingLoop_->run();
}



void Socket::doWriteRender(const OutputSample* first, const OutputSample* last)
{
	LASS_LOCK(socketLock_)
	{
		while (first != last)
		{
			ostream_ << *first++;
		}
	}
}



void Socket::doEndRender()
{
	LASS_LOCK(socketLock_)
	{
		ostream_ << specialToken_ << "end";
	}
	isQuiting_ = true;
	isCancelingLoop_->join();
}



bool Socket::doIsCanceling() const
{
	return isCanceling_;
}



void Socket::isCancelingLoop()
{
	while (!isQuiting_ && !isCanceling_)
	{
		bool isCanceling;
		LASS_LOCK(socketLock_)
		{
			ostream_ << specialToken_ << "?cancel";
			istream_ >> isCanceling;
		}
		isCanceling_ = isCanceling;
		util::Thread::sleep(1000);
	}
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
