#include "system/memorycache.hpp"
#include "system/debug.hpp"
#include <cstring>
#include <vector>
using namespace std;
using namespace solid;

MemoryCache mc(8 * 1024);


struct BaseObject{
	static void operator delete (void *_p, std::size_t _sz){
		mc.free(_p, _sz);
	}
	static void* operator new (std::size_t _sz){
		return mc.allocate(_sz);
	}
};


template <uint16 Sz>
struct TestObject: BaseObject{
	char buf[Sz];
};


int main(int argc, char *argv[]){
	solid::Debug::the().initStdErr(false);
	solid::Debug::the().moduleMask("all");
	
// 	TestObject<4> *p4 = new TestObject<4>;
// 	memset(p4->buf, 0, 4);
// 	delete p4;
	std::vector<TestObject<4>* > vec;
	
	size_t step = 1000;
	size_t repeatcnt = 500;
	size_t cp = repeatcnt * step;
	vec.reserve(cp);
	
	size_t rescnt = mc.reserve(4, cp);
	idbg("Reserved "<<rescnt<<" items");
	//return 0;
	size_t crtcp = 0;
	for(size_t i = 0; i < repeatcnt; ++i){
		crtcp += step;
		idbg("Allocate "<<crtcp<<" items");
		for(size_t j = 0; j < crtcp; ++j){
			vec.push_back(new TestObject<4>);
		}
		idbg("+++++++++++++++++++++++++++++");
		mc.print(4);
		idbg("-----------------------------");
		idbg("delete "<<vec.size()<<" items");
		for(auto it = vec.begin(); it != vec.end(); ++it){
			delete *it;
		}
		idbg("Done delete");
		idbg("+++++++++++++++++++++++++++++");
		mc.print(4);
		idbg("-----------------------------");
		vec.clear();
	}
	return 0;
}

