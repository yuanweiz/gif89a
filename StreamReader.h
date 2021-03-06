#ifndef __STREAMREADER_H
#define __STREAMREADER_H

#include <stdint.h>
#include <cstdio>
#include "Buffer.h"

//template<int N>

class StringView {
public:
	int size();
	char* data();
private:
	char* data_;
	int len_;
};

class StreamReader {
public:
	explicit StreamReader(const char *);
	//StreamReader(FILE*);
	int32_t peekInt32();
	int16_t peekInt16();
	int8_t peekInt8();
	int32_t readInt32();
	int16_t readInt16();
	int8_t readInt8();
	void retrieveString(void *, size_t);
	void retrieve(size_t);
	size_t requireBytes(size_t);
    uint32_t remainingBits();
	uint32_t readBits(uint32_t);
	~StreamReader();
private:
	uint32_t bits_;
	template <class T> T peekInt();
	FILE* file_;
    size_t fileSz;
    size_t readable_;
	Buffer buffer_;
};

#endif// __STREAMREADER_H
