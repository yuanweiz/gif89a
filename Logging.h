#ifndef __LOGGING_H
#define __LOGGING_H
#include <string>
#include <stdint.h>
#include "Buffer.h"
class LogStream {
public:
	LogStream(const char * file, int lineno, int logLevel);
    LogStream & operator << (float);
    LogStream & operator << (double);
    LogStream & operator << (int64_t);
    LogStream & operator << (uint64_t);
    LogStream & operator << (int32_t);
    LogStream & operator << (uint32_t);
    LogStream & operator << (int16_t);
    LogStream & operator << (uint16_t);
    ~LogStream();
	LogStream & operator << (const char* str) ;

	//LogStream & operator << (const void* ptr)
    //{
	//	printf("%x", reinterpret_cast<uintptr_t>(ptr));
	//	return *this;
	//}
	LogStream & operator << (char ch) {
		buffer_.append(ch);
		return *this;
	}
	LogStream & operator << (const std::string & s) {
		//printf("%s", s.c_str());
        buffer_.append(s.c_str(),s.size());
		return *this;
	}
	enum {LogTrace, LogDebug, LogVerbose};
	static void setLevel(int l) { level = l; }
	static int getLevel() { return level; }
private:
    template < class T> void printIntegral(T);
	static int level;
    Buffer buffer_;
};

#define LOG_DEBUG if (LogStream::getLevel() >= LogStream::LogDebug)\
	LogStream(__FILE__, __LINE__, LogStream::LogDebug)
#define LOG_VERBOSE if (LogStream::getLevel() >= LogStream::LogVerbose)\
	LogStream(__FILE__, __LINE__, LogStream::LogVerbose)
#define LOG_TRACE if (LogStream::getLevel() >= LogStream::LogTrace)\
	LogStream(__FILE__, __LINE__, LogStream::LogTrace)

#endif// __LOGGING_H
