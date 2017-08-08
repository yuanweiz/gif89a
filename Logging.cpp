#include "Logging.h"

#include <string.h>
#include <algorithm>
#include <type_traits>

const char* logLevelStr[]{
	"[TRACE ]",
	"[DEBUG ]",
	"[VERB  ]",
};
LogStream::LogStream(const char * file, int lineno, int logLevel)
{
	*this << logLevelStr[logLevel] << 
#ifdef _WIN32
        ::strrchr(file,'\\')+1 
#else 
        __builtin_strrchr(file, '/')+1
#endif
        << ":" << lineno << ":" ;
}
static char digit[] = "0123456789";
template <class T>
void LogStream::printIntegral(T v){
    static_assert(std::is_integral<T>::value,"Expecting integral type");
    T i = v;
    //if (i<0) {
    //    buffer_.append('-');
    //    i=-i;
    //    printIntegral<T>(i);
    //}
    char buf[32], *p=buf;
    size_t len=0;
    while (i!=0){
        ++len;
        *p++=digit[i%10];
        i/=10;
    }
    if (v <0){
        ++len;
        *p='-';
    }
    std::reverse(buf,buf+len);
    buffer_.append(buf,len);
}

LogStream & LogStream::operator<< (float f){
    return this->operator<<(static_cast<double>(f));
}

LogStream & LogStream::operator<< (double f){
    char buf[32];
    int len =::snprintf(buf,32,"%f",f);
    buffer_.append(buf,len);
    return *this;
}

LogStream & LogStream::operator << (int64_t i)
{
    printIntegral(i);
    return *this;
}
LogStream & LogStream::operator << (uint64_t i)
{
    printIntegral(i);
    return *this;
}
LogStream & LogStream::operator << (int32_t i)
{
    printIntegral(i);
    return *this;
}
LogStream & LogStream::operator << (uint32_t i)
{
    printIntegral(i);
    return *this;
}
LogStream & LogStream::operator << (int16_t i)
{
    printIntegral(i);
    return *this;
}
LogStream & LogStream::operator << (uint16_t i)
{
    printIntegral(i);
    return *this;
}

LogStream & LogStream::operator << (const char* str) {
    buffer_.append(str,strlen(str));
    return *this;
}

LogStream::~LogStream(){
    buffer_.append('\n');
    buffer_.append('\0');
    ::printf("%s",buffer_.peek());
    //::snprintf(buffer_.peek(), buffer_.readable(),"%s");
}
int LogStream::level = LogStream::LogVerbose;

