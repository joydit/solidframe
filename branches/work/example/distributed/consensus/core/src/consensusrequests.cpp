#include "example/distributed/consensus/core/consensusrequests.hpp"
#include "example/distributed/consensus/core/consensusmanager.hpp"

#include "foundation/ipc/ipcservice.hpp"

#include "system/debug.hpp"

namespace fdt=foundation;

//--------------------------------------------------------------
foundation::Manager& m(){
	return foundation::m();
}
//--------------------------------------------------------------
const foundation::ObjectUidT& serverUid(){
	static const foundation::ObjectUidT uid(fdt::Manager::the().makeObjectUid(11, 10, 0));
	return uid;
}
//--------------------------------------------------------------
void mapSignals(){
	fdt::ipc::Service::the().registerSerializationType<StoreRequest>();
	fdt::ipc::Service::the().registerSerializationType<FetchRequest>();
	fdt::ipc::Service::the().registerSerializationType<EraseRequest>();
}
//--------------------------------------------------------------
StoreRequest::StoreRequest(const std::string&, uint32 _pos):v(0){
	idbg("");
}
StoreRequest::StoreRequest(){
	idbg("");
}
/*virtual*/ void StoreRequest::sendThisToConsensusObject(){
	DynamicPointer<fdt::Signal> sig(this);
	m().signal(sig, serverUid());
}
//--------------------------------------------------------------
FetchRequest::FetchRequest(const std::string&){
	idbg("");
}
FetchRequest::FetchRequest(){
	idbg("");
}
/*virtual*/ void FetchRequest::sendThisToConsensusObject(){
	DynamicPointer<fdt::Signal> sig(this);
	m().signal(sig, serverUid());
}
//--------------------------------------------------------------
EraseRequest::EraseRequest(const std::string&){
	idbg("");
}
EraseRequest::EraseRequest(){
	idbg("");
}
/*virtual*/ void EraseRequest::sendThisToConsensusObject(){
	DynamicPointer<fdt::Signal> sig(this);
	m().signal(sig, serverUid());
}
//--------------------------------------------------------------