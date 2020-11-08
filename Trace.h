#pragma once

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

enum class TraceLevel
{
    Error,
    Warning,
    Info,
    Debug
};

using LogLevel = TraceLevel;

namespace impl_ {

template <typename T>
void traceSerializeImpl(std::ostream &os, const T &value)
{
    os << value;
}

inline
void traceImpl(std::ostream &)
{}

template <typename T, typename ...T_n>
void traceImpl(std::ostream &os, const T &value, const T_n &...tail)
{
    traceSerializeImpl(os, value);
    traceImpl(os, tail...);
}

inline
TraceLevel currTraceLevel()
{
    static const TraceLevel traceLevel =
        ::getenv("TRACE_LEVEL")
        ? static_cast<TraceLevel>(::atoi(::getenv("TRACE_LEVEL")))
        : TraceLevel::Info;
    return traceLevel;
}

} /* impl_ */


template <typename ...T_n>
void trace(TraceLevel traceLevel, const T_n &...tail)
{
    if(int(impl_::currTraceLevel()) < int(traceLevel)) return;

    std::stringstream ss;

    impl_::traceImpl(ss, tail...);

    std::cerr << ss.rdbuf() << '\n';
}

#ifndef ENABLE_TRACE

#define TRACE(...)

#else

#define TRACE(traceLevel, ...) \
    do \
    { \
        trace(traceLevel, __FILE__, ':', __LINE__, ' ', __FUNCTION__, ' ', __VA_ARGS__); \
    } while(false)

#endif /* ENABLE_TRACE */


#ifndef ENABLE_LOG

#define LOG(...)

#else

#define LOG(logLevel, ...) \
    do \
    { \
        trace(logLevel, __VA_ARGS__); \
    } while(false)

#endif /* ENABLE_TRACE */
