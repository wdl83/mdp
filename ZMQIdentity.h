#pragma once

#include <string>

#include "Ensure.h"
#include "Except.h"


class ZMQIdentity
{
    std::string value_;
    thread_local static int no_;

    static
    std::string uniqueId();
public:
    static constexpr std::size_t maxLength = 255;

    /* invalid identity */
    ZMQIdentity()
    {}

    ZMQIdentity(std::string str): value_{std::move(str)}
    {
        ENSURE(maxLength >= value_.size(), RuntimeError);
    }

    explicit
    operator bool() const {return !value_.empty();}
    const std::string &asString() const {return value_;}
    const char *data() const {return value_.data();}
    std::size_t size() const {return value_.size();}

    friend
    bool operator==(const ZMQIdentity &x, const ZMQIdentity &y)
    {
        return x.value_ == y.value_;
    }

    friend
    bool operator<(const ZMQIdentity &x, const ZMQIdentity &y)
    {
        return x.value_ < y.value_;
    }

    static
    ZMQIdentity unique()
    {
        return ZMQIdentity{uniqueId()};
    }
};

inline
bool operator!= (const ZMQIdentity &x, const ZMQIdentity &y)
{
    return !(x == y);
}
