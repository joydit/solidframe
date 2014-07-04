// frame/reactorbase.hpp
//
// Copyright (c) 2014 Valentin Palade (vipalade @ gmail . com) 
//
// This file is part of SolidFrame framework.
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.
//
#ifndef SOLID_FRAME_REACTOR_BASE_HPP
#define SOLID_FRAME_REACTOR_BASE_HPP

#include <deque>
#include <vector>
#include "frame/objectbase.hpp"

namespace solid{
namespace frame{

class Manager;
class SchedulerBase;

typedef std::vector<UidT>			UidVectorT;

//! The base for every selector
/*!
 * The manager will call raise when an object needs processor
 * time, e.g. because of an event.
 */
class ReactorBase{
public:
	virtual bool raise(UidT const& _robjuid, Event const& _re) = 0;
	virtual void stop() = 0;
	virtual void update() = 0;
	
protected:
	bool setObjectThread(ObjectBase &_robj, const UidT &_uid);
	void stopObject(ObjectBase &_robj);
	SchedulerBase& scheduler(){
		static SchedulerBase	*ps;
		return *ps;
	}
	bool prepareThread();
	void unprepareThread();
private:
	friend	class Manager;
	friend	class SchedulerBase;
	void idInManager(size_t _id);
	void idInScheduler(size_t _id);
	IndexT const& idInManager()const;
	size_t idInScheduler()const;
private:
	IndexT			mgridx;//
	size_t			schidx;
	UidVectorT		freeuidvec;
};


inline void ReactorBase::stopObject(ObjectBase &_robj){
	_robj.stop();
}
inline void ReactorBase::idInManager(size_t _id){
	mgridx = _id;
}
inline void ReactorBase::idInScheduler(size_t _id){
	schidx = _id;
}

inline IndexT const& ReactorBase::idInManager()const{
	return mgridx;
}
inline size_t ReactorBase::idInScheduler()const{
	return schidx;
}

}//namespace frame
}//namespace solid

#endif

