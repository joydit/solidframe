// forkreinit.cpp
//
// Copyright (c) 2012 Valentin Palade (vipalade @ gmail . com) 
//
// This file is part of SolidFrame framework.
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.
//
#include <iostream>
#include <stack>
#include <vector>
#include <deque>
#include <map>
#include <list>
#include "system/filedevice.hpp"
#include "utility/iostream.hpp"
#include "system/debug.hpp"
#include "system/thread.hpp"
#include "serialization/binary.hpp"
#include "serialization/idtypemapper.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>

using namespace std;
using namespace solid;
///\cond 0
class FileInputOutputStream: public InputOutputStream{
public:
	FileInputOutputStream();
	~FileInputOutputStream();
	int openRead(const char *_fn);
	int openWrite(const char *_fn);
	int read(char *, uint32, uint32 _flags = 0);
	int write(const char *, uint32, uint32 _flags = 0);
	int64 seek(int64, SeekRef);
	int64 size()const;
	void close();
private:
	FileDevice	fd;
};
///\endcond
FileInputOutputStream::FileInputOutputStream(){}
FileInputOutputStream::~FileInputOutputStream(){}
int FileInputOutputStream::openRead(const char *_fn){
	return fd.open(_fn, FileDevice::RO);
}
int FileInputOutputStream::openWrite(const char *_fn){
	return fd.open(_fn, FileDevice::WO | FileDevice::TR | FileDevice::CR);
}
int FileInputOutputStream::read(char *_pb, uint32 _bl, uint32 _flags){
	return fd.read(_pb, _bl);
}
int FileInputOutputStream::write(const char *_pb, uint32 _bl, uint32 _flags){
	return fd.write(_pb, _bl);
}
int64 FileInputOutputStream::seek(int64, SeekRef){
	return -1;
}
int64 FileInputOutputStream::size()const{
	return fd.size();
}
void FileInputOutputStream::close(){
	fd.close();
}

///\cond 0
struct Test{
	Test(const char *_fn = NULL);
	template <class S>
	void serialize(S &_s){
		_s.push(no, "Test::no");
		if(S::IsSerializer){
			InputStream *ps = &fs;
			_s.pushStream(ps, "Test::istream");
		}else{
			OutputStream *ps= &fs;
			_s.pushStream(ps, "Test::ostream");
		}
		_s.push(fn,"Test::fn");
	}
	void print();
private:
	int32 					no;
	string					fn;
	FileInputOutputStream	fs;
};
///\endcond

Test::Test(const char *_fn):fn(_fn?_fn:""){
	if(_fn){
		no = 11111;
		fs.openRead(_fn);
	}else{
		fs.openWrite("output.out");
	}
}
//-----------------------------------------------------------------------------------


void Test::print(){
	cout<<endl<<"Test:"<<endl;
	cout<<"no = "<<no<<endl;
	cout<<"fn = "<<fn<<endl<<endl;
}

///\cond 0
void parentRun(int _sd, const char *_fn);
void childRun(int _sd);


typedef serialization::binary::Serializer<void>			BinSerializer;
typedef serialization::binary::Deserializer<void>		BinDeserializer;
typedef serialization::IdTypeMapper<
	BinSerializer,
	BinDeserializer,
	uint32
>													TypeMapper;

static TypeMapper		tpmap;

///\endcond

int main(int argc, char *argv[]){
	int sps[2];
	if(argc != 2){
		cout<<"Expecting a file name as parameter!"<<endl;
		return 0;
	}

	int rv = socketpair(PF_UNIX, SOCK_STREAM, 0, sps);
	if(rv < 0){
		cout<<"error creating socketpair: "<<strerror(errno)<<endl;
		return 0;
	}

	rv = fork();
	if(rv){//the parent
		Thread::init();
#ifdef UDEBUG
		std::string dbgout;
		Debug::the().levelMask("view");
		Debug::the().moduleMask("ser_bin any");
		Debug::the().initFile(argv[0], false, 3, 1024 * 1024 * 64, &dbgout);
		cout<<"debug log to: "<<dbgout<<endl;
#endif
		childRun(sps[1]);
		//close(sps[1]);
	}else{//the child
		Thread::init();
#ifdef UDEBUG
		std::string dbgout;
		Debug::the().levelMask("view");
		Debug::the().moduleMask("ser_bin any");
		Debug::the().initFile(argv[0], false, 3, 1024 * 1024 * 64, &dbgout);
		cout<<"debug log to: "<<dbgout<<endl;
#endif

		parentRun(sps[0], argv[1]);
		//close(sps[0]);
	}
	return 0;
}
enum {BUFSZ = 4 * 1024};
void parentRun(int _sd, const char *_fn){
	char buf[BUFSZ];
	Test t(_fn);
	BinSerializer	ser(tpmap);
	ser.push(t, "test");
	t.print();
	int rv;
	int cnt = 0;
	while((rv = ser.run(buf, BUFSZ)) != 0){
		cnt += rv;
		if(write(_sd, buf, rv) != rv){
			cout<<"Error write: "<<strerror(errno)<<endl;
			return;
		}
	}
	cout<<"Parent finished, writted "<<cnt<<endl;
	sleep(1);
}
void childRun(int _sd){
	char buf[BUFSZ];
	Test t;
	BinDeserializer	des(tpmap);
	des.push(t, "test");
	int rv;
	cout<<"Client reading"<<endl;
	while((rv = read(_sd, buf, BUFSZ)) > 0){
		cout<<"Client read: "<<rv<<endl;
		if(des.run(buf, rv) != rv) break;
		if(des.empty()) break;
	}
	if(rv < 0){
		cout<<"Error read: "<<strerror(errno)<<endl;
		return;
	}
	t.print();
}

