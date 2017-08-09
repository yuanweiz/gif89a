#include "Exception.h"
#include <assert.h>
#ifdef _WIN32
#include <Windows.h>
#include <DbgHelp.h>
void backTrace(std::string & callstack_) {
	auto thisProcess = GetCurrentProcess();
	void * stack[200];
	auto nframes = ::CaptureStackBackTrace(0, sizeof(stack), stack, NULL);
	assert(nframes <= sizeof(stack));
	const int maxNameLen = 255;
	SymInitialize(thisProcess, NULL, TRUE);
	SYMBOL_INFO * info = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 1 + maxNameLen,1);
	info->MaxNameLen = maxNameLen;
	info->SizeOfStruct = sizeof(SYMBOL_INFO);
	for (size_t i = 0;i < nframes;++i) {
		//reuse the same SYMBOL_INFO struct
		SymFromAddr(thisProcess, (DWORD64)stack/*[nframes-1-i]*/[i], 0, info);
		callstack_.push_back('\n');
		callstack_.append(info->Name);
	}
	free(info);
}
#else // for linux
#include <execinfo.h>
void backTrace(std::string & callstack_)
{
	const int len = 200;
	void* buffer[len];
	int nptrs = ::backtrace(buffer, len);
	char** strings = ::backtrace_symbols(buffer, nptrs);
	if (strings)
	{
		for (int i = 0; i < nptrs; ++i)
		{
			// TODO demangle funcion name with abi::__cxa_demangle
			callstack_.push_back('\n');
			callstack_.append(strings[i]);
		}
		free(strings);
	}
}
#endif
Exception::Exception(const char * msg):callstack_(msg) {
	backTrace(callstack_);
}
Exception::Exception(const std::string & msg):callstack_(msg) {
	backTrace(callstack_);
}
const char * Exception::what()const noexcept{
	return callstack_.c_str();
}


LogicError::LogicError(const char*msg) :Exception(msg) {
}
LogicError::LogicError(const std::string&msg) :Exception(msg) {
}
const char*LogicError::what() const noexcept{
	return Exception::what();
}
