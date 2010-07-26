/* Declarations file execpool.hpp
	
	Copyright 2007, 2008 Valentin Palade 
	vipalade@gmail.com

	This file is part of SolidGround framework.

	SolidGround is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	SolidGround is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with SolidGround.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CS_EXECPOOL_HPP
#define CS_EXECPOOL_HPP

#include "utility/workpool.hpp"

#include "activeset.hpp"
#include "objectpointer.hpp"

namespace foundation{
//! A simple execution pool for one shot object execution
/*!
	It doesn't support object signaling and timeouts.
*/
class ExecPool: public WorkPool<ObjectPointer<Object> >, public ActiveSet{
public:
	ExecPool(uint32 _maxthrcnt);
	virtual ~ExecPool();
	void raise(uint _thid);
	void raise(uint _thid, uint _objid);
	void poolid(uint _pid);
	void run(Worker &_rw);
	void prepareWorker();
	void unprepareWorker();
protected:
	typedef WorkPool<ObjectPointer<Object> > WorkPoolT;
	/*virtual*/ int createWorkers(uint);
};


}//namespace

#endif