// protocol/text/writer.hpp
//
// Copyright (c) 2007, 2008 Valentin Palade (vipalade @ gmail . com) 
//
// This file is part of SolidFrame framework.
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.
//
#ifndef SOLID_PROTOCOL_TEXT_WRITER_HPP
#define SOLID_PROTOCOL_TEXT_WRITER_HPP

#include "protocol/text/parameter.hpp"
#include "protocol/text/logger.hpp"
#include "protocol/text/buffer.hpp"
#include "utility/stack.hpp"
#include "utility/holder.hpp"
#include <string>

namespace solid{
typedef std::string String;
namespace protocol{
namespace text{
//! A nonblocking buffer oriented (not line oriented) protocol response builder
/*!
	Here are some characteristics of the writer:<br>
	- it is designed for nonblocking/asynchrounous usage<br>
	- it is buffer oriented not line oriented<br>
	- it is flexible and easely extensible<br>
	
	<b>Overview:</b><br>
		Internally it uses a stack of pairs of a function pointer and a parameter 
		(protocol::Parameter) whith wich the function will be called.
		<br>
		Every function can in turn push new calls in the stack.
		<br>
		There is a very simple and fast state machine based on the return value
		of scheduled functions. The state machine will exit when, either the buffer must be flushed
		and this cannot be done imediatly (wait to flushed asynchrounously), the stack is empty,
		a function return Writer::Failure.<br>
	
	<b>Usage:</b><br>
		Inherit, implement the virtual methods and extend the writer with new 
		writing functions.<br>
		In your protocol (connection) execute loop:<br>
		> push some writing callbacks<br>
		> (repeatedly) call run until BAD or OK is returned<br>
		<br>
		For writing use the defined operator<<(s) and/or callbacks for sending strings/chars/streams etc.<br>
		- Failure usually means that the connection was/must be closed<br>
		- Success means that the stack is empty - it doesnt mean the data was parsed 
		successfully - an error might have occurred and the parser has successfully recovered 
		(use isError)<br>
		
		
	<b>Notes:</b><br>
		- You can safely use pointers to existing parameters within the stack.<br>
		- As an example see test::alpha::Writer (test/foundation/alpha/src/alpha.(h/cpp)).<br>
		- The << operators must be used with care, because although the internal buffer will resize accordigly,
		this is not desirable when scalability is important. So it is the problem of the protocol implemetor
		to ensure that the buffer gets flushed before it's filled/resized.<br>
*/
class Writer{
public:
	typedef int (*FncT)(Writer&, Parameter &_rp);
	enum ReturnValueE{
		Failure = -1,	//!<input closed
		Success = 0,	//!<everything ok, do a pop
		Wait,			//!<Must wait
		Yield,			//!<Must yield the connection
		Continue,		//!<reexecute the top function - no pop
		Error,			//!<parser error - must enter error recovery
		LastReturnValue
	};

	enum ManagementOptionsE{
		ClearLogging,
		ResetLogging,
	};
public:
	//!Writer constructor
	Writer(Logger *_plog = NULL);
	//! Writer destructor
	virtual ~Writer();
	
	//! Sets the internal buffer
	template <class B>
	void buffer(const B &_b){
		doPrepareBuffer(_b.pbeg, _b.pend);
		bh = _b;
	}
	
	//! Gets the internal buffer
	const Buffer& buffer()const;
	
	//! Check if the log is active.
	bool isLogActive()const{return plog != NULL;}
	//! Sheduller push method
	/*!
		\param _pf A pointer to a function
		\param _rp A const reference to a parameter
	*/
	void push(FncT _pf, const Parameter & _rp /*= Parameter()*/);
	//! Sheduller replace top method
	/*!
		\param _pf A pointer to a function
		\param _rp A const reference to a parameter
	*/
	void replace(FncT _pf, const Parameter & _rp = Parameter());
	//! Sheduller push method
	/*!
		\param _pf A pointer to a function
		\retval Parameter Returns a reference to the actual parameter the function will be called with, so you can get pointers to this parameter.
	*/
	Parameter &push(FncT _pf);
	//! Check if the stack is empty
	bool empty()const{return fs.empty();}
	//! The state machine algorithm
	int run();
	//! Convenient method for puting one char on output
	void putChar(char _c1);
	//! Convenient method for puting two chars on output
	void putChar(char _c1, char _c2);
	//! Convenient method for puting three chars on output
	void putChar(char _c1, char _c2, char _c3);
	//! Convenient method for puting fourth chars on output
	void putChar(char _c1, char _c2, char _c3, char _c4);
    //! Convenient method for silently (no logging) puting one char on output
    void putSilentChar(char _c1);
    //! Convenient method for silently (no logging) puting two chars on output
	void putSilentChar(char _c1, char _c2);
	//! Convenient method for silently (no logging) puting three chars on output
	void putSilentChar(char _c1, char _c2, char _c3);
	//! Convenient method for silently (no logging) puting fourth chars on output
	void putSilentChar(char _c1, char _c2, char _c3, char _c4);
	//! Convenient method for putting a string on the output
	void putString(const char* _s, uint32 _sz);
	//! Convenient method for silently putting a string on the output
	void putSilentString(const char* _s, uint32 _sz);
	//! Convenient method for putting a number on the output
	void put(uint32 _v);
	//! Convenient method for silently putting a number on the output
	void putSilent(uint32 _v);
    
    
    //! Convenient method for putting a number on the output
	void put(uint64 _v);
	//! Convenient method for silently putting a number on the output
	void putSilent(uint64 _v);
//writing callbacks
	//! Callback for returning a certain value
	template <bool B>
	static int returnValue(Writer &_rw, Parameter &_rp);
	//! Callback for trying to flush the buffer
	/*!
		The buffer will be flushed if it is filled above a certain level.
	*/
	static int flush(Writer &_rw, Parameter &_rp);
	//! Callback for forced flush the buffer
	/*!
		Usually this is the last callback called for a response.
	*/
	static int flushAll(Writer &_rw, Parameter &_rp);
	//! Callback called after a flush
	static int doneFlush(Writer &_rw, Parameter &_rp);
	//! Callback for sending a stream
	static int putStream(Writer &_rw, Parameter &_rp);
	//! Callback for sending a string/atom
	static int putAtom(Writer &_rw, Parameter &_rp);
	//! Callback for sending a single char
	static int putChar(Writer &_rw, Parameter &_rp);
	//! Callback for sending an uint32
	static int putUInt32(Writer &_rw, Parameter &_rp);
	//! Callback for sending an uint64
	static int putUInt64(Writer &_rw, Parameter &_rp);
	//! Callback for manage opperations
	static int manage(Writer &_rw, Parameter &_rp);
	//! Callback for external (write) opperations
	/*!
		This callback is central for building complex responses and can be used for all kinds
		of opperations from writing, waiting for asynchrounous events/data, computations etc.
		Using this callback one can control the response building from outside writer.
	*/
	template <class C>
    static int reinit(Writer &_rw, Parameter &_rp){
        return reinterpret_cast<C*>(_rp.a.p)->reinitWriter(_rw, _rp);
    }
//convenient stream opperators
	//! Put a char on the output buffer
	Writer& operator << (char _c);
	//! Put a string on the output buffer
	/*!
		Should be use with great care. If you're not sure about the size
		of the string, consider using the putAtom callback.
	*/
	Writer& operator << (const char* _s);
	//! Put a string on the output buffer
	/*!
		Should be use with great care. If you're not sure about the size
		of the string, consider using the putAtom callback.
	*/
	Writer& operator << (const String &_str);
	//! Put a number on the output buffer
	Writer& operator << (uint32 _v);
	//! Put a number on the output buffer
	Writer& operator << (uint64 _v);
	//! Convenient method for flushing the buffer
	int flushAll();
protected:
	//NOTE: the buffer needs to be flushed before calling putRawString
	//! Convenient callback used internaly
	static int putRawString(Writer &_rw, Parameter &_rp);
	//! Convenient callback used internaly
	static int putStreamDone(Writer &_rw, Parameter &_rp);
	int flush();
	void clear();
	//! The writer will call this method when writing data
	virtual int write(char *_pb, uint32 _bl) = 0;
	//! The writer will call this method on manage callback
	virtual int doManage(int _mo);
	void doPrepareBuffer(char *_newbeg, const char *_newend);
protected:
	typedef Holder<Buffer>		BufferHolderT;
	enum States{
		RunState,
		ErrorState
	};
	enum {
		FlushLength = 1024,//!< If buffer data size is above this value, it will be written.
		StartLength = 1024 * 2, //!< The initial buffer capacity.
		MaxDoubleSizeLength = 4096 //!< the length up to which we can double the size of the buffer.
	};
	void resize(uint32 _sz);
	typedef std::pair<FncT, Parameter>	FncPairT;
	typedef Stack<FncPairT> 			FncStackT;
	Logger				*plog;
	BufferHolderT		bh;
	char				*rpos;
	char				*wpos;
	FncStackT         	fs;
	bool				dolog;
	short				state;
};

inline const Buffer& Writer::buffer()const{
	return *bh;
}


}//namespace text
}//namespace protocol
}//namespace solid

#endif
