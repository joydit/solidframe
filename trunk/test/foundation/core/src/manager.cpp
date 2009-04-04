/* Implementation file manager.cpp
	
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

#include <map>
#include <vector>

#include "system/debug.hpp"

#include "utility/iostream.hpp"

#include "algorithm/serialization/binary.hpp"
#include "algorithm/serialization/idtypemap.hpp"

#include "core/manager.hpp"
#include "core/service.hpp"
#include "core/visitor.hpp"
#include "core/command.hpp"
#include "core/connection.hpp"
#include "core/object.hpp"


#include "foundation/selectpool.hpp"
#include "foundation/execpool.hpp"
#include "foundation/filemanager.hpp"
#include "foundation/filekeys.hpp"

#include "foundation/aio/aioselector.hpp"
#include "foundation/aio/aioobject.hpp"

#include "foundation/objectselector.hpp"
#include "foundation/commandexecuter.hpp"
#include "foundation/requestuid.hpp"

#include "foundation/ipc/ipcservice.hpp"


#include <iostream>
using namespace std;

//some forward declarations
namespace foundation{

class ObjectSelector;

namespace aio{
class Selector;
}

namespace tcp{
class ConnectionSelector;
class ListenerSelector;
}//namespace tcp

namespace udp{
class TalkerSelector;
}//namespace udp

}//namespace foundation

namespace fdt=foundation;


namespace test{

//======= FileManager:==================================================
//typedef fdt::FileUidTp			FileUidTp;
typedef Object::RequestUidTp	RequestUidTp;
//----------------------------------------------------------------------
/*
	A command for sending istreams from the fileManager
*/
struct IStreamCommand: test::Command{
	IStreamCommand(StreamPtr<IStream> &_sptr, const FileUidTp &_rfuid, const RequestUidTp &_requid):sptr(_sptr), fileuid(_rfuid), requid(_requid){
		idbg("");
	}
	int execute(Connection &_pcon);
	int execute(Object &_pobj);
	int execute(uint32 _evs, fdt::CommandExecuter&, const CommandUidTp &, TimeSpec &);
	StreamPtr<IStream>	sptr;
	FileUidTp			fileuid;
	RequestUidTp		requid;
};

int IStreamCommand::execute(Connection &_rcon){
	_rcon.receiveIStream(sptr, fileuid, requid);
	return NOK;
}

int IStreamCommand::execute(Object &_robj){
	return _robj.receiveIStream(sptr, fileuid, requid);
}

int IStreamCommand::execute(uint32 _evs, fdt::CommandExecuter& _rce, const CommandUidTp &, TimeSpec &){
	_rce.receiveIStream(sptr, fileuid, requid);
	return BAD;
}
//----------------------------------------------------------------------
/*
	A command for sending ostreams from the fileManager
*/
struct OStreamCommand: test::Command{
	OStreamCommand(StreamPtr<OStream> &_sptr, const FileUidTp &_rfuid, const RequestUidTp &_requid):sptr(_sptr), fileuid(_rfuid), requid(_requid){
		idbg("");
	}
	int execute(Connection &_pcon);
	int execute(Object &_pobj);
	int execute(uint32 _evs, fdt::CommandExecuter&, const CommandUidTp &, TimeSpec &);
	StreamPtr<OStream>	sptr;
	FileUidTp			fileuid;
	RequestUidTp		requid;
};

int OStreamCommand::execute(Connection &_rcon){
	_rcon.receiveOStream(sptr, fileuid, requid);
	return NOK;
}

int OStreamCommand::execute(Object &_robj){
	return _robj.receiveOStream(sptr, fileuid, requid);
}
int OStreamCommand::execute(uint32 _evs, fdt::CommandExecuter& _rce, const CommandUidTp &, TimeSpec &){
	_rce.receiveOStream(sptr, fileuid, requid);
	return NOK;
}
//----------------------------------------------------------------------
/*
	A command for sending iostreams from the fileManager
*/

struct IOStreamCommand: test::Command{
	IOStreamCommand(StreamPtr<IOStream> &_sptr, const FileUidTp &_rfuid, const RequestUidTp &_requid):sptr(_sptr), fileuid(_rfuid), requid(_requid){
		idbg("");
	}
	int execute(Connection &_pcon);
	int execute(Object &_pobj);
	int execute(uint32 _evs, fdt::CommandExecuter&, const CommandUidTp &, TimeSpec &);
	StreamPtr<IOStream>	sptr;
	FileUidTp			fileuid;
	RequestUidTp		requid;
};

int IOStreamCommand::execute(Connection &_rcon){
	_rcon.receiveIOStream(sptr, fileuid, requid);
	return NOK;
}

int IOStreamCommand::execute(Object &_robj){
	return _robj.receiveIOStream(sptr, fileuid, requid);
}

int IOStreamCommand::execute(uint32 _evs, fdt::CommandExecuter& _rce, const CommandUidTp &, TimeSpec &){
	_rce.receiveIOStream(sptr, fileuid, requid);
	return NOK;
}
//----------------------------------------------------------------------
/*
	A command for sending errors from the fileManager
*/

struct StreamErrorCommand: test::Command{
	StreamErrorCommand(int _errid, const RequestUidTp &_requid):errid(_errid), requid(_requid){}
	int execute(Connection &_pcon);
	int execute(Object &_pobj);
	int execute(uint32 _evs, fdt::CommandExecuter&, const CommandUidTp &, TimeSpec &);
	int				errid;
	RequestUidTp	requid;
};

int StreamErrorCommand::execute(Connection &_rcon){
	_rcon.receiveError(errid, requid);
	return NOK;
}
int StreamErrorCommand::execute(Object &_robj){
	return _robj.receiveError(errid, requid);
}
int StreamErrorCommand::execute(uint32 _evs, fdt::CommandExecuter& _rce, const CommandUidTp &, TimeSpec &){
	_rce.receiveError(errid, requid);
	return NOK;
}
//----------------------------------------------------------------------
/*
	Local implementation of the foundation::FileManager wich will now know how
	to send streams/errors to an object.
*/
class FileManager: public fdt::FileManager{
public:
	FileManager(uint32 _maxfcnt = 1024 * 20):fdt::FileManager(_maxfcnt){}
protected:
	/*virtual*/ void sendStream(StreamPtr<IStream> &_sptr, const FileUidTp &_rfuid, const fdt::RequestUid& _rrequid);
	/*virtual*/ void sendStream(StreamPtr<OStream> &_sptr, const FileUidTp &_rfuid, const fdt::RequestUid& _rrequid);
	/*virtual*/ void sendStream(StreamPtr<IOStream> &_sptr, const FileUidTp &_rfuid, const fdt::RequestUid& _rrequid);
	/*virtual*/ void sendError(int _errid, const fdt::RequestUid& _rrequid);
};
void FileManager::sendStream(StreamPtr<IStream> &_sptr, const FileUidTp &_rfuid, const fdt::RequestUid& _rrequid){
	fdt::CmdPtr<fdt::Command>	cp(new IStreamCommand(_sptr, _rfuid, RequestUidTp(_rrequid.reqidx, _rrequid.requid)));
	Manager::the().signalObject(_rrequid.objidx, _rrequid.objuid, cp);
}
void FileManager::sendStream(StreamPtr<OStream> &_sptr, const FileUidTp &_rfuid, const fdt::RequestUid& _rrequid){
	fdt::CmdPtr<fdt::Command>	cp(new OStreamCommand(_sptr, _rfuid, RequestUidTp(_rrequid.reqidx, _rrequid.requid)));
	Manager::the().signalObject(_rrequid.objidx, _rrequid.objuid, cp);
}
void FileManager::sendStream(StreamPtr<IOStream> &_sptr, const FileUidTp &_rfuid, const fdt::RequestUid& _rrequid){
	fdt::CmdPtr<fdt::Command>	cp(new IOStreamCommand(_sptr, _rfuid, RequestUidTp(_rrequid.reqidx, _rrequid.requid)));
	Manager::the().signalObject(_rrequid.objidx, _rrequid.objuid, cp);
}
void FileManager::sendError(int _error, const fdt::RequestUid& _rrequid){
	fdt::CmdPtr<fdt::Command>	cp(new StreamErrorCommand(_error, RequestUidTp(_rrequid.reqidx, _rrequid.requid)));
	Manager::the().signalObject(_rrequid.objidx, _rrequid.objuid, cp);
}

//======= IpcService ======================================================
/*
	Local implementation of the ipc service which will know in which 
	pool to push the ipc talkers.
*/
class IpcService: public fdt::ipc::Service{
public:
	IpcService(uint32 _keepalivetout):fdt::ipc::Service(_keepalivetout){}
protected:
	/*virtual*/void pushTalkerInPool(foundation::Manager &_rm, foundation::aio::Object *_ptkr);
};

//=========================================================================

struct ExtraObjPtr: fdt::ObjPtr<fdt::Object>{
	ExtraObjPtr(fdt::Object *_po);
	~ExtraObjPtr();
	ExtraObjPtr& operator=(fdt::Object *_pobj);
};

ExtraObjPtr::ExtraObjPtr(fdt::Object *_po):fdt::ObjPtr<fdt::Object>(_po){}

ExtraObjPtr::~ExtraObjPtr(){
	this->destroy(this->release());
}
ExtraObjPtr& ExtraObjPtr::operator=(fdt::Object *_pobj){
	ptr(_pobj);
	return *this;
}
//=========================================================================
/*
	A local command executer who knows how to unregister itself from the 
	manager
*/
class CommandExecuter: public fdt::CommandExecuter{
public:
	void removeFromManager();
};
void CommandExecuter::removeFromManager(){
	Manager::the().removeObject(this);
}

//=========================================================================
//The manager's localdata
struct Manager::Data{
	typedef std::vector<ExtraObjPtr>									ExtraObjectVector;
	typedef std::map<const char*, int, StrLess> 						ServiceIdxMap;
	typedef foundation::SelectPool<fdt::ObjectSelector>					ObjSelPoolTp;
	typedef foundation::SelectPool<fdt::aio::Selector>					AioSelectorPoolTp;

	Data(Manager &_rm);
	~Data();
	ExtraObjectVector					eovec;
	ServiceIdxMap						servicemap;// map name -> service index
	//fdt::ipc::Service					*pcs; // A pointer to the ipc service
	ObjSelPoolTp						*pobjectpool[2];//object pools
	AioSelectorPoolTp					*paiopool[2];
	fdt::ObjPtr<fdt::CommandExecuter>		readcmdexec;// read command executer
	fdt::ObjPtr<fdt::CommandExecuter>		writecmdexec;// write command executer
};

//--------------------------------------------------------------------------
typedef serialization::TypeMapper					TypeMapper;
typedef serialization::IdTypeMap					IdTypeMap;
typedef serialization::bin::Serializer				BinSerializer;
Manager::Data::Data(Manager &_rm)
{
	pobjectpool[0] = NULL;
	pobjectpool[1] = NULL;
	
	paiopool[0] = NULL;
	paiopool[1] = NULL;
	
	TypeMapper::registerMap<IdTypeMap>(new IdTypeMap);
	TypeMapper::registerSerializer<BinSerializer>();
	idbg("");
	if(true){
		pobjectpool[0] = new ObjSelPoolTp(	_rm, 
												10,		//max thread cnt
												1024 * 4//max objects per selector
												);		//at most 10 * 4 * 1024 connections
		pobjectpool[0]->start(1);//start with one worker
	}
	idbg("");
	if(true){
		paiopool[0] = new AioSelectorPoolTp(_rm,
										10,			//max thread cnt
										2048		//max aio objects per selector/thread
										);			//at most 10 * 4 * 1024 connections
		paiopool[0]->start(1);//start with one worker
		
		paiopool[1] = new AioSelectorPoolTp(_rm,
										10,			//max thread cnt
										2048		//max aio objects per selector/thread
										);			//at most 10 * 4 * 1024 connections
		paiopool[1]->start(1);//start with one worker
	}
	idbg("");
}

Manager::Data::~Data(){
	if(pobjectpool[0]) pobjectpool[0]->stop();
	if(pobjectpool[1]) pobjectpool[1]->stop();
	delete pobjectpool[0];
	delete pobjectpool[1];
	if(paiopool[0])paiopool[0]->stop();
	if(paiopool[1])paiopool[1]->stop();
	delete paiopool[0];
	delete paiopool[1];
	delete readcmdexec.release();
	delete writecmdexec.release();
}

//----------------------------------------------------------------------------------
ServiceMap& getServiceMap(){
	static ServiceMap sm;
	return sm;
}

void registerService(ServiceCreator _psc, const char* _pname){
	idbg("registering "<<_pname);
	cout<<"registering "<<_pname<<endl;
	getServiceMap()[_pname] = _psc;
}
//----------------------------------------------------------------------------------
template <>
void Manager::pushJob(fdt::Object *_pj, int _pos){
	d.pobjectpool[_pos]->push(fdt::ObjPtr<fdt::Object>(_pj));
}
template <>
void Manager::pushJob(fdt::aio::Object *_pj, int _i){
	d.paiopool[_i]->push(fdt::ObjPtr<fdt::aio::Object>(_pj));
}

/*
NOTE:
	It should be safe to give reference to 'this' to Data constructor, because
	it will be used in the SelectPool(s) constructor to call registerActiveSet(*this),
	which is defined in the base class of the manager which should be initialized first
*/

Manager::Manager():d(*(new Data(*this))){
	//ppools = new PoolContainer(*this);
	if(true){//create register the file manager
		this->fileManager(new FileManager(10));
		fdt::Manager::insertObject(&fileManager());
		fdt::NameFileKey::registerMapper(fileManager());
		fdt::TempFileKey::registerMapper(fileManager());
		this->pushJob((fdt::Object*)&fileManager());
	}
	if(true){// create register the read command executer
		d.readcmdexec = new CommandExecuter;
		fdt::Manager::insertObject(d.readcmdexec.ptr());
		this->pushJob((fdt::Object*)d.readcmdexec.ptr());
	}
	if(true){// create register the write command executer
		d.writecmdexec = new CommandExecuter;
		fdt::Manager::insertObject(d.writecmdexec.ptr());
		this->pushJob((fdt::Object*)d.writecmdexec.ptr());
	}
	if(true){// create register the ipc service
		this->ipc(new IpcService(1000));//one sec keepalive tout
		int pos = fdt::Manager::insertService(&this->ipc());
		if(pos < 0){
			idbg("unable to register service: "<<"ipc");
		}else{
			idbg("service "<<"ipc"<<" registered on pos "<<pos);
			this->pushJob((fdt::Object*)&this->ipc());
			//do not map the ipc!!!
			//d.servicemap["ipc"] = pos;
		}
	}
}

Manager::~Manager(){
	fdt::Manager::stop(true);//wait all services to stop
	delete &d;
}

int Manager::start(const char *_which){
	if(_which){
		Data::ServiceIdxMap::iterator it(d.servicemap.find(_which));
		if(it != d.servicemap.end()){
			service(it->second).start(*this);
		}
	}else{
		for(Data::ServiceIdxMap::iterator it(d.servicemap.begin());it != d.servicemap.end(); ++it){
			service(it->second).start(*this);
		}
	}
	return OK;
}
int Manager::stop(const char *_which){
	if(_which){
		Data::ServiceIdxMap::iterator it(d.servicemap.find(_which));
		if(it != d.servicemap.end()){
			service(it->second).stop(*this);
		}
	}else{
		fdt::Manager::stop(false);
	}
	return OK;
}

void Manager::readCommandExecuterUid(ObjectUidTp &_ruid){
	_ruid.first = d.readcmdexec->id();
	_ruid.second = uid(*d.readcmdexec);
}
void Manager::writeCommandExecuterUid(ObjectUidTp &_ruid){
	_ruid.first = d.writecmdexec->id();
	_ruid.second = uid(*d.writecmdexec);
}

int Manager::insertService(const char* _nm, Service* _psrvc){
	if(_psrvc != NULL){
		int pos = fdt::Manager::insertService(_psrvc);
		if(pos < 0){
			idbg("unable to register service: "<<_nm);
		}else{
			idbg("service "<<_nm<<" registered on pos "<<pos);
			//this->get<ObjSelPoolVecTp>()[0]->push(fdt::ObjPtr<fdt::Object>(_psrvc));
			this->pushJob((fdt::Object*)_psrvc);
			d.servicemap[_nm] = pos;
			return OK;
		}
	}else{
		idbg("service NULL");
	}
	return BAD;
}
void Manager::removeService(Service *_psrvc){
	fdt::Manager::removeService(_psrvc);
}

int Manager::insertListener(const char* _nm, const AddrInfoIterator &_rai, bool _secure){
	Data::ServiceIdxMap::iterator it(d.servicemap.find(_nm));
	if(it != d.servicemap.end()){
		test::Service &ts = static_cast<test::Service&>(this->service(it->second));
		return ts.insertListener(*this, _rai, _secure);
	}else{
		idbg("service not found "<<d.servicemap.size());
		return BAD;
	}
}

int Manager::insertTalker(const char* _nm, const AddrInfoIterator &_rai, const char*_node, const char *_srv){
	Data::ServiceIdxMap::iterator it(d.servicemap.find(_nm));
	if(it != d.servicemap.end()){
		test::Service &ts = static_cast<test::Service&>(this->service(it->second));
		return ts.insertTalker(*this, _rai, _node, _srv);
	}else{
		idbg("service not found "<<d.servicemap.size());
		return BAD;
	}
}

int Manager::insertConnection(
	const char* _nm,
	const AddrInfoIterator &_rai,
	const char*_node,
	const char *_srv,
	bool _secure
){
	Data::ServiceIdxMap::iterator it(d.servicemap.find(_nm));
	if(it != d.servicemap.end()){
		test::Service &ts = static_cast<test::Service&>(this->service(it->second));
		return ts.insertConnection(*this, _rai, _node, _srv);
	}else{
		idbg("service not found "<<d.servicemap.size());
		return BAD;
	}
}

int Manager::visitService(const char* _nm, Visitor &_rov){
	Data::ServiceIdxMap::iterator it(d.servicemap.find(_nm));
	if(it != d.servicemap.end()){
		//test::Service *ts = static_cast<test::Service*>(this->service(it->second));
		//TODO:
		//ts->visit(_rov);
		return OK;
	}else{
		idbg("service not found "<<d.servicemap.size());
		return BAD;
	}
}
//----------------------------------------------------------------------------------
void IpcService::pushTalkerInPool(foundation::Manager &_rm, foundation::aio::Object *_ptkr){
	static_cast<Manager&>(_rm).pushJob(_ptkr, 1);
}
//----------------------------------------------------------------------------------
// Some dummy command methods definitions
int Command::execute(Connection &){
	cassert(false);
	return BAD;
}

int Command::execute(Listener &){
	cassert(false);
	return BAD;
}

int Command::execute(Object &){
	cassert(false);
	return BAD;
}

//----------------------------------------------------------------------------------
// some dummy connection methods definitions
int Connection::receiveIStream(
	StreamPtr<IStream> &,
	const FileUidTp	&,
	const RequestUidTp &_requid,
	int			_which,
	const ObjectUidTp& _from,
	const foundation::ipc::ConnectorUid *_conid
){
	cassert(false);
	return BAD;
}

int Connection::receiveOStream(
	StreamPtr<OStream> &,
	const FileUidTp	&,
	const RequestUidTp &_requid,
	int			_which,
	const ObjectUidTp& _from,
	const foundation::ipc::ConnectorUid *_conid
){
	cassert(false);
	return BAD;
}

int Connection::receiveIOStream(
	StreamPtr<IOStream> &,
	const FileUidTp	&,
	const RequestUidTp &_requid,
	int			_which,
	const ObjectUidTp&_from,
	const foundation::ipc::ConnectorUid *_conid
){
	cassert(false);
	return BAD;
}

int Connection::receiveString(
	const String &_str,
	const RequestUidTp &_requid,
	int			_which,
	const ObjectUidTp&_from,
	const foundation::ipc::ConnectorUid *_conid
){
	cassert(false);
	return BAD;
}
int Connection::receiveData(
	void *_pdata,
	int	_datasz,
	const RequestUidTp &_requid,
	int			_which,
	const ObjectUidTp&_from,
	const foundation::ipc::ConnectorUid *_conid
){
	cassert(false);
	return BAD;
}
int Connection::receiveNumber(
	const int64 &_no,
	const RequestUidTp &_requid,
	int			_which,
	const ObjectUidTp&_from,
	const foundation::ipc::ConnectorUid *_conid
){
	cassert(false);
	return BAD;
}

int Connection::receiveError(
	int _errid, 
	const RequestUidTp &_requid,
	const ObjectUidTp&_from,
	const foundation::ipc::ConnectorUid *_conid
){
	cassert(false);
	return BAD;
}
//----------------------------------------------------------------------------------
//Some dummy object methods definitions
int Object::receiveIStream(
	StreamPtr<IStream> &,
	const FileUidTp	&,
	const RequestUidTp &_requid,
	int			_which,
	const ObjectUidTp&_from,
	const foundation::ipc::ConnectorUid *_conid
){
	cassert(false);
	return BAD;
}

int Object::receiveOStream(
	StreamPtr<OStream> &,
	const FileUidTp	&,
	const RequestUidTp &_requid,
	int			_which,
	const ObjectUidTp&_from,
	const foundation::ipc::ConnectorUid *_conid
){
	cassert(false);
	return BAD;
}

int Object::receiveIOStream(
	StreamPtr<IOStream> &,
	const FileUidTp	&,
	const RequestUidTp &_requid,
	int			_which,
	const ObjectUidTp&_from,
	const foundation::ipc::ConnectorUid *_conid
){
	cassert(false);
	return BAD;
}

int Object::receiveString(
	const String &_str,
	const RequestUidTp &_requid,
	int			_which,
	const ObjectUidTp&_from,
	const foundation::ipc::ConnectorUid *_conid
){
	cassert(false);
	return BAD;
}
int Object::receiveError(
	int _errid, 
	const RequestUidTp &_requid,
	const ObjectUidTp&_from,
	const foundation::ipc::ConnectorUid *_conid
){
	cassert(false);
	return BAD;
}

//----------------------------------------------------------------------------------
}//namespace test
