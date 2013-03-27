#ifndef SYSTEM_CSTRING_HPP
#define SYSTEM_CSTRING_HPP

#include "system/common.hpp"

namespace solid{

const char * char_to_cstring(unsigned _c);

//! Some cross platform cstring utility functions
struct cstring{
    //! Equivalent to strcmp
    static int cmp(const char* _s1, const char *_s2);
    //! Equivalent to strncmp
    static int ncmp(const char* _s1, const char *_s2, uint _len);
    //! Equivalent to strcasecmp
    static int casecmp(const char* _s1, const char *_s2);
    //! Equivalent to strncasecmp
    static int ncasecmp(const char* _s1, const char *_s2, uint _len);
	static size_t nlen(const char *s, size_t maxlen);
    template <class T, T C1>
	static T* find(T *_pc){
		while(*_pc != C1) ++_pc;
		return _pc;
	}
	
	template <class T, T C1, T C2>
	static T* find(T *_pc){
		while(*_pc != C2 && *_pc != C1) ++_pc;
		return _pc;
	}
	
	template <class T, T C1, T C2, T C3>
	static T* find(T *_pc){
		while(*_pc != C2 && *_pc != C1 && *_pc != C3) ++_pc;
		return _pc;
	}
	template <class T, T C1, T C2, T C3>
	static T* findNot(T *_pc){
		while(*_pc == C2 || *_pc == C1 || *_pc == C3) ++_pc;
		return _pc;
	}
};

}//namespace solid

#endif
