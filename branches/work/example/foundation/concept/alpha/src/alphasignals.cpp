#include "utility/istream.hpp"
#include "utility/ostream.hpp"
#include "utility/iostream.hpp"
#include "utility/dynamicpointer.hpp"

#include "system/timespec.hpp"
#include "system/filedevice.hpp"
#include "system/mutex.hpp"

#include "foundation/signalexecuter.hpp"
#include "foundation/requestuid.hpp"
#include "foundation/file/filemanager.hpp"
#include "foundation/ipc/ipcservice.hpp"

#include "core/common.hpp"
#include "core/manager.hpp"
#include "core/signals.hpp"


#include "alphasignals.hpp"
#include "system/debug.hpp"

namespace fdt=foundation;

namespace concept{

namespace alpha{

const SignalTypeIds& SignalTypeIds::the(const SignalTypeIds *_pids){
	static const SignalTypeIds ids(*_pids);
	return ids;
}
	
//-----------------------------------------------------------------------------------
// RemoteListSignal
//-----------------------------------------------------------------------------------
RemoteListSignal::RemoteListSignal(
	uint32 _tout, uint16 _sentcnt
): ppthlst(NULL),err(-1),tout(_tout), success(0), ipcstatus(IpcOnSender){
	idbg(""<<(void*)this);
}
RemoteListSignal::RemoteListSignal(
	const NumberType<1>&
):ppthlst(NULL), err(-1),tout(0), success(0), ipcstatus(IpcOnSender){
	idbg(""<<(void*)this);
}
RemoteListSignal::~RemoteListSignal(){
	idbg(""<<(void*)this);
	if(ipcstatus == IpcOnSender && success == 1){
		idbg("failed receiving response");
		m().signal(fdt::S_KILL | fdt::S_RAISE, fromv.first, fromv.second);
	}
	delete ppthlst;
}
void RemoteListSignal::use(){
	DynamicShared<fdt::Signal>::use();
	idbg(""<<(void*)this<<" usecount = "<<usecount);
}
int RemoteListSignal::release(){
	int rv = DynamicShared<fdt::Signal>::release();
	idbg(""<<(void*)this<<" usecount = "<<usecount);
	return rv;
}
uint32 RemoteListSignal::ipcPrepare(){
	const foundation::ipc::SignalContext	&rsigctx(foundation::ipc::SignalContext::the());
	Locker<Mutex>							lock(mutex());
	
	if(success == 0) success = 1;//wait
	idbg(""<<(void*)this<<" siguid = "<<rsigctx.signaluid.idx<<' '<<rsigctx.signaluid.uid<<" ipcstatus = "<<(int)ipcstatus);
	if(ipcstatus == IpcOnSender){//on sender
		return foundation::ipc::Service::WaitResponseFlag /*| foundation::ipc::Service::SynchronousSendFlag*/;
	}else{
		return 0/*foundation::ipc::Service::SynchronousSendFlag*/;// on peer
	}
}
void RemoteListSignal::ipcReceived(
	fdt::ipc::SignalUid &_rsiguid
){
	DynamicPointer<fdt::Signal> psig(this);
	idbg(""<<(void*)this<<" siguid = "<<siguid.idx<<' '<<siguid.uid);
	conid = fdt::ipc::SignalContext::the().connectionuid;
	++ipcstatus;
	if(ipcstatus == IpcOnPeer){//on peer
		idbg("Received RemoteListSignal on peer");
		m().signal(psig, m().readSignalExecuterUid());
	}else{//on sender
		cassert(ipcstatus == IpcBackOnSender);
		idbg("Received RemoteListSignal back on sender");
		_rsiguid = siguid;
		m().signal(psig, fromv.first, fromv.second);
	}
}
void RemoteListSignal::ipcFail(int _err){
	Locker<Mutex> lock(mutex());
	err = _err;
	if(ipcstatus == IpcOnSender){
		idbg("failed on sender "<<(void*)this);
	}else{
		idbg("failed on peer");
	}
}
void RemoteListSignal::ipcSuccess(){
	Locker<Mutex> lock(mutex());
	success = 2;
	idbg("");
}

int RemoteListSignal::execute(
	DynamicPointer<Signal> &_rthis_ptr,
	uint32 _evs,
	fdt::SignalExecuter&,
	const SignalUidT &,
	TimeSpec &_rts
){
	if(tout){
		idbg("sleep for "<<tout<<" mseconds");
		_rts += tout;
		tout = 0;
		return NOK;
	}
	idbg("done sleeping");
	
	fs::directory_iterator			it,end;
	fs::path						pth(strpth.c_str()/*, fs::native*/);
	
	ppthlst = new RemoteList::PathListT;
	strpth.clear();
	
	if(!exists( pth ) || !is_directory(pth)){
		err = -1;
		if(Manager::the().ipc().sendSignal(_rthis_ptr, conid)){
			idbg("connector was destroyed");
		}
		return BAD;
	}
	
	try{
		it = fs::directory_iterator(pth);
	}catch ( const std::exception & ex ){
		idbg("dir_iterator exception :"<<ex.what());
		err = -1;
		strpth = ex.what();
		if(Manager::the().ipc().sendSignal(_rthis_ptr, SignalTypeIds::the().remotelistresponse, conid)){
			idbg("connector was destroyed");
		}
		return BAD;
	}
	
	while(it != end){
		ppthlst->push_back(std::pair<String, int64>(it->path().c_str(), -1));
		if(is_directory(*it)){
		}else{
			ppthlst->back().second = FileDevice::size(it->path().c_str());
		}
		++it;
	}
	err = 0;
	//Thread::sleep(1000 * 20);
	if(Manager::the().ipc().sendSignal(_rthis_ptr, SignalTypeIds::the().remotelistresponse, conid)){
		idbg("connector was destroyed "<<conid.tid<<' '<<conid.idx<<' '<<conid.uid);
	}else{
		idbg("signal sent "<<conid.tid<<' '<<conid.idx<<' '<<conid.uid);
	}
	return BAD;
}

//-----------------------------------------------------------------------------------
// FetchMasterSignal
//-----------------------------------------------------------------------------------

FetchMasterSignal::~FetchMasterSignal(){
	delete psig;
	idbg((void*)this<<"");
}

void FetchMasterSignal::ipcFail(int _err){
	idbg((void*)this<<"");
	Manager::the().signal(fdt::S_RAISE | fdt::S_KILL, fromv.first, fromv.second);
}
void FetchMasterSignal::print()const{
	idbg((void*)this<<" FetchMasterSignal:");
	idbg("state = "<<state<<" streamsz = "<<streamsz<<" requid = "<<requid<<" fname = "<<fname);
	idbg("fromv.first = "<<fromv.first<<" fromv.second = "<<fromv.second);
	idbg("fuid.first = "<<fuid.first<<" fuid.second = "<<fuid.second);
	idbg("tmpfuid.first = "<<tmpfuid.first<<" tmpfuid.second = "<<tmpfuid.second);
}

uint32 FetchMasterSignal::ipcPrepare(){
	return 0;//foundation::ipc::Service::SynchronousSendFlag;
}


void FetchMasterSignal::ipcReceived(
	fdt::ipc::SignalUid &_rsiguid
){
	DynamicPointer<fdt::Signal> sig(this);
	conid = fdt::ipc::SignalContext::the().connectionuid;;
	state = Received;
	idbg("received master signal");
	print();
	m().signal(sig, m().readSignalExecuterUid());
}
/*
	The state machine running on peer
*/
int FetchMasterSignal::execute(
	DynamicPointer<Signal> &_rthis_ptr,
	uint32 _evs,
	fdt::SignalExecuter& _rce,
	const SignalUidT &_siguid,
	TimeSpec &_rts
){
	Manager &rm(Manager::the());
	cassert(!(_evs & fdt::TIMEOUT));
	switch(state){
		case Received:{
			idbg((void*)this<<" try to open file "<<fname<<" _siguid = "<<_siguid.first<<","<<_siguid.second);
			//try to get a stream for the file:
			foundation::RequestUid reqid(_rce.id(), rm.uid(_rce), _siguid.first, _siguid.second);
			switch(rm.fileManager().stream(ins, fuid, reqid, fname.c_str())){
				case BAD://ouch
					state = SendError;
					idbg("open failed");
					return OK;
				case OK:
					idbg("open succeded");
					state = SendFirstStream;
					return OK;
				case NOK:
					idbg("open wait");
					return NOK;//wait the stream - no timeout
			}
		}break;
		case SendFirstStream:{
			idbg((void*)this<<" send first stream");
			FetchSlaveSignal			*psig(new FetchSlaveSignal);
			DynamicPointer<fdt::Signal>	sigptr(psig);
			
			this->filesz = ins->size();
			psig->tov = fromv;
			psig->filesz = this->filesz;
			psig->streamsz = this->streamsz;
			if(psig->streamsz > psig->filesz){
				psig->streamsz = psig->filesz;
			}
			psig->siguid = _siguid;
			psig->requid = requid;
			psig->fuid = tmpfuid;
			idbg("filesz = "<<this->filesz<<" inpos = "<<filepos);
			this->filesz -= psig->streamsz;
			this->filepos += psig->streamsz;
			fdt::RequestUid reqid(_rce.id(), rm.uid(_rce), _siguid.first, _siguid.second);
			rm.fileManager().stream(psig->ins, fuid, requid, fdt::file::Manager::NoWait);
			psig = NULL;
			if(rm.ipc().sendSignal(sigptr, conid) || !this->filesz){
				idbg("connector was destroyed or filesz = "<<this->filesz);
				return BAD;
			}
			idbg("wait for streams");
			//TODO: put here timeout! - wait for signals
			//_rts.add(30);
			return NOK;
		}
		case SendNextStream:{
			idbg((void*)this<<" send next stream");
			DynamicPointer<fdt::Signal> sigptr(psig);
			psig->tov = fromv;
			psig->filesz = this->filesz;
			//psig->sz = FetchChunkSize;
			if(psig->streamsz > this->filesz){
				psig->streamsz = this->filesz;
			}
			psig->siguid = _siguid;
			//psig->fuid = tmpfuid;
			idbg("filesz = "<<this->filesz<<" filepos = "<<this->filepos);
			this->filesz -= psig->streamsz;
			fdt::RequestUid reqid(_rce.id(), rm.uid(_rce), _siguid.first, _siguid.second);
			rm.fileManager().stream(psig->ins, fuid, requid, fdt::file::Manager::NoWait);
			psig->ins->seek(this->filepos);
			this->filepos += psig->streamsz;
			cassert(psig->ins);
			psig = NULL;
			if(rm.ipc().sendSignal(sigptr, conid) || !this->filesz){
				idbg("connector was destroyed or filesz = "<<this->filesz);
				return BAD;
			}
			idbg("wait for streams");
			//TODO: put here timeout! - wait for signals
			//_rts.add(30);
			return NOK;
		}
		case SendError:{
			idbg((void*)this<<" sending error");
			FetchSlaveSignal *psig = new FetchSlaveSignal;
			DynamicPointer<fdt::Signal> sigptr(psig);
			psig->tov = fromv;
			psig->filesz = -1;
			rm.ipc().sendSignal(sigptr, conid);
			return BAD;
		}
	}
	return BAD;
}

int FetchMasterSignal::receiveSignal(
	DynamicPointer<Signal> &_rsig,
	const ObjectUidT& _from,
	const fdt::ipc::ConnectionUid *_conid
){
	if(_rsig->dynamicTypeId() == InputStreamSignal::staticTypeId()){
		idbg((void*)this<<" Received stream");
		InputStreamSignal &rsig(*static_cast<InputStreamSignal*>(_rsig.ptr()));
		ins = rsig.sptr;
		fuid = rsig.fileuid;
		state = SendFirstStream;
	}else /*if(_rsig->dynamicTypeId() == OutputStreamSignal::staticTypeId()){
		OutputStreamSignal &rsig(*static_cast<OutputStreamSignal*>(_rsig.ptr()));
	}else if(_rsig->dynamicTypeId() == InputOutputStreamSignal::staticTypeId()){
		InputOutputStreamSignal &rsig(*static_cast<InputOutputStreamSignal*>(_rsig.ptr()));
	}else */if(_rsig->dynamicTypeId() == StreamErrorSignal::staticTypeId()){
		//StreamErrorSignal &rsig(*static_cast<StreamErrorSignal*>(_rsig.ptr()));
		state = SendError;
	}else if(_rsig->dynamicTypeId() == FetchSlaveSignal::staticTypeId()){
		//FetchSlaveSignal &rsig(*static_cast<FetchSlaveSignal*>(_rsig.ptr()));
		psig = static_cast<FetchSlaveSignal*>(_rsig.release());
		idbg((void*)this<<" Received slavesignal");
		state = SendNextStream;
	}else return NOK;
	idbg("success");
	return OK;//success reschedule command for execution
}

// int FetchMasterSignal::receiveSignal(
// 	DynamicPointer<fdt::Signal> &_rsig,
// 	int			_which,
// 	const ObjectUidT&_from,
// 	const fdt::ipc::ConnectionUid *
// ){
// 	psig = static_cast<FetchSlaveSignal*>(_rsig.release());
// 	idbg("");
// 	state = SendNextStream;
// 	return OK;
// }
// int FetchMasterSignal::receiveInputStream(
// 	StreamPointer<InputStream> &_rins,
// 	const FileUidT	& _fuid,
// 	int			_which,
// 	const ObjectUidT&,
// 	const fdt::ipc::ConnectionUid *
// ){
// 	idbg("fuid = "<<_fuid.first<<","<<_fuid.second);
// 	ins = _rins;
// 	fuid = _fuid;
// 	state = SendFirstStream;
// 	return OK;
// }
// int FetchMasterSignal::receiveError(
// 	int _errid, 
// 	const ObjectUidT&_from,
// 	const fdt::ipc::ConnectionUid *_conid
// ){
// 	idbg("");
// 	state = SendError;
// 	return OK;
// }

//-----------------------------------------------------------------------------------
// FetchSlaveSignal
//-----------------------------------------------------------------------------------

FetchSlaveSignal::FetchSlaveSignal(): fromv(0xffffffff, 0xffffffff), filesz(-10), streamsz(-1), requid(0){
	idbg(""<<(void*)this);
	serialized = false;
}

FetchSlaveSignal::~FetchSlaveSignal(){
	idbg(""<<(void*)this);
	cassert(serialized);
	print();
// 	if(fromv.first != 0xffffffff){
// 		idbg("unsuccessfull sent");
// 		//signal fromv object to die
// 		Manager::the().signalObject(fromv.first, fromv.second, fdt::S_RAISE | fdt::S_KILL);
// 	}
}
void FetchSlaveSignal::print()const{
	idbg((void*)this<<" FetchSlaveSignal:");
	idbg("filesz = "<<this->filesz<<" streamsz = "<<this->streamsz<<" requid = "<<requid);
	idbg("fuid.first = "<<fuid.first<<" fuid.second = "<<fuid.second);
	idbg("siguid.first = "<<siguid.first<<" siguid.second = "<<siguid.second);
}
int FetchSlaveSignal::sent(const fdt::ipc::ConnectionUid &_rconid){
	idbg((void*)this<<"");
	fromv.first = 0xffffffff;
	return BAD;
}
uint32 FetchSlaveSignal::ipcPrepare(){
	return 0;//foundation::ipc::Service::SynchronousSendFlag;
}
void FetchSlaveSignal::ipcReceived(
	fdt::ipc::SignalUid &_rsiguid
){
	DynamicPointer<fdt::Signal> psig(this);
	conid = fdt::ipc::SignalContext::the().connectionuid;
	if(filesz == -10){
		idbg((void*)this<<" Received FetchSlaveSignal on peer");
		print();
		ObjectUidT	ttov;
		m().readSignalExecuterUid();
		m().signal(psig, m().readSignalExecuterUid());
	}else{
		idbg((void*)this<<" Received FetchSlaveSignal on sender");
		print();
		m().signal(psig, tov.first, tov.second);
	}
}
// Executed on peer within the signal executer
int FetchSlaveSignal::execute(
	DynamicPointer<Signal> &_rthis_ptr,
	uint32 _evs,
	fdt::SignalExecuter& _rce,
	const SignalUidT &,
	TimeSpec &
){
	idbg((void*)this<<"");
	_rce.sendSignal(_rthis_ptr, siguid);
	return BAD;
}

void FetchSlaveSignal::destroyDeserializationStream(
	OutputStream *&_rpos, int64 &_rsz, uint64 &_roff, int _id
){
	idbg((void*)this<<" Destroy deserialization <"<<_id<<"> sz "<<_rsz<<" streamptr "<<(void*)_rpos);
	if(_rsz < 0){
		//there was an error
		filesz = -1;
	}
	delete _rpos;
}

int FetchSlaveSignal::createDeserializationStream(
	OutputStream *&_rpos, int64 &_rsz, uint64 &_roff, int _id
){
	idbg((void*)this<<" "<<_id<<" "<<filesz<<' '<<fuid.first<<' '<<fuid.second);
	if(_id) return NOK;
	if(filesz <= 0) return NOK;
	
	StreamPointer<OutputStream>		sp;
	fdt::RequestUid				requid;
	Manager::the().fileManager().stream(sp, fuid, requid, fdt::file::Manager::Forced);
	if(!sp){
		idbg("");
		return BAD;
	}
	
	idbg((void*)this<<" Create deserialization <"<<_id<<"> sz "<<_rsz<<" streamptr "<<(void*)sp.ptr());
	
	cassert(sp);
	_rpos = sp.release();
	_rsz = streamsz;
	return OK;
}

void FetchSlaveSignal::destroySerializationStream(
	InputStream *&_rpis, int64 &_rsz, uint64 &_roff, int _id
){
	idbg((void*)this<<" doing nothing as the stream will be destroied when the signal will be destroyed");
}

int FetchSlaveSignal::createSerializationStream(
	InputStream *&_rpis, int64 &_rsz, uint64 &_roff, int _id
){
	if(_id || !ins.ptr()) return NOK;
	idbg((void*)this<<" Create serialization <"<<_id<<"> sz "<<_rsz);
	_rpis = ins.ptr();
	_rsz = streamsz;
	return OK;
}

//-----------------------------------------------------------------------------------
// SendStringSignal
//-----------------------------------------------------------------------------------

void SendStringSignal::ipcReceived(
	fdt::ipc::SignalUid &_rsiguid
){
	DynamicPointer<fdt::Signal> psig(this);
	conid = fdt::ipc::SignalContext::the().connectionuid;;
	m().signal(psig, tov.first, tov.second);
}

// int SendStringSignal::execute(concept::Connection &_rcon){
// 	return _rcon.receiveString(str, concept::Connection::RequestUidT(0, 0), 0, fromv, &conid);
// }

//-----------------------------------------------------------------------------------
// SendStreamSignal
//-----------------------------------------------------------------------------------

void SendStreamSignal::ipcReceived(
	fdt::ipc::SignalUid &_rsiguid
){
	DynamicPointer<fdt::Signal> psig(this);
	conid = fdt::ipc::SignalContext::the().connectionuid;;
	m().signal(psig, tov.first, tov.second);
}

void SendStreamSignal::destroyDeserializationStream(
	OutputStream *&_rpos, int64 &_rsz, uint64 &_roff, int _id
){
	idbg("Destroy deserialization <"<<_id<<"> sz "<<_rsz);
}
int SendStreamSignal::createDeserializationStream(
	OutputStream *&_rpos, int64 &_rsz, uint64 &_roff, int _id
){
	if(_id) return NOK;
	idbg("Create deserialization <"<<_id<<"> sz "<<_rsz);
	if(dststr.empty()/* || _rps.second < 0*/) return NOK;
	idbg("File name: "<<this->dststr);
	//TODO:
	int rv = Manager::the().fileManager().stream(this->iosp, this->dststr.c_str(), fdt::file::Manager::NoWait);
	if(rv){
		idbg("Oops, could not open file");
		return BAD;
	}else{
		_rpos = static_cast<OutputStream*>(this->iosp.ptr());
	}
	return OK;
}
void SendStreamSignal::destroySerializationStream(
	InputStream *&_rpis, int64 &_rsz, uint64 &_roff, int _id
){
	idbg("doing nothing as the stream will be destroied when the signal will be destroyed");
}
int SendStreamSignal::createSerializationStream(
	InputStream *&_rpis, int64 &_rsz, uint64 &_roff, int _id
){
	if(_id) return NOK;
	idbg("Create serialization <"<<_id<<"> sz "<<_rsz);
	//The stream is already opened
	_rpis = static_cast<InputStream*>(this->iosp.ptr());
	_rsz = this->iosp->size();
	return OK;
}

// int SendStreamSignal::execute(concept::Connection &_rcon){
// 	{
// 	StreamPointer<InputStream>	isp(static_cast<InputStream*>(iosp.release()));
// 	idbg("");
// 	_rcon.receiveInputStream(isp, concept::Connection::FileUidT(0,0), concept::Connection::RequestUidT(0, 0), 0, fromv, &conid);
// 	idbg("");
// 	}
// 	return _rcon.receiveString(dststr, concept::Connection::RequestUidT(0, 0), 0, fromv, &conid);
// }

//-----------------------------------------------------------------------------------

}//namespace alpha

}//namespace concept
