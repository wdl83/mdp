#pragma once

#include <iostream>
#include <sstream>
#include <string>

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

} /* impl_ */

template <typename ...T_n>
void trace(const T_n &...tail)
{
    std::stringstream ss;

    impl_::traceImpl(ss, tail...);

    std::cerr << ss.rdbuf() << '\n';
}

#ifndef ENABLE_TRACE

#define TRACE(...)

#else

#define TRACE(...) \
    do \
    { \
        trace(__FILE__, ':', __LINE__, ' ', __PRETTY_FUNCTION__, ' ', __VA_ARGS__); \
    } while(false)

#endif /* ENABLE_TRACE */
