/* Declarations file filekey.hpp
	
	Copyright 2010 Valentin Palade 
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

#ifndef SOLID_FRAME_FILE_FILE_KEY_HPP
#define SOLID_FRAME_FILE_FILE_KEY_HPP

#include "system/common.hpp"
#include <string>

namespace frame{
namespace file{
//! A key for requsting a file from file::Manager
struct Key{
	virtual ~Key();
	virtual uint32 mapperId()const = 0;
	virtual Key* clone() const = 0;
	//! If returns true the file key will be deleted
	virtual bool release()const;
	virtual const char* path() const;
	virtual uint64 capacity()const;
};

}//namespace file
}//namepsace foundation
#endif