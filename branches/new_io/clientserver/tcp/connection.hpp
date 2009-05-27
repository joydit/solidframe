/* Declarations file connection.hpp
	
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

#ifndef CS_TCP_CONNECTION_HPP
#define CS_TCP_CONNECTION_HPP


#include "clientserver/core/object.hpp"

namespace clientserver{
namespace tcp{

class Channel;
//! The base class for tcp connections
/*!
	<b>Usage:</b><br>
	- Inherit and implement execute for protocol logic.
	- Asynchrounously read/write data using given clientserver::tcp::Channel.
	
	<b>Notes:</b><br>
	Why associating Connection and Channel and not inheriting from Channel or 
	keep a Channel object, and not a reference:<br>
	- At connections level I do not want to know about some of Channel's interface
	like that it uses descriptors or something like.
	- A pool of Channels would allow inter protocols Channel objects reuse.
*/

class Connection: public Object{
public:
	virtual ~Connection();
	Channel& channel();
	virtual int execute(ulong _evs, TimeSpec &_rtout) = 0;
	virtual int accept(clientserver::Visitor &_roi);
protected:
	Connection(Channel *_pch);
private:
	Channel		&rch;
};

inline Channel& Connection::channel(){return rch;}

}//namespace tcp
}//namespace clientserver

#endif