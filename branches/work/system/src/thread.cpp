/* Implementation file thread.cpp
	
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


#include <cerrno>
#include <cstring>
#include <pthread.h>
#include <limits.h>

#include "system/timespec.hpp"
#include "system/thread.hpp"
#include "system/debug.hpp"
#include "system/condition.hpp"
#include "system/exception.hpp"
#include "system/cassert.hpp"

#include "mutexpool.hpp"

#if defined(ON_FREEBSD)
#include <pmc.h>
#elif defined(ON_MACOS)
#else
#include <sys/sysinfo.h>
#endif


#if defined(ON_WINDOWS)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <windows.h>
//#include <io.h>
#else
#include <unistd.h>
#endif

#if defined(ON_MACOS)
#include <mach/mach_time.h>
#endif


#include <unistd.h>
#include <limits.h>

struct Cleaner{
	~Cleaner(){
		Thread::cleanup();
	}
};

/*static*/ const char* src_file_name(char const *_fname){
	static const unsigned fileoff = (strlen(__FILE__) - strlen(strstr(__FILE__, "system/src")));
	return _fname + fileoff;
}

void throw_exception(const char* const _pt, const char * const _file, const int _line, const char * const _func){
	throw Exception<const char*>(_pt, _file, _line, _func);
}


static const pthread_once_t	oncek = PTHREAD_ONCE_INIT;

struct ThreadData{
	enum {
		MutexPoolSize = 4,
		FirstSpecificId = 0
	};
	//ThreadData();
	//ThreadData():crtthread_key(0), thcnt(0), once_key(PTHREAD_ONCE_INIT){}
	pthread_key_t					crtthread_key;
	uint32    						thcnt;
	pthread_once_t					once_key;
	Condition						gcon;
	Mutex							gmut;
	FastMutexPool<MutexPoolSize>	mutexpool;
	ThreadData():crtthread_key(0), thcnt(0), once_key(oncek){
	}
};

//ThreadData::ThreadData(){
//	crtthread_key = 0;
//	thcnt = 0;
//	once_key = PTHREAD_ONCE_INIT;
//}
static ThreadData& threadData(){
	//TODO: staticproblem
	static ThreadData td;
	return td;
}

Cleaner             			cleaner;
//static unsigned 				crtspecid = 0;
//*************************************************************************
/*static*/ const TimeSpec TimeSpec::max(0xffffffff, 0xffffffff);
#ifdef NINLINES
#include "system/timespec.ipp"
#endif

/*static*/ TimeSpec TimeSpec::createRealTime(){
	TimeSpec ct;
	return ct.currentRealTime();
}
/*static*/ TimeSpec TimeSpec::createMonotonic(){
	TimeSpec ct;
	return ct.currentMonotonic();
}

#if		defined(ON_WINDOWS)

struct TimeStartData{
	TimeStartData(){
		st = time(NULL);
		sc = clock();
	}
	time_t							st;
	UnsignedType<clock_t>::Type		sc;
};

const TimeSpec& TimeSpec::currentRealTime(){
	//TODO: static problem
	static TimeStartData		tsd;
	UnsignedType<clock_t>::Type	cc = clock();
	uint32						secs  = 0;
	uint32						nsecs = 0;
	
	if(tsd.sc <= cc){
		secs = (cc - tsd.sc)/CLOCKS_PER_SEC;
		nsecs = ((((cc - d.sc) % CLOCKS_PER_SEC) * 1000)/CLOCKS_PER_SEC) * 1000000;
	}else{
		//NOTE: find a better way
		UnsignedType<clock_t>::Type tc = cc + (0xffffffff - tsd.sc);
		secs = (tc)/CLOCKS_PER_SEC;
		nsecs = ((((tc) % CLOCKS_PER_SEC) * 1000)/CLOCKS_PER_SEC) * 1000000;
		tsd.sc = cc;
		tsd.st = time(NULL);
	}
	this->set(tsd.st + secs, nsecs);
	return *this;
}

const TimeSpec& TimeSpec::currentMonotonic(){
	static TimeStartData		tsd;
	UnsignedType<clock_t>::Type	cc = clock();
	uint32						secs  = 0;
	uint32						nsecs = 0;
	if(tsd.sc <= cc){
		secs = (cc - tsd.sc)/CLOCKS_PER_SEC;
		nsecs = ((((cc - tsd.sc) % CLOCKS_PER_SEC) * 1000)/CLOCKS_PER_SEC) * 1000000;
	}else{
		//NOTE: find a better way
		UnsignedType<clock_t>::Type tc = cc + (0xffffffff - tsd.sc);
		secs = (tc)/CLOCKS_PER_SEC;
		nsecs = ((((tc) % CLOCKS_PER_SEC) * 1000)/CLOCKS_PER_SEC) * 1000000;
		tsd.sc = cc;
		tsd.st = time(NULL);
	}
	this->set(d.st + secs, nsecs);
	return *this;
}

#elif	defined(ON_MACOS)

struct TimeStartData{
	TimeStartData(){
		st = time(NULL);
		stns = mach_absolute_time();
		stns -= (stns % 1000000000);
	}
	uint64	stns;
	time_t	st;
};
struct HelperMatchTimeBase: mach_timebase_info_data_t{
    HelperMatchTimeBase(){
		this->denom = 0;
		this->numer = 0;
		::mach_timebase_info((mach_timebase_info_data_t*)this);
	}
};
const TimeSpec& TimeSpec::currentRealTime(){
	static TimeStartData		tsd;
	static HelperMatchTimeBase	info;
	uint64				difference = mach_absolute_time() - tsd.stns;

	uint64 elapsednano = difference * (info.numer / info.denom);

	this->seconds(tsd.st + elapsednano / 1000000000);
	this->nanoSeconds(elapsednano % 1000000000);
	return *this;
}

const TimeSpec& TimeSpec::currentMonotonic(){
	static uint64			tsd(mach_absolute_time());
	static HelperMatchTimeBase	info;
	uint64				difference = mach_absolute_time() - tsd;

	uint64 elapsednano = difference * (info.numer / info.denom);

	this->seconds(elapsednano / 1000000000);
	this->nanoSeconds(elapsednano % 1000000000);
	return *this;
}

#else

const TimeSpec& TimeSpec::currentRealTime(){
	clock_gettime(CLOCK_REALTIME, this);
	return *this;
}

const TimeSpec& TimeSpec::currentMonotonic(){
	clock_gettime(CLOCK_MONOTONIC, this);
	return *this;
}

#endif

#if 	defined(USTLMUTEX)
#elif	defined(UBOOSTMUTEX)
#else
//*************************************************************************
#ifdef NINLINES
#include "system/mutex.ipp"
#endif
//*************************************************************************
#ifdef NINLINES
#include "system/condition.ipp"
#endif
//*************************************************************************
#ifdef NINLINES
#include "system/synchronization.ipp"
#endif
//-------------------------------------------------------------------------
int Condition::wait(Locker<Mutex> &_lock, const TimeSpec &_ts){
	return pthread_cond_timedwait(&cond,&_lock.m.mut, &_ts);
}
//-------------------------------------------------------------------------
int Mutex::timedLock(const TimeSpec &_rts){
#if defined (ON_MACOS)
    return -1;
#else
	return pthread_mutex_timedlock(&mut,&_rts);
#endif
}
//-------------------------------------------------------------------------
int Mutex::reinit(Type _type){
#ifdef UPOSIXMUTEX
	pthread_mutex_destroy(&mut);
	pthread_mutexattr_t att;
	pthread_mutexattr_init(&att);
	pthread_mutexattr_settype(&att, (int)_type);
	return pthread_mutex_init(&mut,&att);
#else
	return -1;
#endif
}
#endif
//*************************************************************************
struct MainThread: Thread{
#ifdef _WIN32
#else
	MainThread(bool _detached = true, pthread_t _th = 0):Thread(_detached, _th){}
#endif
	void run(){}
};
/*static*/ void Thread::init(){
	if(pthread_key_create(&threadData().crtthread_key, NULL)) throw -1;
	static MainThread	t(false, pthread_self());
	Thread::current(&t);
}
//-------------------------------------------------------------------------
void Thread::cleanup(){
	pthread_key_delete(threadData().crtthread_key);
}
//-------------------------------------------------------------------------
void Thread::sleep(ulong _msec){
	usleep(_msec*1000);
}
//-------------------------------------------------------------------------
inline void Thread::enter(){
	ThreadData &td(threadData());
    //td.gmut.lock();
    ++td.thcnt; td.gcon.broadcast();
    //td.gmut.unlock();
}
//-------------------------------------------------------------------------
inline void Thread::exit(){
	ThreadData &td(threadData());
    td.gmut.lock();
    --td.thcnt; td.gcon.broadcast();
    td.gmut.unlock();
}
//-------------------------------------------------------------------------
Thread * Thread::current(){
	return reinterpret_cast<Thread*>(pthread_getspecific(threadData().crtthread_key));
}
//-------------------------------------------------------------------------
long Thread::processId(){
	return getpid();
}
//-------------------------------------------------------------------------
Thread::Thread(bool _detached, pthread_t _th):th(_th), dtchd(_detached), pcndpair(NULL){
}
//-------------------------------------------------------------------------
Thread::~Thread(){
	for(SpecVecT::iterator it(specvec.begin()); it != specvec.end(); ++it){
		if(it->first){
			cassert(it->second);
			(*it->second)(it->first);
		}
	}
}
//-------------------------------------------------------------------------
void Thread::dummySpecificDestroy(void*){
}
//-------------------------------------------------------------------------
/*static*/ unsigned Thread::processorCount(){
#if		defined(ON_SOLARIS)
	return 1;
#elif	defined(ON_FREEBSD)
	return 1;//pmc_ncpu();//sysconf(_SC_NPROCESSORS_ONLN)
#elif	defined(ON_MACOS)
    return 1;
#else
	return get_nprocs();
#endif
}
//-------------------------------------------------------------------------
int Thread::join(){
	if(pthread_equal(th, pthread_self())) return NOK;
	if(detached()) return NOK;
	int rcode =  pthread_join(this->th, NULL);
	return rcode;
}
//-------------------------------------------------------------------------
int Thread::detached() const{
	//Locker<Mutex> lock(mutex());
	return dtchd;
}
//-------------------------------------------------------------------------
int Thread::detach(){
	Locker<Mutex> lock(mutex());
	if(detached()) return OK;
	int rcode = pthread_detach(this->th);
	if(rcode == OK)	dtchd = 1;
	return rcode;
}
//-------------------------------------------------------------------------
unsigned Thread::specificId(){
	//TODO: staticproblem
	static unsigned sid = ThreadData::FirstSpecificId - 1;
	Locker<Mutex> lock(gmutex());
	return ++sid;
}
//-------------------------------------------------------------------------
void Thread::specific(unsigned _pos, void *_psd, SpecificFncT _pf){
	Thread *pct = current();
	cassert(pct);
	if(_pos >= pct->specvec.size()) pct->specvec.resize(_pos + 4);
	//This is safe because pair will initialize with NULL on resize
	if(pct->specvec[_pos].first){
		(*pct->specvec[_pos].second)(pct->specvec[_pos].first);
	}
	pct->specvec[_pos] = SpecPairT(_psd, _pf);
	//return _pos;
}
//-------------------------------------------------------------------------
// unsigned Thread::specific(void *_psd){
// 	cassert(current());
// 	current()->specvec.push_back(_psd);
// 	return current()->specvec.size() - 1;
// }
//-------------------------------------------------------------------------
void* Thread::specific(unsigned _pos){
	cassert(current() && _pos < current()->specvec.size());
	return current()->specvec[_pos].first;
}
Mutex& Thread::gmutex(){
	return threadData().gmut;
}
//-------------------------------------------------------------------------
int Thread::current(Thread *_ptb){
	pthread_setspecific(threadData().crtthread_key, _ptb);
	return OK;
}
//-------------------------------------------------------------------------
Mutex& Thread::mutex()const{
	return threadData().mutexpool.getr(this);
}
//-------------------------------------------------------------------------
#ifndef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN 4096
#endif
int Thread::start(int _wait, int _detached, ulong _stacksz){	
	Locker<Mutex> locker(mutex());
	idbgx(Dbg::system, "starting thread "<<th);
	if(th) return BAD;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	if(_detached){
		if(pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED)){
			pthread_attr_destroy(&attr);
			edbgx(Dbg::system, "pthread_attr_setdetachstate: "<<strerror(errno));
			return BAD;
		}
	}
	if(_stacksz){
		if(_stacksz < PTHREAD_STACK_MIN){
			_stacksz = PTHREAD_STACK_MIN;
		}
		int rv = pthread_attr_setstacksize(&attr, _stacksz);
		if(rv){
			edbgx(Dbg::system, "pthread_attr_setstacksize "<<_stacksz<<": "<<strerror(errno));
			pthread_attr_destroy(&attr);
			return BAD;
		}
	}
	ConditionPairT cndpair;
	cndpair.second = 1;
	if(_wait){
		Locker<Mutex>	lock2(gmutex());
		pcndpair = &cndpair;
		Thread::enter();
		//NOTE: DO not access any thread member from now - the thread may be detached
		if(pthread_create(&th,&attr,&Thread::th_run,this)){
			pthread_attr_destroy(&attr);
			edbgx(Dbg::system, "pthread_create: "<<strerror(errno));
			Thread::exit();
			return BAD;
		}
		while(cndpair.second){
			cndpair.first.wait(lock2);
		}
	}else{
		{
			Locker<Mutex>	lock2(gmutex());
			Thread::enter();
		}
		//NOTE: DO not access any thread member from now - the thread may be detached
		if(pthread_create(&th,&attr,&Thread::th_run,this)){
			pthread_attr_destroy(&attr);
			edbgx(Dbg::system, "pthread_create: "<<strerror(errno));
			Thread::exit();
			return BAD;
		}
	}
	pthread_attr_destroy(&attr);
	vdbgx(Dbg::system, "");
	return OK;
}
//-------------------------------------------------------------------------
void Thread::signalWaiter(){
	pcndpair->second = 0;
	pcndpair->first.signal();
	pcndpair = NULL;
}
//-------------------------------------------------------------------------
int Thread::waited(){
	return pcndpair != NULL;
}
//-------------------------------------------------------------------------
void Thread::waitAll(){
	ThreadData &td(threadData());
    Locker<Mutex> lock(td.gmut);
    while(td.thcnt != 0) td.gcon.wait(lock);
}
//-------------------------------------------------------------------------
void* Thread::th_run(void *pv){
	vdbgx(Dbg::system, "thrun enter "<<pv);
	Thread	*pth(reinterpret_cast<Thread*>(pv));
	//Thread::enter();
	Thread::current(pth);
	if(pth->waited()){
		pth->signalWaiter();
		Thread::yield();
	}
	pth->prepare();
	pth->run();
	pth->unprepare();
	if(pth->detached()) delete pth;
	vdbgx(Dbg::system, "thrun exit "<<pv);
	Thread::exit();
	return NULL;
}

//-------------------------------------------------------------------------
