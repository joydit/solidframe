/* Implementation file alphaservice.cpp
	
	Copyright 2007, 2008 Valentin Palade 
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

#include "system/debug.hpp"
#include "core/manager.hpp"


#include "algorithm/serialization/binary.hpp"

#include "foundation/objectpointer.hpp"
#include "foundation/aio/openssl/opensslsocket.hpp"

#include "core/listener.hpp"

#include "alpha/alphaservice.hpp"
#include "alphaconnection.hpp"

namespace fdt=foundation;

namespace concept{
namespace alpha{

struct InitServiceOnce{
	InitServiceOnce(Manager &_rm);
};

InitServiceOnce::InitServiceOnce(Manager &_rm){
	Connection::initStatic(_rm);
}

Service* Service::create(Manager &_rm){
	static InitServiceOnce	init(_rm);
	return new Service();
}

Service::Service(){
}

Service::~Service(){
}

bool Service::insertConnection(
	const SocketDevice &_rsd,
	foundation::aio::openssl::Context *_pctx,
	bool _secure
){
	//create a new connection with the given channel
	fdt::ObjectPointer<Connection> conptr(new Connection(_rsd));
	if(_pctx){
		conptr->socketSecureSocket(_pctx->createSocket());
	}
	//register it into the service
	this->insert<AioSchedulerT>(conptr, 0);
	return true;
}

void Service::insertObject(Connection &_ro, const ObjectUidT &_ruid){
	idbg("alpha "<<fdt::compute_service_id(_ruid.first)<<' '<<fdt::compute_index(_ruid.first)<<' '<<_ruid.second);
}

void Service::eraseObject(const Connection &_ro){
	ObjectUidT objuid(_ro.uid());
	idbg("alpha "<<fdt::compute_service_id(objuid.first)<<' '<<fdt::compute_index(objuid.first)<<' '<<objuid.second);
}
}//namespace alpha
}//namespace concept

