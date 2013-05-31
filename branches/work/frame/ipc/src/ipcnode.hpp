/* Declarations file ipcnode.hpp
	
	Copyright 2013 Valentin Palade 
	vipalade@gmail.com

	This file is part of SolidFrame framework.

	SolidFrame is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	SolidFrame is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with SolidFrame.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SOLID_FRAME_IPC_SRC_IPC_NODE_HPP
#define SOLID_FRAME_IPC_SRC_IPC_NODE_HPP

#include "frame/aio/aiomultiobject.hpp"
#include "frame/ipc/ipcconnectionuid.hpp"

namespace solid{

class SocketAddress;

namespace frame{

namespace aio{
namespace openssl{
class Context;
}
}

namespace ipc{

struct ConnectData;
struct Packet;

class Service;

class Node: public Dynamic<Node, frame::aio::MultiObject>{
public:
	Node(
		const SocketDevice &_rsd,
		Service &_rservice,
		uint16 _id
	);
	~Node();
	int execute(ulong _sig, TimeSpec &_tout);
	
	uint32 pushSession(
		const SocketAddress &_rsa,
		const ConnectData &_rconndata,
		uint32 _idx = 0xffffffff
	);
	void pushConnection(
		SocketDevice &_rsd,
		uint32 _netoff,
		aio::openssl::Context *_pctx,
		bool _secure
	);
private:
	void doInsertNewSessions();
	void doPrepareInsertNewSessions();
	void doInsertNewConnections();
	int doReceiveDatagramPackets(uint _atmost, const ulong _sig);
	void doDispatchReceivedDatagramPacket(
		char *_pbuf,
		const uint32 _bufsz,
		const SocketAddress &_rsap
	);
	void doScheduleSendConnect(uint16 _idx, ConnectData &_rcd);
	uint16 doCreateSocket(const uint32 _netidx);
	void doTrySendSocketBuffers(const uint _sockidx);
	void doSendDatagramPackets();
	void doReceiveStreamData(const uint _sockidx);
	void doPrepareSocketReconnect(const uint _sockidx);
	void doHandleSocketEvents(const uint _sockidx, ulong _evs);
	uint16 doReceiveStreamPacket(const uint _sockidx);
	bool doOptimizeReadBuffer(const uint _sockidx);
	void doDoneSendDatagram();
	bool doReceiveConnectStreamPacket(const uint _sockidx, Packet &_rp);
private:
	struct Data;
	Data &d;
};



}//namespace ipc
}//namespace frame
}//namespace solid

#endif
