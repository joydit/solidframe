// protocol/text/logger.hpp
//
// Copyright (c) 2007, 2008 Valentin Palade (vipalade @ gmail . com) 
//
// This file is part of SolidFrame framework.
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.
//
#ifndef SOLID_PROTOCOL_TEXT_LOGGER_HPP
#define SOLID_PROTOCOL_TEXT_LOGGER_HPP

#include "system/common.hpp"
#include <string>

namespace solid{
namespace protocol{
namespace text{

//! A class for protocol level logging.
/*!
	Use it with a protocol::Reader and a protocol::Writer.
	It compacts the protocol communication and ease it for reading
	(e.g. some unprintable characters are replaced with some suggestive text).
	It is not a final class, you should inherit and implement doInFlush and
	doOutFlush.
	There are two sets of functions: "in" for input - incomming protocol data
	(the reader) and "out" for outgoing protocol data (the writer).
*/
class Logger{
public:
	//! Constructor with an estimated line size
	/*!
		Lines bigger then _linesz will be split on 
		multiple lines
	*/
	Logger(uint32 _linesz = 1024);
	//! Virtual destructor
	virtual ~Logger();
	//!Flushes the incomming log data
	void inFlush();
	//! Logs a char
	void inChar(int _c);
	//! Logs an atom 
	/*! No check for unprintable chars is done
	*/
	void inAtom(const char *_pb, unsigned _bl);
	//! Logs a literal
	/*! Check for unprintable chars is done
	*/
	void inLiteral(const char *_pb, unsigned _bl);
	//! Logs a Reader locate data
	/*!
		Locate data appears when the reader recovers from an error, by skipping a line
		Not the entire line is logged, but it's first part and '...'.
		inLocate will call inLiteral and append '...' to log line.
	*/
	void inLocate(const char *_pb, unsigned _bl);
	
	//! Flushes the outgoing log data
	void outFlush();
	//! Logs one char
	void outChar(int _c1);
	//! Logs two chars
	void outChar(int _c1, int _c2);
	//! Logs three chars
	void outChar(int _c1, int _c2, int _c3);
	//! Logs four chars
	void outChar(int _c1, int _c2, int _c3, int _c4);
	//! Logs an atom
	void outAtom(const char *_pb, unsigned _bl);
	//! Logs a literal
	void outLiteral(const char *_pb, unsigned _bl);
protected:
	//! Flushes the input log data
	/*!
		Implement this to actually protocol log data
	*/
	virtual void doInFlush(const char *_pd, unsigned _dl) = 0;
	//! Flushes the input log data
	/*!
		Implement this to actually protocol log data
	*/
	virtual void doOutFlush(const char *_pd, unsigned _dl) = 0;
private:
	std::string 	ins;
	std::string		outs;
	const uint32	linesz;
};


}//namespace text
}//namespace protocol
}//namespace solid

#endif

