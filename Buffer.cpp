#include <string.h>
#include <assert.h>
#include "Buffer.h"
Buffer::Buffer() :start_(0), end_(0) {
}
char * Buffer::data() {
	return data_;
}

char * Buffer::peek() {
	return data_ + start_;
}
void Buffer::retrieve(size_t sz) {
	start_ += sz;
}
size_t Buffer::readable() {
	assert(end_ >= start_);
	return end_ - start_;
}

void Buffer::retrieveString(void *dst, size_t sz) {
	::memcpy(dst, peek(), sz);
	retrieve(sz);
}

void Buffer::append(const char*src, size_t sz) {
	::memcpy(data_+end_, src, sz);
	end_ += sz;
}
void Buffer::append(char c){
    data_[end_++] = c;
}
