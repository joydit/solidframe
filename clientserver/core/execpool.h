/* Declarations file execpool.h
	
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

#ifndef CS_EXECPOOL_H
#define CS_EXECPOOL_H

#include "utility/workpool.h"

#include "activeset.h"
#include "objptr.h"

namespace clientserver{

class ExecPool: public WorkPool<ObjPtr<Object> >, public ActiveSet{
public:
	ExecPool(){}
	virtual ~ExecPool(){}
	void raise(uint _thid){}
	void raise(uint _thid, ulong _objid){}
	void poolid(uint _pid){}
protected:
	typedef WorkPool<ObjPtr<Object> > WorkPoolTp;
	virtual void run();
	virtual int createWorkers(uint) = 0;
};


}//namespace

#endif
