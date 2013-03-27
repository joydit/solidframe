/* Declarations file mutexpool.hpp
	
	Copyright 2007, 2008 Valentin Palade 
	vipalade@gmail.com

	This file is part of SolidFrame framework.

	SolidFrame is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	SolidFrame is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with SolidFrame.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MUTEXPOOLPP_HPP
#define MUTEXPOOLPP_HPP

#include <iostream>
#include "system/mutex.hpp"

namespace solid{

template <ushort V>
class FastMutexPool{
        enum {sz = (V > 10)?1023:((V<2)?(2):((1<<V)-1))};
public:
        FastMutexPool(){
                //std::cout<<"sz = "<<sz<<std::endl;
                for(ulong i=0;i<(sz+1);++i) pool[i] = NULL;
        }
        ~FastMutexPool(){
                Locker<Mutex> lock(mutex);
                for(ulong i=0;i<(sz+1);++i) delete pool[i];
        }
        Mutex *getp(const void *_ptr){
                ulong pos = ((ulong)_ptr) & sz;
                Mutex *m;
                mutex.lock();
                if((m = pool[pos]) == NULL){
                        m = pool[pos] = new Mutex;
                }
                mutex.unlock();
                return m;
        }
        Mutex &getr(const void *_ptr){
                ulong pos = ((ulong)_ptr) & sz;
                Mutex *m;
                mutex.lock();
                if((m = pool[pos]) == NULL){
                        m = pool[pos] = new Mutex;
                }
                mutex.unlock();
                return *m;
        }
private:
        Mutex mutex;
        Mutex *pool[sz+1];
};

}//namespace solid

#endif

