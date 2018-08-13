#include "sys/types.h"
#include "unistd.h"

#include <sstream>
#include <thread>

#include "ZMQIdentity.h"


thread_local int ZMQIdentity::no_;

std::string ZMQIdentity::uniqueId()
{
    std::ostringstream oss;

    oss
        << ::getpid()
        << "#" << std::this_thread::get_id()
        << "@" << no_;
    ++no_;
    return oss.str();
}
