#ifndef __EXCEPTION_H_
#define __EXCEPTION_H_
#include <stdexcept>
#include <string>
class Exception : public std::exception {
public:
	explicit Exception(const char*);
	explicit Exception(const std::string& );
	virtual const char* what() const noexcept;
private:
	std::string callstack_;
};

class LogicError : private Exception {
public:
	explicit LogicError(const char*);
	explicit LogicError(const std::string&);
	virtual const char* what() const noexcept;
};
#endif// __EXCEPTION_H
