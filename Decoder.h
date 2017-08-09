#ifndef _DECODER_H
#define _DECODER_H
#include <memory>
#include "Exception.h"

class ParseError: public Exception{
    public:
    explicit ParseError(const char* str)
        :Exception(str)
    {
    }
};

class Decoder{
public:
    explicit Decoder(const char* fname);
    ~Decoder();
    void decode();
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
#endif// _DECODER_H
