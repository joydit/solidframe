/* Declarations file ipcsession.hpp
	
	Copyright 2010 Valentin Palade
	vipalade@gmail.com

	This file is part of SolidGround framework.

	SolidGround is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	SolidGround is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with SolidGround.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef IPC_CONNECTION_HPP
#define IPC_CONNECTION_HPP

#include "iodata.hpp"
#include "ipctalker.hpp"
#include "system/timespec.hpp"

#include "utility/dynamicpointer.hpp"

struct SocketAddress;
struct SockAddrPair;
struct Inet4SockAddrPair;
struct Inet6SockAddrPair;
struct TimeSpec;
struct AddrInfoIterator;


namespace foundation{

struct Signal;

namespace ipc{

class Session{
public:
	typedef std::pair<const Inet4SockAddrPair*, int> Addr4PairT;
	typedef std::pair<const Inet6SockAddrPair*, int> Addr6PairT;
public:
	static void init();
	static int parseAcceptedBuffer(const Buffer &_rbuf);
	static int parseConnectingBuffer(const Buffer &_rbuf);
	
	Session(
		const Inet4SockAddrPair &_raddr,
		uint32 _keepalivetout
	);
	Session(
		const Inet4SockAddrPair &_raddr,
		int _basport,
		uint32 _keepalivetout
	);
	
	~Session();
	
	const Inet4SockAddrPair* peerAddr4()const;
	const Addr4PairT* baseAddr4()const;
	const SockAddrPair* peerSockAddr()const;
	
	bool isConnected()const;
	bool isDisconnecting()const;
	bool isConnecting()const;
	bool isAccepting()const;

	void prepare();
	void reconnect(Session *_pses);	
	int pushSignal(DynamicPointer<Signal> &_rsig, uint32 _flags);
	bool pushReceivedBuffer(Buffer &_rbuf, const TimeSpec &_rts);
	
	void completeConnect(int _port);
	
	bool executeTimeout(
		Talker::TalkerStub &_rstub,
		uint32 _id,
		const TimeSpec &_rts
	);
	int execute(Talker::TalkerStub &_rstub, const TimeSpec &_rts);
	bool pushSentBuffer(
		uint32 _id,
		const char *_data,
		const uint16 _size
	);
	
};

}//namespace ipc
}//namespace foundation


#endif
