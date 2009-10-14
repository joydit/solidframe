/* Declarations file istream.hpp
	
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

#ifndef ISTREAMPP_HPP
#define ISTREAMPP_HPP

#include "stream.hpp"
#include "common.hpp"
//! A stream for reading 
class IStream: virtual public Stream{
public:
	virtual ~IStream();
	virtual int read(char *, uint32, uint32 _flags = 0) = 0;
	bool readAll(char *, uint32, uint32 _flags = 0);
	bool iok()const;
	bool ieof()const;
	bool ibad()const;
	bool ifail()const;
};

//! An IStreamIterator - an offset within the stream: a pointer to an istream
struct IStreamIterator{
	IStreamIterator(IStream *_ps = NULL, int64 _off = 0);
	void reinit(IStream *_ps = NULL, int64 _off = 0);
	int64 start();
	IStream* operator->() const{return ps;}
	IStream& operator*() {return *ps;}
	IStream		*ps;
	int64		off;
};

#ifdef UINLINES
#include "src/istream.ipp"
#endif

#endif