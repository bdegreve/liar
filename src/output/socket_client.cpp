/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2023  Bram de Greve (bramz@users.sourceforge.net)
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
#include "socket_client.h"
#include "socket_helpers.h"
#include <lass/util/thread_fun.h>

namespace liar
{
namespace output
{


PY_DECLARE_CLASS_DOC(SocketClient, "sends output samples over socket");
PY_CLASS_CONSTRUCTOR_2(SocketClient, const std::string&, SocketClient::TPort)

// --- public --------------------------------------------------------------------------------------

SocketClient::SocketClient(const std::string& address, TPort port):
	isQuiting_(false),
	isCanceling_(false)
{
	socket_.connect(address, port);
	istream_.setSocket(&socket_);
	ostream_.setSocket(&socket_);
}



SocketClient::~SocketClient()
{
	if (!isQuiting_)
	{
		endRender();
	}
}



// --- private -------------------------------------------------------------------------------------

const TResolution2D SocketClient::doResolution() const
{
	std::lock_guard<std::mutex> lock(mutex_);
	TResolution2D resolution;
	writeCommand(ostream_, scResolution);
	LASS_ENFORCE(readResolution(istream_, resolution));
	return resolution;
}



void SocketClient::doBeginRender()
{
	std::lock_guard<std::mutex> lock(mutex_);
	writeCommand(ostream_, scBeginRender);
	isCancelingLoop_.reset(util::threadFun(util::makeCallback(this, &SocketClient::isCancelingLoop), util::threadJoinable));
	isCancelingLoop_->run();
}



void SocketClient::doWriteRender(const OutputSample* first, const OutputSample* last)
{
	std::lock_guard<std::mutex> lock(mutex_);
	while (first != last)
	{
		writeSample(ostream_, *first++);
	}
	ostream_.flush();
}



void SocketClient::doEndRender()
{
	std::lock_guard<std::mutex> lock(mutex_);
	writeCommand(ostream_, scEndRender);
	isQuiting_ = true;
	isCancelingLoop_->join();
}



bool SocketClient::doIsCanceling() const
{
	return isCanceling_;
}



void SocketClient::isCancelingLoop()
{
	while (!isQuiting_ && !isCanceling_)
	{
		bool isCanceling = false;
		std::lock_guard<std::mutex> lock(mutex_);
		{
			writeCommand(ostream_, scIsCanceling);
			if (!ostream_.good())
			{
				isCanceling = true;
			}
			else
			{
				istream_ >> isCanceling;
				isCanceling |= !istream_.good();
			}
		}
		isCanceling_ = isCanceling;
		util::Thread::sleep(500);
	}
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
