/* Implementation file betaservice.cpp
	
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

#include "system/debug.hpp"
#include "core/manager.hpp"
#include "foundation/objectpointer.hpp"

#include "core/listener.hpp"

#include "beta/betaservice.hpp"
#include "betaconnection.hpp"
#include "betatalker.hpp"

namespace fdt = foundation;

namespace concept{
namespace beta{

concept::Service* Service::create(){
	return new Service;
}

Service::Service(){
}

Service::~Service(){
}

int Service::insertConnection(
	concept::Manager &_rm,
	const SocketDevice &_rsd,
	foundation::aio::openssl::Context *_pctx,
	bool _secure
){
	Connection *pcon = new Connection(_rsd);
	if(this->insert(*pcon, this->index())){
		delete pcon;
		return BAD;
	}
	_rm.pushJob(static_cast<fdt::aio::Object*>(pcon));
	return OK;
}

int Service::insertListener(
	concept::Manager &_rm,
	const AddrInfoIterator &_rai,
	bool _secure
){
	SocketDevice sd;
	sd.create(_rai);
	sd.makeNonBlocking();
	sd.prepareAccept(_rai, 100);
	if(!sd.ok()) return BAD;
	concept::Listener *plis = new concept::Listener(sd);
	
	if(this->insert(*plis, this->index())){
		delete plis;
		return BAD;
	}	
	_rm.pushJob(static_cast<fdt::aio::Object*>(plis));
	return OK;
}
int Service::insertTalker(
	Manager &_rm, 
	const AddrInfoIterator &_rai,
	const char *_node,
	const char *_svc
){
	SocketDevice sd;
	sd.create(_rai);
	sd.bind(_rai);
	Talker *ptkr = new Talker(sd);
	if(this->insert(*ptkr, this->index())){
		delete ptkr;
		return BAD;
	}
	_rm.pushJob(static_cast<fdt::aio::Object*>(ptkr));
	return OK;
}

int Service::insertConnection(
	Manager &_rm, 
	const AddrInfoIterator &_rai,
	const char *_node,
	const char *_svc
){
	Connection *pcon = new Connection(_node, _svc);
	if(this->insert(*pcon, this->index())){
		delete pcon;
		return BAD;
	}
	_rm.pushJob(static_cast<fdt::aio::Object*>(pcon));
	return OK;
}

int Service::removeTalker(Talker& _rtkr){
	this->remove(_rtkr);
	return OK;
}

int Service::removeConnection(Connection &_rcon){
	//TODO:
	this->remove(_rcon);
	return OK;
}

}//namespace echo
}//namespace concept

