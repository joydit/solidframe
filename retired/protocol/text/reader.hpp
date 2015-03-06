// protocol/text/reader.hpp
//
// Copyright (c) 2007, 2008 Valentin Palade (vipalade @ gmail . com) 
//
// This file is part of SolidFrame framework.
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.
//
#ifndef SOLID_PROTOCOL_TEXT_READER_HPP
#define SOLID_PROTOCOL_TEXT_READER_HPP

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

class Reader;
//! A dummy key for usage with Reader::fetchKey
/*!
	Use DummyKey::pointer() on the creator function called by Reader::fetchKey,
	when the mathed string has no coresponding object.E.g. a flag list where the
	flags are given by name.
*/
struct DummyKey{
	//! Returns a pointer to a static DummyKey object.
	static DummyKey* pointer();
	void initReader(const Reader &);
};
//! A nonblocking buffer oriented (not line oriented) protocol parser
/*!
	Here are some characteristics of the reader:<br>
	- it is designed for nonblocking/asynchrounous usage<br>
	- it is buffer oriented not line oriented<br>
	- has suport for recovering from parsing errors<br>
	- it is flexible and easely extensible<br>
	
	<b>Overview:</b><br>
		Internally it uses a stack of pairs of a function pointer and a parameter 
		(see protocol::Parameter) whith wich the function will be called.
		<br>
		Every function can in turn push new calls in the stack.
		<br>
		There is a very simple and fast state machine based on the return value
		of scheduled functions. The state machine will exit when, either the buffer is empty
		and it cannot be filled (wait to be filled asynchrounously), the stack is empty,
		a function return Reader::Failure.<br>
		
	
	<b>Usage:</b><br>
		Inherit, implement the virtual methods and extend the reader with new 
		parsing functions.<br>
		In your protocol (connection) execute loop:<br>
		> push some parsing callbacks and reset the parser state<br>
		> (repeatedly) call run until BAD or OK is returned<br>
		<br>
		- Failure usually means that the connection was/must be closed<br>
		- Success means that the stack is empty - it doesnt mean the data was
		parsed successfully - an error might have occurred and the parser has successfully recovered 
		(use isError)<br>
		
	<b>Notes:</b><br>
		- You can safely use pointers to existing parameters within the stack.<br>
		- As an example see test::alpha::Writer (test/foundation/alpha/src/alpha.(h/cpp)).<br>
*/
class Reader{
public:
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
		ClearTmpString,
	};
	enum BasicErrorsE{
		Unexpected,
		StringTooLong,
		EmptyAtom,
		NotANumber,
		LastBasicError,
		IOError
	};
	typedef int (*FncT)(Reader&, Parameter &_rp);
public:
	//! Constructor
	/*!
		It can get a pointer to a logger object which it will
		use to log protocol/filtered communication level data.
		\param _plog A pointer to a logger object or NULL
	*/
	Reader(Logger *_plog = NULL);
	
	//!virtual destructor
	virtual ~Reader();
	
	//! Sets the internal buffer
	template <class B>
	void buffer(const B &_b){
		doPrepareBuffer(_b.pbeg, _b.pend);
		bh = _b;
	}
	
	//! Gets the internal buffer
	const Buffer& buffer()const;
	
	//! Sheduller push method
	/*!
		\param _pf A pointer to a function
		\param _rp A const reference to a parameter
	*/
	void push(FncT _pf, const Parameter & _rp/* = Parameter()*/);
	void pop();
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
	//!Check if the stack is empty
	bool empty()const{return fs.empty();}
	//! The state machine algorithm
	int run();
	//! Check if the reader tries to recover from parsing error
	bool isRecovering()const;
	//! Check if there was a parsing error
	bool isError()const;
	//! Check if the log is active
	bool isLogActive()const{ return plog != NULL;}
	//! Reset the reader state - prepare for new parsing.
	void resetState();
//reading static functions
	//!Callback for checking if the current char from input is a certain one
	static int checkChar(Reader &_rr, Parameter &_rp);
	//!Callback for fetching dummy literals - used in case of error recovering
	static int fetchLiteralDummy(Reader &_rr, Parameter &_rp);
	//!Callback fetching a literal string
	static int fetchLiteralString(Reader &_rr, Parameter &_rp);
	
	//!Callback fetching a literal string
	static int fetchLiteralStream(Reader &_rr, Parameter &_rp);
	//!Callback for refilling the input buffers
	static int refill(Reader &_rr, Parameter &_rp);
	static int refillDone(Reader &_rr, Parameter &_rp);
	//!Callback for popping certain callbacks from the stack
	static int pop(Reader &_rr, Parameter &_rp);
	//!Callback for returning a certain value
	template <bool B>
	static int returnValue(Reader &_rr, Parameter &_rp);
	//!Callback for reader management - set reset certain data
	static int manage(Reader &_rr, Parameter &_rp);
	//!Callback for fetching an unsigned number
	static int fetchUInt32(Reader &_rr, Parameter &_rp);
	//!Callback for fetching an unsigned number
	static int fetchUInt64(Reader &_rr, Parameter &_rp);
	//!Callback for dropping the current char
	static int dropChar(Reader &_rr, Parameter &_rp);
	//!Callback for saving the current char
	static int saveCurrentChar(Reader &_rr, Parameter &_rp);
	//!Callback for saving the current char
	static int fetchChar(Reader &_rr, Parameter &_rp);
	//! Callback - external reading plugin
	/*! You'd better have one in the most derived reader too*/
	template <class C>
	static int reinit(Reader &_rr, Parameter &_rp){
		return reinterpret_cast<C*>(_rp.a.p)->reinitReader(_rr, _rp);
	}
    //! Callback for fetching a key or a string
    /*!
    	Use this callback to fetch keys which are objects which
    	can in their turn push new parsing callbacks.
    */
	template <class Rdr, class Creator, class Filter, class Key/* = DummyKey*/>
	static int fetchKey(Reader &_rr, Parameter &_rp){
		Rdr	&rr(static_cast<Rdr&>(_rr));
		int rv = rr.template fetch<Filter>(_rr.tmp, _rp.b.u32);
		switch(rv){
			case Success:break;
			case Wait:
				_rr.push(&Reader::refill);
				return Continue;
			case Error:
				_rr.basicError(StringTooLong);
				return Error;
		}
		Key* pk = reinterpret_cast<Creator*>(_rp.a.p)->create(_rr.tmp, rr);
		if(pk){
			_rr.fs.pop();
			_rr.tmp.clear();
			pk->initReader(rr);
			return Continue;
		}else{
			_rr.keyError(_rr.tmp);
			_rr.tmp.clear();
			return Error;
		}
	}
	//! Callback for fetching a filtered string
	template <class Filter>
	static int fetchFilteredString(Reader &_rr, Parameter &_rp){
		int rv = _rr.fetch<Filter>(*static_cast<String*>(_rp.a.p), _rp.b.u32);
		switch(rv){
			case Success:
				if(static_cast<String*>(_rp.a.p)->empty()){
					_rr.basicError(EmptyAtom);
					return Error;
				}
				break;
			case Wait:
				_rr.push(&Reader::refill);
				return Continue;
			case Error:
				_rr.basicError(StringTooLong);
				return Error;
		}
		return Success;
	}
	//! Callback which check the current char using the filter and if it passes it pops callbacks
	
	template <class Filter>
	static int checkIfCharThenPop(Reader &_rr, Parameter &_rp){
		int c;
		if(_rr.peek(c)){
			_rr.push(&Reader::refill);
			return Continue;
		}
		if(Filter::check(c)){
			return pop(_rr, _rp);
		}
		return Success;
	}
	
protected:
	enum States{
		RunState,
		RecoverState,
		ErrorState,
		FatalErrorState,
		IOErrorState
	};
protected:
	static int fetchLiteralStreamContinue(Reader &_rr, Parameter &_rp);
	//! Tries to peek the current char
	int peek(int &_c);
	//! Get the current char = peek + drop
	int get(int &_c);
	//! Move forward in the input buffer
	void drop();
	//! Convenient method for fetching a literal string
	int fetchLiteral(String &_rds, uint32 &_rdsz);
	//! Convenient method for fetching a filtered string
	template<class Filter>
	int fetch(String &_rds, uint32 _maxsz){
		const char *tbeg = rpos;
		while(rpos != wpos && Filter::check(*(rpos))) ++rpos;
		
		const uint32 sz(_rds.size() + (rpos - tbeg));
		
		if(sz <= _maxsz){
			_rds.append(tbeg, rpos - tbeg);
			if(rpos != wpos){
				if(dolog) plog->inLiteral(_rds.data(),_rds.size());
				return Success;
			}
		}else{
			if(dolog) plog->inLiteral(_rds.data(), _rds.size());
			return Error;
		}
		return Wait;
	}
	//! Locate a certain char (using filter) keeping in _rds the last _keep chars
	template <class Filter>
	int locate(String &_rds, uint32 &_rdsz, int _keep){
		//int rcode;
		if(_rdsz < (ulong)(bh->pend - bh->pbeg)) return Failure;
		const char *tbeg = rpos;
		cassert(rpos <= wpos);
		while(rpos != wpos && !Filter::check(*rpos)) ++rpos;
		int slen = rpos - tbeg;
		if(slen < _keep){
			if(_rds.size() >= (unsigned)_keep){
				_rds.erase(0, _rds.size() - _keep + slen);
			}
			_rds.append(tbeg, slen);
		}else{
			_rds.assign(rpos - _keep, _keep);
		}
		_rdsz -= slen;
		if(rpos != wpos){
			if(dolog) plog->inLocate(_rds.data(), _rds.size());
			return Success;
		}
		return Wait;
	}
	//! The reader will call this method when refilling its buffer
	virtual int read(char *_pb, uint32 _bl) = 0;
	//! Used for asynchrounous reading - must return the lenght or read data
	virtual int readSize()const = 0;
	//! The reader will call this method on manage callback
	virtual int doManage(int _mo);
	//! The reader will call this method when preparing for error recovery
	virtual void prepareErrorRecovery();
	//! The reader will call this method when a char error occured (unexpected char .. etc)
	virtual void charError(char _popc, char _expc) = 0;
	//! The reader will call this method when a key error occured (unexpected name .. etc)
	virtual void keyError(const String &_pops, int _id = Unexpected) = 0;
	//! The reader will call this method when other basic error occured
	virtual void basicError(int _id) = 0;
	void doPrepareBuffer(char *_newbeg, const char *_newend);
protected:
	typedef Holder<Buffer>		BufferHolderT;
	typedef std::pair<FncT, Parameter>	FncPairT;
	Logger				*plog;
	BufferHolderT		bh;
	char				*rpos;//write buff pos
	char				*wpos;//read buff pos
	short				state;
	bool				dolog;
	Stack<FncPairT>	fs;
	String				tmp;
};

inline const Buffer& Reader::buffer()const{
	return *bh;
}

}//namespace text
}//namespace protocol
}//namespace solid

#endif
