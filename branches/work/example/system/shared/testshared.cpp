#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/iterator/iterator_concepts.hpp>

#include "system/sharedbackend.hpp"

#include "system/debug.hpp"

#include <memory>
#include <iostream>
#include <deque>
#include <vector>
using namespace std;

struct Test{
	int v;
	Test(int _v = 0):v(_v){
		//idbg("");
	}
	~Test(){
		//idbg("");
	}
	
	int test(int _x){
		return v * _x;
	}
};

static SharedStub	ss(0);

template <class T>
class SharedPtr{
public:
	static void delT(void *_pv){
		delete reinterpret_cast<T*>(_pv);
	}
	typedef SharedPtr<T> ThisT;
	SharedPtr():pss(&ss){}
	
	SharedPtr(T *_pt):pss(SharedBackend::the.create(_pt, &delT)){
	}
	
	SharedPtr(const ThisT& _rptr):pss(_rptr.pss){
		idbg("");
		SharedBackend::use(*pss);
	}
	SharedPtr(SharedPtr&& __r) noexcept
      : ThisT(std::move(__r)) {
		  idbg("");
	}
	~SharedPtr(){
		T* pt = get();
		if(SharedBackend::the.release(*pss)){
			delete pt;
		}
	}
	
	ThisT& operator=(const ThisT&_rptr){
		idbg("");
		release();
		pss = _rptr.pss;
		SharedBackend::the.use(*pss);
		return *this;
	}
	
	ThisT& operator=(ThisT&& __r) noexcept{
		this->ThisT::operator=(std::move(__r));
		idbg("");
		return *this;
	}
	
	void release()noexcept{
		T* pt = get();
		if(SharedBackend::the.release(*pss)){
			delete pt;
		}
		pss = &ss;
	}
	
	bool empty()const noexcept{
		return pss->ptr == NULL;
	}
	T* ptr()const noexcept{
		return reinterpret_cast<T*>(pss->ptr);
	}
	T* get()const{
		return reinterpret_cast<T*>(pss->ptr);
	}
	T* operator->()const noexcept{
		return ptr();
	}
private:
	SharedStub *pss;
};
template <class T>
class WeakPtr;

template <class T>
class NoSharedPtr{
public:
	typedef NoSharedPtr<T> ThisT;
	
	NoSharedPtr():p(NULL){}
	
	NoSharedPtr(T *_p):p(_p){
	}
	
	NoSharedPtr(const ThisT& _rptr):p(_rptr.p){
		_rptr.p = NULL;
	}
	~NoSharedPtr(){
		delete p;
	}
	
	ThisT& operator=(const ThisT&_rptr){
		if(!empty()){
			release();
		}
		p = _rptr.p;
		_rptr.p = NULL;
		return *this;
	}
	
	void release(){
		delete p;
		p = NULL;
	}
	
	bool empty()const{
		return p == NULL;
	}
	T* ptr()const{
		return p;
	}
	
	T* operator->()const{
		return ptr();
	}
private:
	friend class WeakPtr<T>;
	mutable T *p;
};

template <class T>
class WeakPtr{
public:
	typedef WeakPtr<T> ThisT;
	
	WeakPtr():p(NULL){}
	
	WeakPtr(T *_p):p(_p){
	}
	
	WeakPtr(const ThisT& _rptr):p(_rptr.p){
	}
	WeakPtr(const NoSharedPtr<T>& _rptr):p(_rptr.p){
	}
	~WeakPtr(){
	}
	
	ThisT& operator=(const ThisT&_rptr){
		if(!empty()){
			release();
		}
		p = _rptr.p;
		return *this;
	}
	
	void release(){
		p = NULL;
	}
	
	bool empty()const{
		return p == NULL;
	}
	T* ptr()const{
		return p;
	}
	
	T* operator->()const{
		return ptr();
	}
private:
	mutable T *p;
};



//typedef boost::shared_ptr<Test>		TestSharedPtrT;
//typedef std::shared_ptr<Test>		TestSharedPtrT;
typedef SharedPtr<Test>				TestSharedPtrT;
typedef WeakPtr<Test>				TestWeakPtrT;
typedef deque<TestSharedPtrT>		TestDequeT;
//typedef vector<TestWeakPtrT>		TestVectorT;
typedef vector<TestSharedPtrT>		TestVectorT;
typedef deque<boost::thread*>		ThreadVectorT;

struct Runner{
	void operator()();
};

static TestDequeT &testdeq = *(new TestDequeT);

int main(int argc, char *argv[]){
	if(argc != 4){
		cout<<"Use: ./example_testshare repeatcnt objcnt thrcnt"<<endl;
		return 0;
	}
#ifdef UDEBUG
	std::string s;
	Dbg::instance().levelMask("view");
	Dbg::instance().moduleMask("any");
	Dbg::instance().initStdErr(
		false,
		&s
	);
#endif
	int repcnt = atoi(argv[1]);
	int objcnt = atoi(argv[2]);
	int thrcnt = atoi(argv[3]);
	
	TestSharedPtrT		t1(new Test(1));
	TestSharedPtrT		t2;
	
	t2 = t1;
	
	ThreadVectorT thrvec;
	
	while(repcnt--){
		idbg("create "<<objcnt<<" objects:");
		for(int i = 0; i < objcnt; ++i){
			TestSharedPtrT sp(new Test(i));
			testdeq.push_back(sp);
		}
		idbg("create "<<thrcnt<<" threads:");
		for(int i = 0; i < thrcnt; ++i){
			Runner          r;
			boost::thread   *pthr = new boost::thread(r);
			thrvec.push_back(pthr);
		}
		idbg("wait for all threads");
		for(int i = 0; i < thrcnt; ++i){
			thrvec[i]->join();
			delete thrvec[i];
		}
		thrvec.clear();
// 		for(TestDequeT::const_iterator it(testdeq.begin()); it != testdeq.end(); ++it){
// 			delete (*it).get();
// 		}
		testdeq.clear();
	}
	delete &testdeq;
	return 0;
}

void Runner::operator()(){
	TestVectorT  &testvec(*(new TestVectorT));
	//idbg("thread started");
	for(TestDequeT::const_iterator it(testdeq.begin()); it != testdeq.end(); ++it){
		testvec.push_back(*it);
	}
// 	int i = 0;
// 	for(TestVectorT::const_iterator it(testvec.begin()); it != testvec.end(); ++it){
// 		i += (*it)->test(it - testvec.begin());
// 	}
	//idbg("thread done "<<i);
	delete &testvec;
}
