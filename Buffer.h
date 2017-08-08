#ifndef __BUFFER_H
#define __BUFFER_H
#include <sys/types.h>
class Buffer {
public:
	Buffer(); 
	void append(const char *,size_t);
	void append(char);
	void retrieve(size_t);
	void retrieveString(void *, size_t);
	char* data();
	void add(size_t);
	char* peek();
	size_t readable();
	size_t start_, end_;
//const int N = 65536;
private:
	char data_[65536];
};
#endif// __BUFFER_H
