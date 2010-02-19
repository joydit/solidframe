#include <iostream>
#include "system/debug.hpp"
#include "utility/dynamictype.hpp"

using namespace std;

struct AObject: Dynamic<AObject>{
	AObject(int _v):v(_v){}
	int v;
};

struct BObject: Dynamic<BObject>{
	BObject(int _v1, int _v2):v1(_v1), v2(_v2){}
	int v1;
	int v2;
};

struct CObject: Dynamic<CObject, BObject>{
	CObject(int _v1, int _v2, int _v3):Dynamic<CObject, BObject>(_v1, _v2), v3(_v3){}
	int v3;
};

struct DObject: Dynamic<DObject, AObject>{
	DObject(const string& _s, int _v): Dynamic<DObject, AObject>(_v), s(_s){}
	string s;
};


class FirstExecuter{
protected:
	typedef DynamicReceiver<int, FirstExecuter>	IntDynamicReceiverT;
public:
	FirstExecuter(){
	}
	static void dynamicRegister(){
		IntDynamicReceiverT::add<AObject, FirstExecuter>();
		IntDynamicReceiverT::add<BObject, FirstExecuter>();
	}
	void push(const DynamicPointer<> &_dp){
		dr.push(_dp);
	}
	int dynamicExecuteDefault(DynamicPointer<> &_dp);
	int dynamicExecute(const DynamicPointer<AObject> &_rdp);
	int dynamicExecute(const DynamicPointer<BObject> &_rdp);
	
protected:
	IntDynamicReceiverT		dr;
};

int FirstExecuter::dynamicExecuteDefault(DynamicPointer<> &_dp){
	idbg("");
	return -1;
}

int FirstExecuter::dynamicExecute(const DynamicPointer<AObject> &_dp){
	idbg("v = "<<_dp->v);
	return _dp->v;
}

int FirstExecuter::dynamicExecute(const DynamicPointer<BObject> &_dp){
	idbg("v1 = "<<_dp->v1<<" v2 = "<<_dp->v2);
	return _dp->v2;
}

class SecondExecuter: public FirstExecuter{
public:
	SecondExecuter(){
	}
	
	static void dynamicRegister(){
		FirstExecuter::dynamicRegister();
		IntDynamicReceiverT::add<BObject, SecondExecuter>();
		IntDynamicReceiverT::add<CObject, SecondExecuter>();
		IntDynamicReceiverT::add<DObject, SecondExecuter>();
	}
	
	void run();
	
	int dynamicExecute(const DynamicPointer<BObject> &_dp);
	int dynamicExecute(const DynamicPointer<CObject> &_dp);
	int dynamicExecute(const DynamicPointer<DObject> &_dp);
};


void SecondExecuter::run(){
	int rv = dr.prepareExecute();
	idbg("Executing "<<rv<<" calls");
	while(dr.hasCurrent()){
		rv = dr.executeCurrent(*this);
		idbg("call returned "<<rv);
		dr.next();
	}
	//dr.executeCurrent(*this);
}

int SecondExecuter::dynamicExecute(const DynamicPointer<BObject> &_dp){
	idbg("v1 = "<<_dp->v1<<" v2 = "<<_dp->v2);
	return _dp->v1;
}
int SecondExecuter::dynamicExecute(const DynamicPointer<CObject> &_dp){
	idbg("v1 = "<<_dp->v1<<" v2 = "<<_dp->v2<<" v3 "<<_dp->v3);
	return _dp->v1;
}
int SecondExecuter::dynamicExecute(const DynamicPointer<DObject> &_dp){
	idbg("s = "<<_dp->s<<" v = "<<_dp->v);
	return _dp->v;
}

static const DynamicRegisterer<SecondExecuter>	dre;

int main(){
#ifdef UDEBUG
	Dbg::instance().levelMask();
	Dbg::instance().moduleMask();
	Dbg::instance().initStdErr(false);
#endif
	uint32 v = 2;
	uint32 v2 = __sync_add_and_fetch(&v, 2);
	idbg("v2 = "<<v2);
	
	SecondExecuter	e;
	e.push(DynamicPointer<>(new AObject(1)));
	e.push(DynamicPointer<>(new BObject(2,3)));
	e.push(DynamicPointer<>(new CObject(1,2,3)));
	e.push(DynamicPointer<>(new DObject("hello1", 4)));
	e.run();
	e.push(DynamicPointer<>(new AObject(11)));
	e.push(DynamicPointer<>(new BObject(22,33)));
	e.push(DynamicPointer<>(new CObject(11,22,33)));
	e.push(DynamicPointer<>(new DObject("hello2", 44)));
	e.run();
	e.push(DynamicPointer<>(new AObject(111)));
	e.push(DynamicPointer<>(new BObject(222,333)));
	e.push(DynamicPointer<>(new CObject(111,222,333)));
	e.push(DynamicPointer<>(new DObject("hello3", 444)));
	e.run();
	return 0;
}
