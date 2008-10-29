/* Declarations file multiconnectionselector.hpp
	
	Copyright 2007, 2008 Valentin Palade 
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

#ifndef CS_TCP_MULTICONNECTIONSELECTOR_HPP
#define CS_TCP_MULTICONNECTIONSELECTOR_HPP

#include "clientserver/core/common.hpp"
#include "clientserver/core/objptr.hpp"


struct TimeSpec;
struct epoll_event;

namespace clientserver{
namespace tcp{

class MultiConnection;
class ChannelData;
class Channel;
class DataNode;

typedef ObjPtr<MultiConnection>	MultiConnectionPtrTp;


//! A selector for tcp::Connection, that uses epoll on linux.
/*!
	It is designed to work with clientserver::SelectPool
*/
class MultiConnectionSelector{
public:
	typedef MultiConnectionPtrTp		ObjectTp;
	
	MultiConnectionSelector();
	~MultiConnectionSelector();
	int reserve(ulong _cp);
	//signal a specific object
	void signal(uint _pos = 0);
	void run();
	uint capacity()const;
	uint size() const;
	int  empty()const;
	int  full()const;
	
	void push(const ObjectTp &_rcon, uint _thid);
	void prepare();
	void unprepare();
private:
	struct Stub;
	int doReadPipe();
	int doExecute(Stub &_rstub, ulong _evs, TimeSpec &_crttout, epoll_event &_rev);
	uint doIo(Channel &_r, ulong _evs);
	uint doAllIo();
	uint doFullScan();
	uint doExecuteQueue();
	
	void doUnregisterObject(MultiConnection &_robj);
	void doPrepareObjectWait(Stub &_rstub, const TimeSpec &_crttout);
private://data
	struct Data;
	Data	&d;
};


}//namespace tcp
}//namespace clientserver

#endif

