// frame/aio/aiocompletion.hpp
//
// Copyright (c) 2014 Valentin Palade (vipalade @ gmail . com) 
//
// This file is part of SolidFrame framework.
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.
//
#ifndef SOLID_FRAME_AIO_COMPLETION_HPP
#define SOLID_FRAME_AIO_COMPLETION_HPP

#include "frame/aio/aiocommon.hpp"
#include "frame/aio/aioforwardcompletion.hpp"
#include "system/error.hpp"
#include "system/cassert.hpp"

namespace solid{
struct Device;
namespace frame{
namespace aio{

class Object;
class Reactor;
struct ObjectProxy;
struct ReactorContext;
struct ReactorEvent;

class CompletionHandler: public ForwardCompletionHandler{
	static void on_init_completion(CompletionHandler&, ReactorContext &);
protected:
	static CompletionHandler* completion_handler(ReactorContext &);
	typedef void (*CallbackT)(CompletionHandler&, ReactorContext &);
public:
	CompletionHandler(
		ObjectProxy const &_rop,
		CallbackT _pcall = &on_init_completion
	);
	
	~CompletionHandler();
	
	bool isActive()const{
		return  idxreactor != static_cast<size_t>(-1);
	}
	bool isRegistered()const{
		return pprev != nullptr;
	}
	bool activate(Object const &_robj);
	void deactivate();
	void unregister();
protected:
	CompletionHandler(CallbackT _pcall = &on_init_completion);
	
	void completionCallback(CallbackT _pcbk);
	ReactorEventsE reactorEvent(ReactorContext &_rctx)const;
	Reactor& reactor(ReactorContext &_rctx)const;
	void error(ReactorContext &_rctx, ERROR_NS::error_condition const& _err)const;
	void systemError(ReactorContext &_rctx, ERROR_NS::error_code const& _err)const;
	void addDevice(ReactorContext &_rctx, Device const &_rsd, const ReactorWaitRequestsE _req);
private:
	friend class Reactor;
	
	void handleCompletion(ReactorContext &_rctx){
		(*call)(*this, _rctx);
	}
private:
	friend class Object;
private:
	ForwardCompletionHandler		*pprev;
	size_t							idxreactor;//index within reactor
	CallbackT						call;
};

inline void CompletionHandler::completionCallback(CallbackT _pcbk){
	call = _pcbk;
}


}//namespace aio
}//namespace frame
}//namespace solid


#endif
