#include "system/thread.hpp"
#include "system/debug.hpp"
#include "system/cassert.hpp"
#include "system/mutex.hpp"

#include "system/mutualstore.hpp"
#include "utility/dynamictype.hpp"
#include "utility/dynamicpointer.hpp"
#include "utility/shared.hpp"

#include <vector>

#ifdef HAS_GNU_ATOMIC
#include <ext/atomicity.h>
#endif

namespace solid{

//---------------------------------------------------------------------
//----	Shared	----
//---------------------------------------------------------------------

namespace{
typedef MutualStore<Mutex>	MutexStoreT;
#ifdef HAS_SAFE_STATIC
MutexStoreT &mutexStore(){
	static MutexStoreT		mtxstore(3, 2, 2);
	return mtxstore;
}

uint32 specificId(){
	static const uint32 id(Thread::specificId());
	return id;
}

#else

MutexStoreT &mutexStoreStub(){
	static MutexStoreT		mtxstore(3, 2, 2);
	return mtxstore;
}

uint32 specificIdStub(){
	static const uint32 id(Thread::specificId());
	return id;
}


void once_cbk_store(){
	mutexStoreStub();
}

void once_cbk_specific(){
	specificIdStub();
}

MutexStoreT &mutexStore(){
	static boost::once_flag once = BOOST_ONCE_INIT;
	boost::call_once(&once_cbk_store, once);
	return mutexStoreStub();
}

uint32 specificId(){
	static boost::once_flag once = BOOST_ONCE_INIT;
	boost::call_once(&once_cbk_specific, once);
	return specificIdStub();
}
	

#endif

}//namespace

/*static*/ Mutex& Shared::mutex(void *_pv){
	return mutexStore().at((uint32)reinterpret_cast<ulong>(_pv));
}
Shared::Shared(){
	mutexStore().safeAt((uint32)reinterpret_cast<ulong>(this));
}
Mutex& Shared::mutex(){
	return mutex(this);
}

//---------------------------------------------------------------------
//----	DynamicPointer	----
//---------------------------------------------------------------------

void DynamicPointerBase::clear(DynamicBase *_pdyn){
	cassert(_pdyn);
	if(!_pdyn->release()) delete _pdyn;
}

void DynamicPointerBase::use(DynamicBase *_pdyn){
	_pdyn->use();
}

void DynamicPointerBase::storeSpecific(void *_pv)const{
	Thread::specific(specificId(), _pv);
}

/*static*/ void* DynamicPointerBase::fetchSpecific(){
	return Thread::specific(specificId());
}


struct DynamicMap::Data{
	Data(){}
	Data(Data& _rd):fncvec(_rd.fncvec){}
	typedef std::vector<FncT>	FncVectorT;
	FncVectorT fncvec;
};

/*static*/ uint32 DynamicMap::generateId(){
	static uint32 u(0);
	Thread::gmutex().lock();
	uint32 v = ++u;
	Thread::gmutex().unlock();
	return v;
}
#ifdef HAS_SAFE_STATIC

static Mutex & dynamicMutex(){
	static Mutex mtx;
	return mtx;
}
#else
static Mutex & dynamicMutexStub(){
	static Mutex mtx;
	return mtx;
}

void once_cbk_dynamic(){
	dynamicMutexStub();
}

static Mutex & dynamicMutex(){
	static boost::once_flag once = BOOST_ONCE_INIT;
	boost::call_once(&once_cbk_dynamic, once);
	return dynamicMutexStub();
}
#endif

void DynamicRegistererBase::lock(){
	dynamicMutex().lock();
}
void DynamicRegistererBase::unlock(){
	dynamicMutex().unlock();
}

DynamicMap::DynamicMap(DynamicMap *_pdm):d(*(_pdm ? new Data(_pdm->d) : new Data)){
}

DynamicMap::~DynamicMap(){
	delete &d;
}

void DynamicMap::callback(uint32 _tid, FncT _pf){
	if(_tid >= d.fncvec.size()){
		d.fncvec.resize(_tid + 1);
	}
	//cassert(!d.fncvec[_tid]);
	d.fncvec[_tid] = _pf;
}

DynamicMap::FncT DynamicMap::callback(uint32 _id)const{
	FncT pf = NULL;
	if(_id < d.fncvec.size()){
		pf = d.fncvec[_id];
	}
	return pf;
}



DynamicBase::~DynamicBase(){}

DynamicMap::FncT DynamicBase::callback(const DynamicMap &_rdm){
	return NULL;
}

/*virtual*/ void DynamicBase::use(){
	idbgx(Debug::utility, "DynamicBase");
}

//! Used by DynamicPointer to know if the object must be deleted
/*virtual*/ int DynamicBase::release(){
	idbgx(Debug::utility, "DynamicBase");
	return 0;
}

/*virtual*/ bool DynamicBase::isTypeDynamic(uint32 _id)const{
	return false;
}

void DynamicSharedImpl::doUse(){
	idbgx(Debug::utility, "DynamicSharedImpl");
#ifdef HAS_GNU_ATOMIC
	__gnu_cxx:: __atomic_add_dispatch(&usecount, 1);
#else
	Locker<Mutex>	lock(this->mutex());
	++usecount;
#endif
}

int DynamicSharedImpl::doRelease(){
	idbgx(Debug::utility, "DynamicSharedImpl");
#ifdef HAS_GNU_ATOMIC
	const int rv = __gnu_cxx::__exchange_and_add_dispatch(&usecount, -1) - 1;
#else
	Locker<Mutex>	lock(this->mutex());
	--usecount;
	const int rv = usecount;
#endif
	return rv;
}

}//namespace solid
