#include "StreamReader.h"
#include "Exception.h"
#include <string.h>
#include <assert.h>

template<class T>
T StreamReader::peekInt()
{
	T t;
	::memcpy(&t, buffer_.peek(), sizeof(T));
	return t;
}

int8_t StreamReader::readInt8() {
	int8_t i = peekInt8();
	buffer_.retrieve(sizeof(int8_t));
	return i;
}

int8_t StreamReader::peekInt8() {
	return peekInt<int8_t>();
}

int16_t StreamReader::readInt16() {
	int16_t i = peekInt16();
	buffer_.retrieve(sizeof(int16_t));
	return i;
}

int16_t StreamReader::peekInt16() {
	return peekInt<int16_t>();
}

int32_t StreamReader::readInt32() {
	int32_t i = peekInt32();
	buffer_.retrieve(sizeof(int32_t));
	return i;
}

int32_t StreamReader::peekInt32() {
	return peekInt<int32_t>();
}


StreamReader::StreamReader(const char* fname)
	:bits_(8)
{
    file_ = ::fopen(fname, "r");
    if (file_==nullptr){
        throw Exception("error opening file");
    }
    ::fseek(file_, 0, SEEK_END);
    readable_ = ftell(file_);
    ::fseek(file_, 0, SEEK_SET);
}
StreamReader::~StreamReader() {
    if (file_)
        ::fclose(file_);
}

uint32_t StreamReader::readBits(uint32_t bits) {
	assert(bits_ <= 8 && bits_ > 0);
	if (bits < bits_) {
		bits_ -= bits;
		uint8_t c_ = peekInt8(); //boost to uint32_t
		uint32_t c = c_; 
        assert ((c & 0xffffff00) == 0);
        c>>=(8-bits);
        //take least significant bits;
		return c & ((1<<bits)-1);
	}
	else {
        uint32_t tot_bits=bits_;
		uint32_t ret = 0;
		uint8_t c_ = readInt8();
        uint32_t c = c_ ; //boost to uint32_t
		ret = (c >> (8-bits_));
		bits -= bits_;
		while (bits>=8) {
			bits -= 8;
            c_ = readInt8();
            c = c_;
			ret += (c << tot_bits);
            tot_bits+=8;
		}
		bits_ = 8 - bits; 
		c_ = peekInt8();
        c = c_;
		//ret = (ret << bits) + (c >> bits_);
        uint32_t leastNbits=((1<<bits)-1);
		ret += ( (c & leastNbits) << tot_bits);
		return ret;
	}
}
void StreamReader::retrieveString(void * dst, size_t sz) {
	buffer_.retrieveString(dst, sz);
}

void StreamReader::readFile(size_t sz) {
	char * buf = new char[sz];
	auto nRead = fread(buf,1, sz, file_);
	buffer_.append(buf, nRead);
	delete[] buf;
}

size_t StreamReader::readable() {
	return buffer_.readable();
}
uint32_t StreamReader::remainingBits() {
	return bits_;
}
