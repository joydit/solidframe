/* Implementation file utility.cpp
	
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

#include "utility/workpool.hpp"
#include "utility/polycontainer.hpp"
#include "utility/common.hpp"

namespace solid{

const uint8 reverted_chars[] = {
	0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,0x10,0x90,
	0x50,0xD0,0x30,0xB0,0x70,0xF0,0x08,0x88,0x48,0xC8,
	0x28,0xA8,0x68,0xE8,0x18,0x98,0x58,0xD8,0x38,0xB8,
	0x78,0xF8,0x04,0x84,0x44,0xC4,0x24,0xA4,0x64,0xE4,
	0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,0x0C,0x8C,
	0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,0x1C,0x9C,0x5C,0xDC,
	0x3C,0xBC,0x7C,0xFC,0x02,0x82,0x42,0xC2,0x22,0xA2,
	0x62,0xE2,0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,
	0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,0x1A,0x9A,
	0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,0x06,0x86,0x46,0xC6,
	0x26,0xA6,0x66,0xE6,0x16,0x96,0x56,0xD6,0x36,0xB6,
	0x76,0xF6,0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,
	0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,0x01,0x81,
	0x41,0xC1,0x21,0xA1,0x61,0xE1,0x11,0x91,0x51,0xD1,
	0x31,0xB1,0x71,0xF1,0x09,0x89,0x49,0xC9,0x29,0xA9,
	0x69,0xE9,0x19,0x99,0x59,0xD9,0x39,0xB9,0x79,0xF9,
	0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,0x15,0x95,
	0x55,0xD5,0x35,0xB5,0x75,0xF5,0x0D,0x8D,0x4D,0xCD,
	0x2D,0xAD,0x6D,0xED,0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,
	0x7D,0xFD,0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,
	0x13,0x93,0x53,0xD3,0x33,0xB3,0x73,0xF3,0x0B,0x8B,
	0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,0x1B,0x9B,0x5B,0xDB,
	0x3B,0xBB,0x7B,0xFB,0x07,0x87,0x47,0xC7,0x27,0xA7,
	0x67,0xE7,0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
	0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,0x1F,0x9F,
	0x5F,0xDF,0x3F,0xBF,0x7F,0xFF
};

uint8 bit_count(const uint8 _v){
	static const uint8 cnts[] = {
		0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
		4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
	};
	return cnts[_v];
}

uint16 bit_count(const uint16 _v){
	return bit_count((uint8)(_v & 0xff)) + bit_count((uint8)(_v >> 8));
}

uint32 bit_count(const uint32 _v){
	return bit_count((uint8)(_v & 0xff)) +
		bit_count((uint8)((_v >> 8) & 0xff)) +
		bit_count((uint8)((_v >> 16) & 0xff)) +
		bit_count((uint8)((_v >> 24) & 0xff));
}

uint64 bit_count(const uint64 _v){
	return bit_count((uint32)(_v & 0xffffffff)) +
		bit_count((uint32)(_v >> 32));
}

inline uint8 compute_crc_value(uint8 _pos){
	if(_pos < (1 << 5)){
		return (bit_count(_pos) << 5) | _pos;
	}else{
		return 0xff;
	}
}

inline uint16 compute_crc_value(uint16 _pos){
	if(_pos < (1 << 12)){
		return (bit_count(_pos) << 12) | _pos;
	}else{
		return 0xffff;
	}
}

inline uint32 compute_crc_value(uint32 _pos){
	if(_pos < (1 << 27)){
		return (bit_count(_pos) << 27) | _pos;
	}else{
		return 0xffffffff;
	}
}

inline uint64 compute_crc_value(uint64 _pos){
	if(_pos < (1ULL << 58)){
		return (bit_count(_pos) << 58) | _pos;
	}else{
		return -1LL;
	}
}


/*static*/ CRCValue<uint8> CRCValue<uint8>::check_and_create(uint8 _v){
	CRCValue<uint8> crcv(_v, true);
	if(crcv.crc() == bit_count(crcv.value())){
		return crcv;
	}else{
		return CRCValue<uint8>(0xff, true);
	}
}
/*static*/ bool CRCValue<uint8>::check(uint8 _v){
	CRCValue<uint8> crcv(_v, true);
	return crcv.crc() == bit_count(crcv.value());
}
CRCValue<uint8>::CRCValue(uint8 _v):v(compute_crc_value(_v)){
}

/*static*/ CRCValue<uint16> CRCValue<uint16>::check_and_create(uint16 _v){
	CRCValue<uint16> crcv(_v, true);
	if(crcv.crc() == bit_count(crcv.value())){
		return crcv;
	}else{
		return CRCValue<uint16>(0xffff, true);
	}
}
/*static*/ bool CRCValue<uint16>::check(uint16 _v){
	CRCValue<uint16> crcv(_v, true);
	return crcv.crc() == bit_count(crcv.value());
}
CRCValue<uint16>::CRCValue(uint16 _v):v(compute_crc_value(_v)){
}


/*static*/ CRCValue<uint32> CRCValue<uint32>::check_and_create(uint32 _v){
	CRCValue<uint32> crcv(_v, true);
	if(crcv.crc() == bit_count(crcv.value())){
		return crcv;
	}else{
		return CRCValue<uint32>(0xffffffff, true);
	}
}
/*static*/ bool CRCValue<uint32>::check(uint32 _v){
	CRCValue<uint32> crcv(_v, true);
	return crcv.crc() == bit_count(crcv.value());
}

CRCValue<uint32>::CRCValue(uint32 _v):v(compute_crc_value(_v)){
}

/*static*/ CRCValue<uint64> CRCValue<uint64>::check_and_create(uint64 _v){
	CRCValue<uint64> crcv(_v, true);
	if(crcv.crc() == bit_count(crcv.value())){
		return crcv;
	}else{
		return CRCValue<uint64>(-1LL, true);
	}
}
/*static*/ bool CRCValue<uint64>::check(uint64 _v){
	CRCValue<uint64> crcv(_v, true);
	return crcv.crc() == bit_count(crcv.value());
}

CRCValue<uint64>::CRCValue(uint64 _v):v(compute_crc_value(_v)){
}
	
}//namespace solid
