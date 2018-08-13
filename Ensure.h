#pragma once

#include <exception>
#include <string>

#include "Trace.h"

struct DbgInfo
{
    const char *file;
    int line;
    const char *function;
};

inline
std::string toString(DbgInfo dbgInfo)
{
    std::string str;

    if(nullptr != dbgInfo.file)
    {
        str += dbgInfo.file;
        str += ':';
    }
    if(0 != dbgInfo.line)
    {
        str += std::to_string(dbgInfo.line);
        str += ' ';
    }

    if(nullptr != dbgInfo.file)
    {
        str += dbgInfo.function;
        str += ' ';
    }

    return str;
}

/* to filter out exceptions throws by ENSURE */
struct EnsureException
{
    DbgInfo dbgInfo;

    EnsureException(DbgInfo info):
        dbgInfo{info}
    {}

    std::string toString() const {return ::toString(dbgInfo);}
};

template <typename BaseException>
struct Exception: public EnsureException, public BaseException
{
    Exception(DbgInfo info, const char *msg):
        EnsureException{info},
        BaseException{EnsureException::toString() + ' ' + msg + '\n'}
    {}
};

#define TO_STRING_IMPL(x) #x
#define TO_STRING(x) TO_STRING_IMPL(x)

#define ENSURE(cond, ExceptionType) \
    do \
    { \
        if(!(cond)) \
        { \
            throw \
                ExceptionType \
                { \
                    DbgInfo{__FILE__, __LINE__, __PRETTY_FUNCTION__}, \
                    TO_STRING(ExceptionType) " : " TO_STRING(cond) \
                }; \
        } \
    } while(false)

#define ASSERT(cond) assert(cond)
