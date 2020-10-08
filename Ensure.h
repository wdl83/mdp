#pragma once

#include <cstring>
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
    const char *invariant;

    EnsureException(DbgInfo info, const char *msg):
        dbgInfo{info},
        invariant{msg}
    {}

    std::string toString() const {return ::toString(dbgInfo);}
};

template <typename BaseException, int ID>
struct ExceptionImpl: public EnsureException, public BaseException
{
    static constexpr const int id = ID;

    ExceptionImpl(DbgInfo info, const char *msg):
        EnsureException{info, msg},
        BaseException{'#' + std::to_string(id) + ' ' + EnsureException::toString() + ' ' + msg}
    {}
};

#define EXCEPTION(T) ExceptionImpl<T, __COUNTER__>

#define TO_STRING_IMPL(x) #x
#define TO_STRING(x) TO_STRING_IMPL(x)

#define ENSURE(cond, ExceptionType) \
    do \
    { \
        if(!(cond)) \
        { \
            throw \
                (ExceptionType) \
                { \
                    DbgInfo{__FILE__, __LINE__, __PRETTY_FUNCTION__}, \
                    TO_STRING(ExceptionType) " : " TO_STRING(cond) \
                }; \
        } \
    } while(false)

#define ASSERT(cond) assert(cond)

using RuntimeError = EXCEPTION(std::runtime_error);

struct ReportCErrNo : public std::runtime_error
{
    ReportCErrNo(const std::string &msg):
        std::runtime_error{std::string{strerror(errno)} + " at " + msg}
    {}
};

using CRuntimeError = EXCEPTION(ReportCErrNo);
