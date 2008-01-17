#include "clientserver/filekeys.h"
#include "clientserver/filemapper.h"
#include "clientserver/filemanager.h"
#include <map>

using namespace std;

namespace clientserver{
//---------------------------------------------------------------
struct LessStrCmp{
	inline bool operator()(const char* const& _s1, const char* const& _s2)const{
		return strcmp(_s1, _s2) < 0;
	}
};

class NameFileMapper: public FileMapper{
public:
	NameFileMapper(const char *_prefix){
		if(_prefix && _prefix[0]){
			pfx = _prefix;
			if(pfx[pfx.size() - 1] != '\\'){
				pfx += '\\';
			}
		}
	}
	int find(const char *_fname)const;
	void insert(const char *_fname, int _val);
	void erase(const char *_fname);
	const string & prefix()const {return pfx;}
private:
	typedef std::map<const char*, uint32, LessStrCmp> NameMapTp;
	NameMapTp nm;
	string pfx;
};

class TempFileMapper: public FileMapper{
public:
	TempFileMapper(const char *_prefix){
		if(_prefix && _prefix[0]){
			pfx = _prefix;
			if(pfx[pfx.size() - 1] != '\\'){
				pfx += '\\';
			}
		}
	}
	void createFileName(string &_fname, unsigned _fileid);
private:
	void initFolders();
	string pfx;
};
//---------------------------------------------------------------
bool FileKey::release()const{
	return true;
}
//---------------------------------------------------------------
/*static*/ void NameFileKey::registerMapper(FileManager &_fm, const char *_prefix){
	_fm.mapper(new NameFileMapper(_prefix));
}
NameFileKey::NameFileKey(const char *_fname):name(_fname){}

NameFileKey::NameFileKey(const std::string &_fname):name(_fname){}

void NameFileKey::fileName(FileManager &_fm, int _fileid, string &_fname)const{
	const NameFileMapper *pm(_fm.mapper<NameFileMapper>());
	assert(pm);
	_fname = pm->prefix();
	_fname.append(name);
}
int NameFileKey::find(FileManager &_fm)const{
	const NameFileMapper *pm(_fm.mapper<NameFileMapper>());
	assert(pm);
	return pm->find(name.c_str());
	
}

void NameFileKey::insert(FileManager &_fm, int _fileid)const{
	NameFileMapper *pm(_fm.mapper<NameFileMapper>());
	assert(pm);
	pm->insert(name.c_str(), _val);
}

void NameFileKey::erase(FileManager &_fm, int _fileid)const{
	NameFileMapper *pm(_fm.mapper<NameFileMapper>());
	assert(pm);
	pm->erase(name.c_str());
}

FileKey* NameFileKey::clone()const{
	return new NameFileKey(*this);
}
//---------------------------------------------------------------
void FastNameFileKey::fileName(const FileManager &, int _fileid, string &)const{
	assert(false);
}

int FastNameFileKey::find(FileManager &_fm)const{
	const NameFileMapper *pm(_fm.mapper<NameFileMapper>());
	assert(pm);
	return pm->find(name);
}

void FastNameFileKey::insert(FileManager &_fm, int _fileid)const{
	assert(false);
	return -1;
}

void FastNameFileKey::erase(FileManager &_fm, int _fileid)const{
	assert(false);
	return -1;
}

FileKey* FastNameFileKey::clone()const{
	return new NameFileKey(name);
}
//---------------------------------------------------------------
int NameFileMapper::find(const char *_fname)const{
	NameMapTp::const_iterator it(nm.find(_fname));
	if(it != nm.end()) return it->second;
	return -1;
}
void NameFileMapper::insert(const char *_fname, int _fileid){
	pair<NameMapTp::iterator,bool> rv(nm.insert(NameMapTp::value_type(_fname, _fileid)));
	assert(rv.second);
}
void NameFileMapper::erase(const char *_fname){
	nm.erase(_fname);
}

//---------------------------------------------------------------
/*static */void TempFileKey::registerMapper(FileManager &, const char *_prefix){
	_fm.mapper(new TempFileMapper(_prefix));
}
void TempFileKey::fileName(FileManager &_fm, int _fileid, string &_fname)const{
	TempFileMapper *pm(_fm.mapper<TempFileMapper>());
	pm->createFileName(_fname, _fileid);
}
int TempFileKey::find(FileManager &)const{
	return -1;
}
void TempFileKey::insert(FileManager &_fm, int _fileid)const{
/*	TempFileMapper *pm(_fm.mapper<TempFileMapper>());
	assert(pm);*/
}
void TempFileKey::erase(FileManager &_fm, int _fileid)const{
	TempFileMapper *pm(_fm.mapper<TempFileMapper>());
	assert(pm);
	
}

static TempFileKey tempk;

bool TempFileKey::release()const{
	return false;
}

FileKey* TempFileKey::clone()const{
	return &tempk;
}
//---------------------------------------------------------------
TempFileMapper::TempFileMapper(const char *_prefix){
	if(_prefix && _prefix[0]){
		pfx = _prefix;
		if(pfx.size() && pfx[pfx.size() - 1] != '\\'){
			pfx += '\\';
		}
	}
	initFolders();
}
void TempFileMapper::createFileName(string &_fname, unsigned _fileid){
	char name[32];
	unsigned int fldid = _fileid & 0xff;
	unsigned int filid = _fileid >> 8;
	sptrintf(name, "%X/%x", fldid, filid);
	_fname = pfx;
	_fname.append(name);
}
void TempFileMapper::initFolders(){
	string path(pfx);
	char name[32];
	for(unsigned i(0); i <= 0xff; ++i){
		path.resize(pfx.size());
		sptrintf(name, "%X", fldid, filid);
		path.append(name);
		//TODO: createFolder(path);
	}
}

}//namespace clientserver

